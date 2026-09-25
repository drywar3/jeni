#include "semantics/impl.h"
#include "ast/statements.h"
#include "semantics/checks/check_stmt.h"

static bool discover_variable(SemanticContext *sema, StatementPointer stmt);

bool discover_statement(SemanticContext *sema, StatementPointer stmt) {
    switch (stmt->kind) {
    case STMT_Variable:
        return discover_variable(sema, stmt);
    default:
        MINI_UNREACHABLE();
    }
}

WorkerStatus check_statement(SemanticContext *sema, void *data);

bool discover_variable(SemanticContext *sema, StatementPointer stmt) {
    MINI_ASSERT(stmt->kind == STMT_Variable,);
    StmtVariable *variable = (StmtVariable*)stmt;

    if (sema::symbol_is_defined(sema, GLOBAL_SCOPE, variable->name.value)) {
        MINI_UNREACHABLE("symbol redefinition");
    }

    Symbol symbol{ .scope_id = sema::current_scope(sema) };
    symbol.name = variable->name.value;
    symbol.kind = SymbolKind::Variable;
    symbol.as.variable.is_initialized = variable->is_initialized;

    SymbolId id = sema::register_symbol_in(sema,
                                           GLOBAL_SCOPE,
                                           variable->name.value,
                                           variable->name.locus,
                                           symbol);
    printf("%.*s || ID(%zu)\n", SVARG(variable->name.value), id.id);
    return true;
}
