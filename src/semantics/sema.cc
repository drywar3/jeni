#include "sema.h"
#include "semantics/type/ids.h"
#include "semantics/checks/check_stmt.h"

#include <mini.cc/array.h>

SemanticStorage semastore_init(Mini_Allocator allocator)
{
    SemanticStorage storage{
        .scopes  = sema::ScopeStorage(allocator),
        .symbols = sema::SymbolStorage(allocator),
        .types   = sema::TypeStorage(allocator),
    };
    /* invalid scope */
    storage.scopes.add_value(
        sema::scope_init(sema::ScopeKind::Invalid, std::nullopt, allocator));
    return storage;
}

SemanticContext semactx_init(Mini_Allocator allocator,
                             DiagnosticPool *diagnostics,
                             SemanticStorage *store)
{
    SemanticContext sema{
        .pending_workers =
            HashMap<sema::SymbolId, MINI_ARRAY(Worker)>(allocator),
        .current_scope = {},
        .block_save_points = SemanticContext::BlockSavePoints(allocator)};
    sema.allocator    = allocator;
    sema.diagnostics  = diagnostics;
    sema.store        = store;
    sema.global_scope = (sema::ScopeId)sema.store->scopes.add_value(
        scope_init(sema::ScopeKind::Global, std::nullopt, allocator));
    sema.current_scope = sema.global_scope;
    return sema;
}

void semactx_resolve(SemanticContext *sema, Program *program)
{
    for (usize n = 0; n < mini_array_count(program->ast); ++n) {
        StatementPointer stmt = program->ast[n];
        discover_statement(sema, stmt);
    }

    for (usize n = 0; n < mini_array_count(program->ast); ++n) {
        StatementPointer stmt = program->ast[n];
        WorkerStatus status   = check_statement(sema, (void *)stmt);

        if (status == WorkerStatus::Pending && !status.is_handled) {
            for (sema::SymbolId id : mini::iterate(status.waiting_on)) {
                sema->register_worker(id,
                                      Worker{(void *)stmt, check_statement});
            }
            // sema->register_worker(status.waiting_on[0], Worker{(void*)stmt,
            // check_statement});
        }
    }

    while (!sema->pending_workers.empty()) {
        for (auto [id, workers, x] : sema->pending_workers) {
            sema::Symbol *symbol = sema->symbols().at_index_ptr(usize(id));
            if (symbol->resolve_state == sema::SymbolState::Resolved) {
                for (usize n = 0; n < mini_array_count(workers); ++n) {
                    Worker worker       = workers[n];
                    WorkerStatus status = worker.func(sema, worker.data, true);
                    if (status == WorkerStatus::Done ||
                        status == WorkerStatus::Failed) {
                        mini_array_remove(workers, n);
                    }
                }
            } else if (symbol->resolve_state == sema::SymbolState::Failed) {
                sema->pending_workers.erase(id);
                continue;
            }

            if (mini_array_count(workers) == 0) {
                sema->pending_workers.erase(id);
            }
        }
    }
}

using namespace sema::type_id;

constexpr sema::Type TYPES[] = {
    [(int)Error]  = sema::Type(sema::TypeKind::Error),
    [(int)Void]   = sema::Type(sema::TypeKind::Void),
    [(int)Int]    = sema::Type(sema::TypeKind::Int),
    [(int)Uint]   = sema::Type(sema::TypeKind::Uint),
    [(int)String] = sema::Type(sema::TypeKind::String),
    [(int)Char]   = sema::Type(sema::TypeKind::Char),
    [(int)Bool]   = sema::Type(sema::TypeKind::Bool),
};

void semastore_init_builtin_types(SemanticStorage *store)
{
    for (usize n = 0; n < (usize)sema::type_id::_LAST_; n++) {
        store->types.add_value(TYPES[n]);
    }
}
