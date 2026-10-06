#include "semantics/impl.h"
#include "ast/statements.h"
#include "semantics/worker.h"
#include "semantics/type/coercer.h"
#include "semantics/type/resolver.h"
#include "semantics/entities/scope.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_block.h"
#include "semantics/checks/check_function_definition.h"

#include <mini.cc/array.h>
#include <mini.cc/scope_guard.h>

static bool discover_variable(SemanticContext *sema, StatementPointer stmt,
                              sema::ScopeId scope);

bool discover_statement(SemanticContext *sema, StatementPointer stmt)
{
    switch (stmt->kind) {
    case STMT_Variable:
        return discover_variable(sema, stmt, sema->global_scope);
    default:
        MINI_UNREACHABLE();
    }
}

WorkerStatus check_variable(SemanticContext *sema, void *data,
                            bool is_resumption);

WorkerStatus check_statement(SemanticContext *sema, void *data,
                             bool is_resumption)
{
    StatementPointer stmt = (StatementPointer)data;
    switch (stmt->kind) {
    case STMT_Variable:
        return check_variable(sema, stmt, is_resumption);
    case STMT_Block:
        return check_block(sema, (StmtBlock *)stmt, is_resumption);
    case STMT_Expr:
        return sema::check_expression(sema, stmt);
    default:
        MINI_UNREACHABLE();
    }
}

WorkerStatus check_variable(SemanticContext *sema, void *data,
                            bool is_resumption)
{
    StmtVariable *variable = (StmtVariable *)data;
    MINI_ASSERT(variable->base.kind == STMT_Variable,
                "expected variable statement");

    Name varname = variable->name;

    sema::ScopeId previous_scope = sema->current_scope;
    sema::ScopeId current_scope =
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
            return WorkerStatus::Failed;
    }

    auto symbol_opt =
        sema::find_symbol_in(sema, current_scope, varname.value);

    MINI_ASSERT(symbol_opt.has_value(), "invalid symbol");

    sema::SymbolProxy symbol = *symbol_opt;

    if (symbol->is_state(sema::SymbolState::Resolved))
        return WorkerStatus::Done;

    ExpressionPointer initializer = variable->initializer;

    if (variable->is_initialized && initializer->kind == EXPR_Function)
        return sema::check_function_definition(sema, variable, is_resumption);

    symbol->set_state(sema::SymbolState::Resolving);

    if (variable->is_initialized) {
        WorkerStatus init_status = sema::check_expression(sema, initializer);
        if (init_status == WorkerStatus::Failed) {
            symbol->set_state(sema::SymbolState::Failed);
            return WorkerStatus::Failed;
        }

        if (init_status != WorkerStatus::Done) {
            // Restore Unresolved status so worker re-entry tracks clean state
            symbol->set_state(sema::SymbolState::Unresolved);
            return init_status;
        }
    }

    if (variable->type_is_defined) {
        TypehintPointer typehint = variable->typehint;
        WorkerStatus type_status =
            sema::resolve_typehint(sema, typehint, variable->name.locus);

        if (type_status != WorkerStatus::Done) {
            if (type_status == WorkerStatus::Failed) {
                symbol->set_state(sema::SymbolState::Failed);
            } else {
                symbol->set_state(sema::SymbolState::Unresolved);
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
                symbol->set_state(sema::SymbolState::Failed);
                return WorkerStatus::Failed;
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
            symbol->set_state(sema::SymbolState::Failed);
            return WorkerStatus::Failed;
        }
    }

    symbol->set_state(sema::SymbolState::Resolved);
    sema->wake_up_workers(
        *sema::get_id_of_symbol(sema, current_scope, symbol->name));

    return WorkerStatus::Done;
}

bool discover_variable(SemanticContext *sema, StatementPointer stmt,
                       sema::ScopeId scope)
{
    MINI_ASSERT(stmt->kind == STMT_Variable, "expected variable statement");
    StmtVariable *variable = (StmtVariable *)stmt;

    if (sema::symbol_is_defined(sema, scope, variable->name.value)) {
        auto first = *sema::find_symbol_in(sema, scope, variable->name.value);
        if ((first->is_state(sema::SymbolState::Resolved) ||
             first->is_state(sema::SymbolState::Resolving)) &&
            first->locus == variable->name.locus) {
            return true;
        }

        Diagnostic diag = diag_create(Severity::Error, variable->name.locus, "variable redeclaration",
                                      mini_string_build(sema->allocator,
                                                        "symbol `%.*s` is already defined",
                                                        SVARG(variable->name.value)));
        sema::report(sema,
                     diag_add_label(diag, Label{"symbol was first defined here",
                                                first->locus}));
        return false;
    }

    sema::Symbol symbol{};
    symbol.name                       = variable->name.value;
    symbol.kind                       = sema::SymbolKind::Variable;
    symbol.locus                      = variable->name.locus;
    symbol.scope_id                   = scope;
    symbol.set_state(sema::SymbolState::Unresolved);
    symbol.variable.is_initialized = variable->is_initialized;
    sema::register_symbol_in(sema, scope, variable->name.value,
                             variable->name.locus, symbol);
    return true;
}
