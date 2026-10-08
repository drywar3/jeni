#pragma once

#include "ast/ast.h"
#include "diagnostic.h"
#include "misc/map.h"
#include "semantics/entities/call_schema.h"
#include "semantics/entities/scope.h"
#include "semantics/entities/type.h"
#include "worker.h"

#include <mini.c/array.h>
#include <mini.c/mini_def.h>

struct Semantic_Storage {
    using Call_Schemas = HashMap<sema::Symbol_Id, sema::Function_Call_Schema>;

    sema::Scope_Storage scopes;
    sema::Symbol_Storage symbols;
    sema::Type_Storage types;
    Call_Schemas call_schemas;
};

struct Semantic_Context {
    /* maps [symbol id] -> pending workers */
    HashMap<sema::Symbol_Id, mini::Array<Worker>> pending_workers;

    using Block_Save_Points = HashMap<Locus, usize>;

    Diagnostic_Pool *diagnostics;
    Mini_Allocator allocator;
    Semantic_Storage *store;

    sema::Scope_Id global_scope;
    sema::Scope_Id current_scope;
    Block_Save_Points block_save_points;

    auto &types() { return store->types; }
    auto &scopes() { return store->scopes; }
    auto &symbols() { return store->symbols; }

    const auto &types() const { return store->types; }
    const auto &scopes() const { return store->scopes; }
    const auto &symbols() const { return store->symbols; }

    sema::Function_Call_Schema *get_call_schema(sema::Symbol_Id symbol_id);
    void set_call_schema(sema::Symbol_Id symbol_id,
                         sema::Function_Call_Schema schema);

    bool block_has_save_point(Locus locus) const
    {
        return block_save_points.contains(locus);
    }

    usize get_block_save_point(Locus locus) const
    {
        return *block_save_points.find(locus);
    }

    void set_block_save_point(Locus locus, usize point)
    {
        block_save_points[locus] = point;
    }

    void register_worker(sema::Symbol_Id id, Worker worker)
    {
        if (pending_workers.contains(id)) {
            pending_workers[id].append(worker);
        } else {
            mini::Array<Worker> workers = mini::Array<Worker>(allocator);
            workers.append(worker);
            pending_workers[id] = workers;
        }
    }

    void wake_up_workers(sema::Symbol_Id symbol_id)
    {
        if (!pending_workers.contains(symbol_id))
            return;

        auto &workers = *pending_workers.find(symbol_id);
        for (usize n = 0; n < workers.count(); ++n) {
            Worker worker            = workers[n];
            Worker_Status new_status = worker.resume(this);
            if (new_status == Worker_Status::Done ||
                new_status == Worker_Status::Failed) {
                workers.remove(n);
            }
        }

        if (workers.count() == 0) {
            pending_workers.erase(symbol_id);
        }
    }
};

Semantic_Storage semastore_init(Mini_Allocator allocator);
void semastore_init_builtin_types(Semantic_Storage *store);

Semantic_Context semactx_init(Mini_Allocator allocator,
                              Diagnostic_Pool *diagnostics,
                              Semantic_Storage *store);
void semactx_resolve(Semantic_Context *sema, Program *program);
