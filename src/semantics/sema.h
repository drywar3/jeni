#pragma once

#include "worker.h"
#include "ast/ast.h"
#include "diagnostic.h"
#include "semantics/entities/scope.h"

#include <mini.c/mini_def.h>
#include <mini.c/array.h>
#include <unordered_map>

constexpr ScopeId GLOBAL_SCOPE{0};

typedef struct SemanticContext {
    /* maps [symbol id] -> pending workers */    /* maps [symbol id] -> pending symbol ids */
    std::unordered_map<usize, MINI_ARRAY(Worker)> pending_workers;

    DiagnosticPool *diagnostics;
    Mini_Allocator allocator;
    ScopeStorage   scopes;
    SymbolStorage  symbols;

    ScopeId        current_scope;
} SemanticContext;

SemanticContext semactx_init(Mini_Allocator allocator, DiagnosticPool *diagnostics);
void semactx_resolve(SemanticContext *sema, Program *program);
