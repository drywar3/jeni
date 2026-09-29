#pragma once

#include <mini.c/allocator.h>

#include "ast/misc.h"
#include "hir/types/stmt.h"
#include "semantics/sema.h"

namespace hir
{
    using Program = MINI_ARRAY(hir::Statement*);

    struct Context {
        Mini_Allocator         allocator;
        const SemanticStorage *store;

        auto &types() { return store->types; }
        auto &scopes() { return store->scopes; }
        auto &symbols() { return store->symbols; }

        const auto &types() const { return store->types; }
        const auto &scopes() const { return store->scopes; }
        const auto &symbols() const { return store->symbols; }

        template<typename T>
        hir::Statement *new_stmt(hir::Statement::Kind kind, T obj) {
            hir::Statement *stmt = MINI_ALLOC(allocator, hir::Statement);
            T *mem               = (T*)&stmt->as;
            *mem = obj;
            return stmt;
        }
    };

    Context ctx_init(Mini_Allocator allocator, const SemanticStorage *store);
    Program program_from_raw(Context *ctx, ::Program ast);
} // namespace hir
