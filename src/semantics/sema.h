#pragma once

#include "worker.h"
#include "ast/ast.h"
#include "misc/map.h"
#include "diagnostic.h"
#include "semantics/entities/type.h"
#include "semantics/entities/scope.h"

#include <mini.c/mini_def.h>
#include <mini.c/array.h>

struct SemanticStorage {
    sema::ScopeStorage scopes;
    sema::SymbolStorage symbols;
    sema::TypeStorage types;
};

struct SemanticContext {
    /* maps [symbol id] -> pending workers */ /* maps [symbol id] -> pending
                                                 symbol ids */
    HashMap<usize, MINI_ARRAY(Worker)> pending_workers;

    DiagnosticPool *diagnostics;
    Mini_Allocator allocator;
    SemanticStorage *store;

    sema::ScopeId global_scope;
    sema::ScopeId current_scope;

    auto &types() { return store->types; }
    auto &scopes() { return store->scopes; }
    auto &symbols() { return store->symbols; }

    const auto &types() const { return store->types; }
    const auto &scopes() const { return store->scopes; }
    const auto &symbols() const { return store->symbols; }
};

SemanticStorage semastore_init(Mini_Allocator allocator);
void semastore_init_builtin_types(SemanticStorage *store);

SemanticContext semactx_init(Mini_Allocator allocator,
                             DiagnosticPool *diagnostics,
                             SemanticStorage *store);
void semactx_resolve(SemanticContext *sema, Program *program);
