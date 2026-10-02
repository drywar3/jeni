#pragma once

#include "hir/convert.h"
#include "semantics/sema.h"
#include "lir/types/value.h"
#include "lir/types/global.h"
#include "lir/types/buildr.h"

#include <mini.cc/array.h>
#include <mini.c/allocator.h>

namespace lir
{
    struct Context {
        ValueStorage           values;
        BlockStorage           blocks;
        const SemanticStorage *store;
        Mini_Allocator         allocator;

        BlockId new_block(std::optional<BlockId> parent);
        Block  *get_block(BlockId id);
    };

    struct Module {
        using Globals = mini::Array<Global>;

        Context *context;
        Globals  globals;

        Buildr buildr()
        {
            return Buildr{.mod = this, .function = nullptr };
        }
    };

    Context ctx_init(Mini_Allocator allocator,
                     const SemanticStorage *store);
    void ctx_destroy(Context *context);

    Module module_init(Context *context);
    void inflate_module(Module *mod, const hir::Program program);


    void lower_statement(Buildr *b, const hir::Statement *stmt);
    ValueId lower_expression(Buildr *b, const hir::Expression *expression);
    TypePtr lower_type(Buildr *b, sema::TypeId type_id);
} // namespace lir
