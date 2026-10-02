#pragma once

#include "semantics/sema.h"
#include "lir/types/type.h"
#include "lir/types/function.h"

namespace lir
{
    struct Function;
    struct Module;

    struct Buildr /* intentional spelling */ {
        Module   *mod;
        Function *function;
        BlockId   current_block;

        /* create and add a parameter to the function */
        ValueId parameter(TypePtr type, mini::StringView name);

        void new_block();
        void end_block();

        Mini_Allocator allocator() const;
        const SemanticStorage *store() const;

        const sema::Type &get_type_layout(sema::TypeId type_id)
        {
            return store()->types.at_index(usize(type_id));
        }

        ValueId create_integer(int64 value);

        usize new_local(mini::StringView name);

        std::optional<lir::Local> find_local(mini::StringView name);

        ValueId create_local_ref(Local local);
        ValueId create_glob_ref(mini::StringView name);
        ValueId create_alloca(mini::StringView name, lir::TypePtr type);
        void    create_store(ValueId dst, lir::TypePtr type, ValueId value);
    };
} // namespace lir
