#pragma once

#include "hir/convert.h"
#include "hir/types/expr.h"
#include "lir/types/buildr.h"
#include "lir/types/function.h"
#include "lir/types/global.h"
#include "lir/types/value.h"
#include "semantics/sema.h"

#include <mini.c/allocator.h>
#include <mini.cc/array.h>

namespace lir
{
    struct Context {
        lir::ValueStorage values;
        lir::BlockStorage blocks;
        lir::Type_Storage types;

        const Semantic_Storage *store;
        Mini_Allocator allocator;

        lir::ValueId const_true;
        lir::ValueId const_false;

        lir::BlockId new_block(std::optional<BlockId> parent);
        lir::Block *get_block(lir::BlockId id);

        lir::Type_Id new_type(lir::Type_Builder type_builder);
        lir::Type_Builder *get_type(lir::Type_Id type_id);
        lir::Type_Builder *get_type(mini::StringView name);
        lir::Type_Id get_id_of_type(mini::StringView name);
    };

    struct Module {
        using Globals = mini::Array<Global>;

        Context *context;
        Globals globals;

        auto allocator() const { return context->allocator; }

        Buildr new_buildr()
        {
            return Buildr{
                .mod = this, .function = nullptr, .current_block = BlockId(0)};
        }
    };

    Context ctx_init(Mini_Allocator allocator, const Semantic_Storage *store);
    void ctx_destroy(Context *context);

    Module module_init(Context *context);
    void inflate_module(Module *mod, const hir::Program program);

    void lower_statement(Buildr *b, const hir::Statement *stmt);
    ValueId lower_expression(Buildr *b, const hir::Expression *expression);
    ValueId lower_function_call(Buildr *b, const hir::Expression *expression);
    TypePtr lower_type(Buildr *b, sema::Type_Id type_id);

    lir::Global lower_glob_function(lir::Buildr *b, const hir::Statement *stmt);
} // namespace lir
