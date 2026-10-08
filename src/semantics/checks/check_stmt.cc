#include "semantics/checks/check_stmt.h"
#include "ast/statements.h"
#include "semantics/checks/check_block.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_for_ever.h"
#include "semantics/checks/check_function_definition.h"
#include "semantics/checks/check_if_stmt.h"
#include "semantics/checks/check_return_stmt.h"
#include "semantics/entities/scope.h"
#include "semantics/impl.h"
#include "semantics/type/coercer.h"
#include "semantics/type/resolver.h"
#include "semantics/worker.h"

#include <mini.cc/array.h>
#include <mini.cc/scope_guard.h>

static bool discover_variable(Semantic_Context *sema, StatementPointer stmt,
                              sema::Scope_Id scope);

bool sema::discover_statement(Semantic_Context *sema, StatementPointer stmt)
{
    switch (stmt->kind) {
    case STMT_Variable:
        return discover_variable(sema, stmt, sema->global_scope);
    default:
        MINI_UNREACHABLE();
    }
}

Worker_Status check_variable(Semantic_Context *sema, void *data,
                             bool is_resumption);

Worker_Status sema::check_statement(Semantic_Context *sema, void *data,
                                    bool is_resumption)
{
    StatementPointer stmt = (StatementPointer)data;
    switch (stmt->kind) {
    case STMT_Variable:
        return check_variable(sema, stmt, is_resumption);
    case STMT_Block:
        return check_block(sema, (ast::stmt::Block *)stmt, is_resumption);
    case STMT_Expr:
        return sema::check_expression(sema, stmt);
    case STMT_If:
        return sema::check_if_stmt(sema, stmt);
    case STMT_Return:
        return sema::check_return_stmt(sema, stmt);
    case STMT_For_Ever:
        return sema::check_for_ever_stmt(sema, stmt);
    case STMT_Break:
        return sema::check_break_stmt(sema, stmt);
    default:
        MINI_UNREACHABLE();
    }
}

Worker_Status check_variable(Semantic_Context *sema, void *data,
                             bool is_resumption)
{
    ast::stmt::Variable *variable = (ast::stmt::Variable *)data;
    MINI_ASSERT(variable->kind == STMT_Variable, "expected variable statement");

    Name varname = variable->name;

    sema::Scope_Id previous_scope = sema->current_scope;
    sema::Scope_Id current_scope =
        sema::find_scope_by_locus(sema, varname.locus);

    if (current_scope != sema::INVALID_SCOPE) {
        sema->current_scope = current_scope;
    } else {
        current_scope = sema->current_scope;
        sema::link_locus_to_scope(sema, varname.locus, current_scope);
    }

    auto scope_guard =
        mini::ScopeGuard([&] { sema->current_scope = previous_scope; });

    if (current_scope != sema->global_scope) {
        if (!discover_variable(sema, (StatementPointer)variable, current_scope))
            return Worker_Status::Failed;
    }

    auto symbol_opt = sema::find_symbol_in(sema, current_scope, varname.value);

    MINI_ASSERT(symbol_opt.has_value(), "invalid symbol");

    sema::Symbol_Proxy symbol = *symbol_opt;

    if (symbol->is_state(sema::Symbol_State::Resolved))
        return Worker_Status::Done;

    ExpressionPointer initializer = variable->initializer;

    if (variable->is_initialized && initializer->kind == EXPR_Function)
        return sema::check_function_definition(sema, variable, is_resumption);

    symbol->set_state(sema::Symbol_State::Resolving);

    if (variable->is_initialized) {
        Worker_Status init_status = sema::check_expression(sema, initializer);
        if (init_status == Worker_Status::Failed) {
            symbol->set_state(sema::Symbol_State::Failed);
            return Worker_Status::Failed;
        }

        if (init_status != Worker_Status::Done) {
            // Restore Unresolved status so worker re-entry tracks clean state
            symbol->set_state(sema::Symbol_State::Unresolved);
            return init_status;
        }
    }

    if (variable->type_is_defined) {
        TypehintPointer typehint = variable->typehint;
        Worker_Status type_status =
            sema::resolve_typehint(sema, typehint, variable->name.locus);

        if (type_status != Worker_Status::Done) {
            if (type_status == Worker_Status::Failed) {
                symbol->set_state(sema::Symbol_State::Failed);
            } else {
                symbol->set_state(sema::Symbol_State::Unresolved);
            }
            return type_status;
        }

        if (variable->is_initialized) {
            auto recieved_type =
                sema::get_type_at_locus(sema, initializer->locus);
            auto expected_type =
                sema::get_type_at_locus(sema, variable->name.locus);

            if (!sema::coerce_type_into(sema, expected_type, recieved_type,
                                        typehint->locus, initializer->locus)) {
                symbol->set_state(sema::Symbol_State::Failed);
                return Worker_Status::Failed;
            }

            symbol->variable.type_id = expected_type;
        }
    } else {
        if (variable->is_initialized) {
            auto inferred_type =
                sema::get_type_at_locus(sema, initializer->locus);
            symbol->variable.type_id = inferred_type;
        } else {
            // Handle error case: var x; without type or initializer
            Diagnostic diag = diag_create(
                Severity::Error, variable->name.locus, "type error",
                "variables without initializers must specify an explicit type");
            sema::report(sema, diag);
            symbol->set_state(sema::Symbol_State::Failed);
            return Worker_Status::Failed;
        }
    }

    symbol->set_state(sema::Symbol_State::Resolved);
    sema->wake_up_workers(
        *sema::get_id_of_symbol(sema, current_scope, symbol->name));

    return Worker_Status::Done;
}

bool discover_variable(Semantic_Context *sema, StatementPointer stmt,
                       sema::Scope_Id scope)
{
    MINI_ASSERT(stmt->kind == STMT_Variable, "expected variable statement");
    ast::stmt::Variable *variable = (ast::stmt::Variable *)stmt;

    if (sema::symbol_is_defined(sema, scope, variable->name.value)) {
        auto first = *sema::find_symbol_in(sema, scope, variable->name.value);
        if ((first->is_state(sema::Symbol_State::Resolved) ||
             first->is_state(sema::Symbol_State::Resolving)) &&
            first->locus == variable->name.locus) {
            return true;
        }

        Diagnostic diag = diag_create(
            Severity::Error, variable->name.locus, "variable redeclaration",
            mini_string_build(sema->allocator,
                              "symbol `%.*s` is already defined",
                              SVARG(variable->name.value)));
        sema::report(sema,
                     diag_add_label(diag, Label{"symbol was first defined here",
                                                first->locus}));
        return false;
    }

    sema::Symbol symbol{};
    symbol.name     = variable->name.value;
    symbol.kind     = sema::Symbol_Kind::Variable;
    symbol.locus    = variable->name.locus;
    symbol.scope_Id = scope;
    symbol.set_state(sema::Symbol_State::Unresolved);
    symbol.variable.is_initialized = variable->is_initialized;
    symbol.variable.mutability     = variable->mutability;
    sema::register_symbol_in(sema, scope, variable->name.value,
                             variable->name.locus, symbol);
    return true;
}
