#include "semantics/impl.h"
#include "ast/statements.h"
#include "semantics/type/coercer.h"
#include "semantics/type/resolver.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/checks/check_expr.h"
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

WorkerStatus check_block(SemanticContext *sema, void *data, bool is_resumption);
WorkerStatus check_variable(SemanticContext *sema, void *data, bool is_resumption);

WorkerStatus check_statement(SemanticContext *sema, void *data, bool is_resumption)
{
    StatementPointer stmt = (StatementPointer)data;
    switch (stmt->kind) {
    case STMT_Variable:
        return check_variable(sema, stmt, is_resumption);
    case STMT_Block:
        return check_block(sema, stmt, is_resumption);
    default:
        MINI_UNREACHABLE();
    }
}

WorkerStatus check_variable(SemanticContext *sema, void *data, bool is_resumption)
{
    StmtVariable *variable = (StmtVariable *)data;
    MINI_ASSERT(variable->base.kind == STMT_Variable, );

    Name varname = variable->name;

    /* attempt to resume in the scope of the previous resumption */
    sema::ScopeId previous_scope = sema->current_scope;
    sema::ScopeId current_scope  =sema::find_scope_by_locus(
                                                            sema, varname.locus);;

    if (current_scope != sema::INVALID_SCOPE) {
        sema->current_scope = current_scope;
    } else {
        /* else we can just continue in the current scope */
        current_scope = sema->current_scope;
        sema::link_locus_to_scope(sema, varname.locus, current_scope); /* link it so we remember in any next resumes */
    }

    sema->current_scope = current_scope;

    auto scope_guard = mini::ScopeGuard([&]{
        sema->current_scope = previous_scope;
    });

    if (current_scope != sema->global_scope) {
        if (!discover_variable(sema, (StatementPointer)variable, current_scope))
            return WorkerStatus::Failed;
    }

    sema::Symbol *symbol =
        sema::find_symbol_in(sema, current_scope, varname.value);

    MINI_ASSERT(symbol != nullptr, "invalid symbol");

    /* avoid resolving the same symbol twice */
    if (symbol->resolve_state == sema::SymbolState::Resolved)
        return WorkerStatus::Done;

    symbol->resolve_state         = sema::SymbolState::Resolving;
    ExpressionPointer initializer = variable->initializer;

    if (variable->is_initialized && initializer->kind == EXPR_Function)
        return sema::check_function_definition(sema, variable, is_resumption);


    if (variable->is_initialized) {
        WorkerStatus init_status = sema::check_expression(sema, initializer);
        if (init_status == WorkerStatus::Failed) {
            symbol->resolve_state = sema::SymbolState::Failed;
        }

        if (init_status != WorkerStatus::Done)
            return init_status;
    }

    if (variable->type_is_defined) {
        TypehintPointer typehint = variable->typehint;
        if (auto s =
                sema::resolve_typehint(sema, typehint, variable->name.locus);
            s != WorkerStatus::Done)
            return s;

        if (variable->is_initialized) {
            auto recieved_type =
                sema::get_type_at_locus(sema, initializer->locus);
            auto expected_type =
                sema::get_type_at_locus(sema, variable->name.locus);
            if (!sema::coerce_type_into(sema, expected_type, recieved_type,
                                        typehint->locus, initializer->locus)) {
                return WorkerStatus::Failed;
            }

            symbol->as.variable.type_id = expected_type;
        }
    } else {
        auto inferred_type = sema::get_type_at_locus(sema, initializer->locus);
        symbol->as.variable.type_id = inferred_type;
    }

    symbol->resolve_state = sema::SymbolState::Resolved;
    sema->wake_up_workers(*sema::get_id_of_symbol(sema, current_scope, symbol->name));

    return WorkerStatus::Done;
}

bool discover_variable(SemanticContext *sema, StatementPointer stmt,
                       sema::ScopeId scope)
{
    MINI_ASSERT(stmt->kind == STMT_Variable, );
    StmtVariable *variable = (StmtVariable *)stmt;

    if (sema::symbol_is_defined(sema, scope, variable->name.value)) {
        const auto *first =
            sema::find_symbol_in(sema, scope, variable->name.value);

        if ((first->resolve_state == sema::SymbolState::Resolved ||
             first->resolve_state == sema::SymbolState::Resolving) &&
            first->locus == variable->name.locus) {
            return true;
        }

        Diagnostic diag = diag_create(
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
    sema::SymbolId id                 = sema::register_symbol_in(
        sema, scope, variable->name.value, variable->name.locus, symbol);
    return true;
}

WorkerStatus check_block(SemanticContext *sema, void *data, bool is_resumption)
{
    StmtBlock *block    = (StmtBlock *)data;
    sema::ScopeId previous_scope = sema->current_scope;
    sema::ScopeId current_scope =
        sema::find_scope_by_locus(sema, block->base.locus);

    if (current_scope != sema::INVALID_SCOPE) {
        sema->current_scope = current_scope;
        MINI_ASSERT(is_resumption,);
    } else {
        current_scope = sema::enter_scope(sema, sema::ScopeKind::Block);
        sema::link_locus_to_scope(sema, block->base.locus, current_scope);
    }

    auto scope_end = mini::ScopeGuard([&]{ sema->current_scope = previous_scope; });

    usize start = 0;
    if (sema->block_has_save_point(block->base.locus)) {
        start = sema->get_block_save_point(block->base.locus);
    }

    for (usize n = start; n < mini_array_count(block->body); ++n) {
        StatementPointer stmt = block->body[n];
        WorkerStatus s        = check_statement(sema, stmt, is_resumption);

        if (s == WorkerStatus::Pending) {
            for (sema::SymbolId id : mini::iterate(s.waiting_on)) {
                sema->register_worker(id, Worker{(void*)stmt, check_statement});
            }
            sema->set_block_save_point(block->base.locus, n);
            return s;
        }

        if (s != WorkerStatus::Done) {
            sema->set_block_save_point(block->base.locus, n + 1);
            return s;
        }
    }

    return WorkerStatus::Done;
}
