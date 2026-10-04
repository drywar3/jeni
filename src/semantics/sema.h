#pragma once

#include "worker.h"
#include "ast/ast.h"
#include "misc/map.h"
#include "diagnostic.h"
#include "semantics/entities/type.h"
#include "semantics/entities/scope.h"
#include "semantics/entities/call_schema.h"

#include <mini.c/mini_def.h>
#include <mini.c/array.h>

struct SemanticStorage {
    using CallSchemas = HashMap<sema::SymbolId, sema::FunctionCallSchema>;

    sema::ScopeStorage  scopes;
    sema::SymbolStorage symbols;
    sema::TypeStorage   types;
    CallSchemas         call_schemas;
};

struct SemanticContext {
    /* maps [symbol id] -> pending workers */
    HashMap<sema::SymbolId, MINI_ARRAY(Worker)> pending_workers;

    using BlockSavePoints = HashMap<Locus, usize>;

    DiagnosticPool *diagnostics;
    Mini_Allocator allocator;
    SemanticStorage *store;

    sema::ScopeId global_scope;
    sema::ScopeId current_scope;
    BlockSavePoints block_save_points;

    auto &types() { return store->types; }
    auto &scopes() { return store->scopes; }
    auto &symbols() { return store->symbols; }

    const auto &types() const { return store->types; }
    const auto &scopes() const { return store->scopes; }
    const auto &symbols() const { return store->symbols; }

    sema::FunctionCallSchema* get_call_schema(sema::SymbolId symbol_id);
    void set_call_schema(sema::SymbolId symbol_id, sema::FunctionCallSchema schema);

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

    void register_worker(sema::SymbolId id, Worker worker)
    {
        MINI_ARRAY(Worker) workers;
        if (pending_workers.contains(id)) {
            workers = pending_workers[id];
        } else {
            workers = MINI_ARRAY_INIT(allocator, Worker);
            pending_workers[id] = workers;
        }
        mini_array_append(workers, worker);
    }

    void wake_up_workers(sema::SymbolId symbol_id)
    {
        if (!pending_workers.contains(symbol_id))
            return;

        Worker *workers = *pending_workers.find(symbol_id);
        for (usize n = 0; n < mini_array_count(workers); ++n) {
            Worker worker = workers[n];
            WorkerStatus new_status =
                worker.func(this, worker.data, true);
            if (new_status == WorkerStatus::Done || new_status == WorkerStatus::Failed) {
                mini_array_remove(workers, n);
            }
        }

        if (mini_array_count(workers) == 0) {
            pending_workers.erase(symbol_id);
        }
    }
};

SemanticStorage semastore_init(Mini_Allocator allocator);
void semastore_init_builtin_types(SemanticStorage *store);

SemanticContext semactx_init(Mini_Allocator allocator,
                             DiagnosticPool *diagnostics,
                             SemanticStorage *store);
void semactx_resolve(SemanticContext *sema, Program *program);
