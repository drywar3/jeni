#pragma once

#include "worker.h"
#include "source.h"
#include "ast/ast.h"
#include "misc/map.h"
#include "diagnostic.h"
#include "compeval/engine.h"
#include "semantics/entities/type.h"
#include "semantics/entities/scope.h"
#include "semantics/entities/call_schema.h"
#include "semantics/store.h"

#include <mini.c/array.h>
#include <mini.c/mini_def.h>

struct Semantic_Context {
    /* maps [symbol id] -> pending workers */
    HashMap<sema::Symbol_Id, mini::Array<Worker>> pending_workers;

    using Block_Save_Points = HashMap<Locus, usize>;

    Diagnostic_Pool *diagnostics;

    Mini_Allocator allocator;
    Mini_Allocator persistent_allocator;

    Semantic_Storage *store;
    SourceManager *sources;
    compeval::Engine *vm;

    Opt<sema::Type_Id>   type_context = None;
    Opt<sema::Symbol_Id> current_namespace = None;

    sema::Semantic_Id           my_id;
    SourceId                    source_id;
    HashMap<mini::StringView, Locus> loaded_programs;

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

    Opt<sema::Type_Id> get_current_type_context() const;
    Opt<sema::Type_Id> set_current_type_context(Opt<sema::Type_Id> context);

    Opt<sema::Symbol_Id> get_current_namespace() const;
    Opt<sema::Symbol_Id> set_current_namespace(Opt<sema::Symbol_Id> ns);


    template<typename RT, typename Fn>
    RT perform_with_type_context_with_result(Opt<sema::Type_Id> context, Fn fn) {
        auto save = set_current_type_context(context);
        RT result = fn();
        set_current_type_context(save);
        return result;
    }

    template<typename Fn>
    void perform_with_type_context(Opt<sema::Type_Id> context, Fn fn) {
        auto save = set_current_type_context(context);
        fn();
        set_current_type_context(save);
    }

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
            Worker_Status new_status = worker.resume(store);
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
                              Mini_Allocator persistent_allocator,
                              Diagnostic_Pool *diagnostics,
                              Semantic_Storage *store,
                              SourceManager *sources,
                              compeval::Engine *vm,
                              sema::Semantic_Id sema_id,
                              SourceId source_id);

void semactx_resolve(Semantic_Context *sema, Program *program);
