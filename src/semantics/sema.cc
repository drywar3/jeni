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
        .call_schemas = SemanticStorage::CallSchemas(allocator),
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
        HashMap<sema::SymbolId, mini::Array<Worker>>(allocator),
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
        sema::discover_statement(sema, stmt);
    }

    for (usize n = 0; n < mini_array_count(program->ast); ++n) {
        StatementPointer stmt = program->ast[n];
        WorkerStatus status   = sema::check_statement(sema, (void *)stmt);

        if (status == WorkerStatus::Pending) {
            for (sema::SymbolId id : mini::iterate(status.waiting_on)) {
                sema->register_worker(id,
                                      Worker(stmt, sema::check_statement, sema->current_scope));
            }
        }
    }

    while (!sema->pending_workers.empty()) {
        for (auto [id, workers, x] : sema->pending_workers) {
            sema::Symbol *symbol = sema->symbols().at_index_ptr(usize(id));
            if (symbol->is_state(sema::SymbolState::Resolved)) {
                for (usize n = 0; n < workers.count(); ++n) {
                    Worker worker       = workers[n];
                    WorkerStatus status = worker.resume(sema);
                    if (status == WorkerStatus::Done ||
                        status == WorkerStatus::Failed) {
                        workers.remove(n);
                    }
                }
            } else if (symbol->is_state(sema::SymbolState::Failed)) {
                sema->pending_workers.erase(id);
                continue;
            }

            if (workers.count() == 0) {
                sema->pending_workers.erase(id);
            }
        }
    }
}

sema::FunctionCallSchema* SemanticContext::get_call_schema(sema::SymbolId symbol_id)
{
    return store->call_schemas.find(symbol_id);
}

void SemanticContext::set_call_schema(sema::SymbolId symbol_id, sema::FunctionCallSchema schema)
{
    MINI_ASSERT(get_call_schema(symbol_id) == nullptr, "symbol call schema already exists");
    store->call_schemas[symbol_id] = schema;
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
    [(int)Int64]   = sema::Type(sema::TypeKind::Int64),
};

void semastore_init_builtin_types(SemanticStorage *store)
{
    for (usize n = 0; n < (usize)sema::type_id::_LAST_; n++) {
        store->types.add_value(TYPES[n]);
    }
}
