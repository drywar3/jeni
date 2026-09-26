#include "sema.h"
#include "semantics/checks/check_stmt.h"

SemanticContext semactx_init(Mini_Allocator allocator, DiagnosticPool *diagnostics) {
    SemanticContext sema{ .current_scope = GLOBAL_SCOPE, .pending_workers = HashMap<usize, MINI_ARRAY(Worker)>(allocator) };
    sema.allocator = allocator;
    sema.diagnostics = diagnostics;
    auto *values = sema.scopes.values();
    mini_array_append(values, scope_init(ScopeKind::Global, std::nullopt, allocator));
    return sema;
}

void semactx_resolve(SemanticContext *sema, Program *program) {
    for (usize n = 0; n < mini_array_count(program->ast); ++n) {
        StatementPointer stmt = program->ast[n];
        discover_statement(sema, stmt);
    }
}
