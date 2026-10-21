#include "ast/statements.h"
#include "semantics/impl.h"
#include "semantics/worker.h"
#include "semantics/type/coercer.h"
#include "semantics/type/resolver.h"
#include "semantics/entities/scope.h"
#include "semantics/checks/check_use.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/checks/check_block.h"
#include "semantics/checks/check_if_stmt.h"
#include "semantics/checks/check_for_ever.h"
#include "semantics/checks/check_struct_decl.h"
#include "semantics/checks/check_return_stmt.h"
#include "semantics/checks/check_function_definition.h"
#include "semantics/checks/check_namespace_definition.h"

#include <mini.cc/array.h>
#include <mini.cc/scope_guard.h>

static bool discover_variable(Semantic_Context *sema, StatementPointer stmt,
                              sema::Scope_Id scope);

bool discover_expression(Semantic_Context *sema, Expression *expr, sema::Scope_Id scope_id)
{
    switch (expr->kind) {
    case EXPR_Use: return discover_use_expr(sema, expr, scope_id);
    default: sema::report_error(sema, *expr, "expression is not allowed here");
        return false;
    }
}

bool sema::discover_statement(Semantic_Context *sema, Statement *stmt, sema::Scope_Id scope_id)
{
    switch (stmt->kind) {
    case STMT_Variable:
        return discover_variable(sema, stmt, scope_id);
    case STMT_Expr: return discover_expression(sema, (Expression*)stmt, scope_id);
    default:
        MINI_UNREACHABLE();
    }
}

Worker_Status check_variable(Semantic_Context *sema, Statement *&data,
                             bool is_resumption);

Worker_Status sema::check_statement(Semantic_Context *sema, Worker_Payload data,
                                    bool is_resumption)
{
    auto &stmt = data.as_stmt();
    switch (stmt->kind) {
    case STMT_Variable:
        return check_variable(sema, stmt, is_resumption);
    case STMT_Block:
        return check_block(sema, (ast::stmt::Block *)stmt, is_resumption);
    case STMT_Expr:
        return sema::check_expression(sema, (Expression*&)stmt);
    case STMT_If:
        return sema::check_if_stmt(sema, stmt);
    case STMT_Return:
        return sema::check_return_stmt(sema, stmt);
    case STMT_For_Ever:
        return sema::check_for_ever_stmt(sema, stmt);
    case STMT_While_Loop: return sema::check_while_loop(sema, stmt);
    case STMT_Break:
        return sema::check_break_stmt(sema, stmt);
    default:
        MINI_UNREACHABLE();
    }
}

Worker_Status check_variable(Semantic_Context *sema, Statement *&data,
                             bool is_resumption)
{
    auto &variable = (ast::stmt::Variable *&)data;
    MINI_ASSERT(variable->kind == STMT_Variable, "expected variable statement");

    Name varname = variable->name;

    if (!is_resumption) {
        if (!discover_variable(sema, (StatementPointer)variable, sema->current_scope))
            return Worker_Status::Failed;
    }

    auto symbol_opt = sema::find_symbol_in(sema, sema->current_scope, varname.value);

    MINI_ASSERT(symbol_opt.has_value(), "invalid symbol");

    sema::Symbol_Proxy symbol = *symbol_opt;

    if (symbol->is_state(sema::Symbol_State::Resolved))
        return Worker_Status::Done;

    auto &initializer = variable->initializer;

    if (variable->is_initialized) {
        switch (initializer->kind) {
        case EXPR_Function:   return sema::check_function_definition(sema, variable, is_resumption);
        case EXPR_Struct:     return sema::check_struct_decl(sema, variable, is_resumption);
        case EXPR_Namespace:  return sema::check_namespace_definition(sema, variable, is_resumption);
        default:              break;
        }
    }

    symbol->set_state(sema::Symbol_State::Resolving);

    const auto check_initializer = [&]() -> Worker_Status {
        Worker_Status init_status = sema::check_expression(sema, initializer);
        if (init_status == Worker_Status::Failed) {
            symbol->set_state(sema::Symbol_State::Failed);
            return Worker_Status::Failed;
        }

        if (init_status != Worker_Status::Done) {
            symbol->set_state(sema::Symbol_State::Unresolved);
            return init_status;
        }

        return Worker_Status::Done;
    };

    if (variable->type_is_defined) {
        Expression *typehint = variable->typehint;
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

        auto expected_type =
            sema::get_type_at_locus(sema, variable->name.locus);

        if (variable->is_initialized) {
            auto save = sema->set_current_type_context(expected_type);
            //auto scope_guard = mini::ScopeGuard([&](){sema->set_current_type_context(save);});
            MINI_DEFER {sema->set_current_type_context(save); };
            auto status = check_initializer();
            if (!status.is_done()) return status;
            auto recieved_type =
                sema::get_type_at_locus(sema, initializer->locus);
            if (!sema::coerce_type_into(sema, expected_type, recieved_type,
                                        typehint->locus, initializer->locus)) {
                symbol->set_state(sema::Symbol_State::Failed);
                return Worker_Status::Failed;
            }

        }
        symbol->variable.type_id = expected_type;
        sema::link_locus_to_type(sema, symbol->locus, expected_type);
    } else {
        if (variable->is_initialized) {
            auto save = sema->set_current_type_context(None);
            MINI_DEFER {sema->set_current_type_context(save); };
            auto status = check_initializer();
            if (!status.is_done()) return status;
            auto inferred_type =
                sema::get_type_at_locus(sema, initializer->locus);
            symbol->variable.type_id = inferred_type;
            sema::link_locus_to_type(sema, symbol->locus, inferred_type);
        } else {
            Diagnostic diag = diag_create(
                Severity::Error, variable->name.locus, "type error",
                "variables without initializers must specify an explicit type");
            sema::report(sema, diag);
            symbol->set_state(sema::Symbol_State::Failed);
            sema::link_locus_to_type(sema, symbol->locus, sema::type_id::Error);
            return Worker_Status::Failed;
        }
    }

    hir::Profile *profile   = &symbol->profile;
    profile->name           = symbol->name.to_string(sema->allocator);
    profile->repr_name      = symbol->name.to_string(sema->allocator);
    profile->canonical_name = mini_string_build(sema->allocator, "");
    /* if we are not in a function scope, we should mangle the name of the variable */
    if (!sema::find_first_scope_of(sema, sema::Scope_Kind::Function, [](auto *sema, auto &scope) {})) {
        if (symbol->def_ns)
            sema::generate_canon_name(sema, *symbol->def_ns, &profile->canonical_name);
        mini_string_append_fmt(&profile->canonical_name, "_%zu%.*s", symbol->name.length(), SVARG(symbol->name.base()));
    } else {
        profile->canonical_name = mini_sv_to_string(symbol->name, sema->allocator);
    }


    if (variable->mutability && variable->is_initialized) {
        auto value = sema->vm->eval(sema, variable->initializer);
        MINI_ASSERT(value.has_value(), );
        symbol->variable.value = *value;
    }

    symbol->set_state(sema::Symbol_State::Resolved);
    sema->wake_up_workers(symbol.get_id());

    return Worker_Status::Done;
}

bool discover_variable(Semantic_Context *sema, StatementPointer stmt,
                       sema::Scope_Id scope)
{
    MINI_ASSERT(stmt->kind == STMT_Variable, "expected variable statement");
    ast::stmt::Variable *variable = (ast::stmt::Variable *)stmt;

    if (sema::symbol_is_defined(sema, scope, variable->name.value)) {
        auto first = *sema::find_symbol_in(sema, scope, variable->name.value);
        if (// (first->is_state(sema::Symbol_State::Resolved) ||
            //  first->is_state(sema::Symbol_State::Resolving)) &&
            first->locus == variable->name.locus) {
            return true;
        }

        Diagnostic diag = diag_create(
            Severity::Error, variable->name.locus, "variable redeclaration",
            mini_string_build(sema->allocator,
                              "symbol `%.*s` is already defined",
                              SVARG(variable->name.value.base())));
        sema::report(sema,
                     diag_add_label(diag, Label{"symbol was first defined here",
                                                first->locus}));
        return false;
    }

    sema::Symbol symbol{};
    symbol.name     = variable->name.value;
    symbol.kind     = sema::Symbol_Kind::Variable;
    symbol.locus    = variable->name.locus;
    symbol.scope_id = scope;
    symbol.set_state(sema::Symbol_State::Unresolved);
    symbol.variable.mutability     = variable->mutability;
    symbol.variable.is_initialized = variable->is_initialized;
    symbol.variable.expr           = variable->initializer;
    symbol.def_ns                  = sema->get_current_namespace();
    sema::register_symbol_in(sema, scope, variable->name.value,
                             variable->name.locus, symbol);
    return true;
}
