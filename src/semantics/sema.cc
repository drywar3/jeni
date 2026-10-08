#include "sema.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/type/ids.h"

#include <mini.cc/array.h>

Semantic_Storage semastore_init(Mini_Allocator allocator)
{
    Semantic_Storage storage{
        .scopes       = sema::Scope_Storage(allocator),
        .symbols      = sema::Symbol_Storage(allocator),
        .types        = sema::Type_Storage(allocator),
        .call_schemas = Semantic_Storage::Call_Schemas(allocator),
    };
    /* invalid scope */
    storage.scopes.add_value(
        sema::scope_init(sema::Scope_Kind::Invalid, std::nullopt, allocator));
    return storage;
}

Semantic_Context semactx_init(Mini_Allocator allocator,
                              Diagnostic_Pool *diagnostics,
                              Semantic_Storage *store)
{
    Semantic_Context sema{
        .pending_workers =
            HashMap<sema::Symbol_Id, mini::Array<Worker>>(allocator),
        .current_scope     = {},
        .block_save_points = Semantic_Context::Block_Save_Points(allocator)};
    sema.allocator    = allocator;
    sema.diagnostics  = diagnostics;
    sema.store        = store;
    sema.global_scope = (sema::Scope_Id)sema.store->scopes.add_value(
        scope_init(sema::Scope_Kind::Global, std::nullopt, allocator));
    sema.current_scope = sema.global_scope;
    return sema;
}

void semactx_resolve(Semantic_Context *sema, Program *program)
{
    for (usize n = 0; n < mini_array_count(program->ast); ++n) {
        StatementPointer stmt = program->ast[n];
        sema::discover_statement(sema, stmt);
    }

    for (usize n = 0; n < mini_array_count(program->ast); ++n) {
        StatementPointer stmt = program->ast[n];
        Worker_Status status  = sema::check_statement(sema, (void *)stmt);

        if (status == Worker_Status::Pending) {
            for (sema::Symbol_Id id : mini::iterate(status.waiting_on)) {
                sema->register_worker(id, Worker(stmt, sema::check_statement,
                                                 sema->current_scope));
            }
        }
    }

    while (!sema->pending_workers.empty()) {
        for (auto [id, workers, x] : sema->pending_workers) {
            sema::Symbol *symbol = sema->symbols().at_index_ptr(usize(id));
            if (symbol->is_state(sema::Symbol_State::Resolved)) {
                for (usize n = 0; n < workers.count(); ++n) {
                    Worker worker        = workers[n];
                    Worker_Status status = worker.resume(sema);
                    if (status == Worker_Status::Done ||
                        status == Worker_Status::Failed) {
                        workers.remove(n);
                    }
                }
            } else if (symbol->is_state(sema::Symbol_State::Failed)) {
                sema->pending_workers.erase(id);
                continue;
            }

            if (workers.count() == 0) {
                sema->pending_workers.erase(id);
            }
        }
    }
}

sema::Function_Call_Schema *
Semantic_Context::get_call_schema(sema::Symbol_Id symbol_id)
{
    return store->call_schemas.find(symbol_id);
}

void Semantic_Context::set_call_schema(sema::Symbol_Id symbol_id,
                                       sema::Function_Call_Schema schema)
{
    MINI_ASSERT(get_call_schema(symbol_id) == nullptr,
                "symbol call schema already exists");
    store->call_schemas[symbol_id] = schema;
}

using namespace sema::type_id;

constexpr sema::Type TYPES[] = {
    [(int)Error]  = sema::Type(sema::Type_Kind::Error),
    [(int)Void]   = sema::Type(sema::Type_Kind::Void),
    [(int)Int]    = sema::Type(sema::Type_Kind::Int),
    [(int)Uint]   = sema::Type(sema::Type_Kind::Uint),
    [(int)String] = sema::Type(sema::Type_Kind::String),
    [(int)Char]   = sema::Type(sema::Type_Kind::Char),
    [(int)Bool]   = sema::Type(sema::Type_Kind::Bool),
    [(int)Int64]  = sema::Type(sema::Type_Kind::Int64),
};

void semastore_init_builtin_types(Semantic_Storage *store)
{
    for (usize n = 0; n < (usize)sema::type_id::_LAST_; n++) {
        store->types.add_value(TYPES[n]);
    }
}
