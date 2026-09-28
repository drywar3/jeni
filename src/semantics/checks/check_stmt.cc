#include "semantics/impl.h"
#include "ast/statements.h"
#include "semantics/type/resolver.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_function_definition.h"

#include <mini.cc/dtor.h>

static bool discover_variable(SemanticContext *sema, StatementPointer stmt, sema::ScopeId scope);

bool discover_statement(SemanticContext *sema, StatementPointer stmt)
{
    switch (stmt->kind) {
    case STMT_Variable:
        return discover_variable(sema, stmt, sema->global_scope);
    default:
        MINI_UNREACHABLE();
    }
}

WorkerStatus check_block(SemanticContext *sema, void *data);
WorkerStatus check_variable(SemanticContext *sema, void *data);

WorkerStatus check_statement(SemanticContext *sema, void *data)
{
    StatementPointer stmt = (StatementPointer)data;
    switch (stmt->kind) {
    case STMT_Variable:
        return check_variable(sema, stmt);
    case STMT_Block:
        return check_block(sema, stmt);
    default:
        MINI_UNREACHABLE();
    }
}

WorkerStatus check_variable(SemanticContext *sema, void *data)
{
    WorkerStatus status         = WorkerStatus::Done;
    sema::ScopeId current_scope = sema->current_scope;

    StmtVariable *variable = (StmtVariable *)data;
    MINI_ASSERT(variable->base.kind == STMT_Variable, );
    Name varname = variable->name;

    if (current_scope != sema->global_scope) {
        if (!discover_variable(sema, (StatementPointer)variable, current_scope))
            return WorkerStatus::Failed;
    }

    sema::SymbolPointer symbol =
        sema::find_symbol_in(sema, current_scope, varname.value);
    MINI_ASSERT(symbol != nullptr, "invalid symbol");
    /* avoid resolving the same symbol twice */
    if (symbol->resolve_state == sema::SymbolState::Resolved)
        return WorkerStatus::Done;

    MINI_ASSERT(symbol->resolve_state == sema::SymbolState::Unresolved,
                "symbol is already resolved");

    symbol->resolve_state = sema::SymbolState::Resolving;
    ExpressionPointer initializer = variable->initializer;

    if (variable->is_initialized && initializer->kind == EXPR_Function)
        return sema::check_function_definition(sema, variable);

    if (variable->type_is_defined) {
        TypehintPointer typehint = variable->typehint;
        if (auto s = sema::resolve_typehint(sema, typehint,
                                            variable->name.locus);
            s != WorkerStatus::Done)
            return s;
    } else {
        if (variable->is_initialized) {
            if (auto status = sema::check_expression(sema, initializer);
                status == WorkerStatus::Done) {
            }
        }
    }

    symbol->resolve_state = sema::SymbolState::Resolved;

    return status;
}

bool discover_variable(SemanticContext *sema, StatementPointer stmt, sema::ScopeId scope)
{
    MINI_ASSERT(stmt->kind == STMT_Variable, );
    StmtVariable *variable = (StmtVariable *)stmt;

    if (sema::symbol_is_defined(sema, scope,
                                variable->name.value)) {
        const auto *first = sema::find_symbol_in(sema, scope,
                                                 variable->name.value);

        if (first->resolve_state == sema::SymbolState::Resolved &&
            first->locus == stmt->locus) {
            return true;
        }

        Diagnostic diag   = diag_create(
            DIAG_Error, variable->name.locus, "variable redeclaration",
            mini_string_build(sema->allocator,
                              "symbol `%.*s` is already defined",
                              SVARG(variable->name.value)));
        sema::report(sema,
                     diag_add_label(diag, Label{"symbol was first defined here",
                                                first->locus}));
        return false;
    }

    sema::Symbol symbol{.scope_id = scope};
    symbol.name                       = variable->name.value;
    symbol.kind                       = sema::SymbolKind::Variable;
    symbol.locus                      = variable->name.locus;
    symbol.resolve_state              = sema::SymbolState::Unresolved;
    symbol.as.variable.is_initialized = variable->is_initialized;
    sema::SymbolId id =
        sema::register_symbol_in(sema, scope, variable->name.value,
                                 variable->name.locus, symbol);
    return true;
}

WorkerStatus check_block(SemanticContext *sema, void *data)
{
    StmtBlock *block    = (StmtBlock *)data;
    WorkerStatus status = WorkerStatus::Done;

    sema::ScopeId previous_scope = sema->current_scope;
    sema::ScopeId current_scope  = sema::find_scope_by_locus(sema, block->base.locus);

    if (current_scope != sema::INVALID_SCOPE) {
        sema->current_scope = current_scope;
    } else {
        current_scope = sema::enter_scope(sema, sema::ScopeKind::Block);
        sema::link_locus_to_scope(sema, block->base.locus, current_scope);
    }

    auto scope_end = mini::AttachDtor(block, [&](auto s) {
        sema->current_scope = previous_scope;
    });

    for (usize n = 0; n < mini_array_count(block->body); ++n) {
        StatementPointer stmt = block->body[n];
        auto s                = check_statement(sema, stmt);
        switch (s) {
        case WorkerStatus::Failed:
            status = s;
            break;
        case WorkerStatus::Done:
            continue;
        case WorkerStatus::Pending:
            return s;
        default:
            MINI_UNREACHABLE();
        }
    }
    return status;
}
