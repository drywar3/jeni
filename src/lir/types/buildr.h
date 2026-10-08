#pragma once

#include "lir/types/function.h"
#include "lir/types/global.h"
#include "lir/types/type.h"
#include "mini.cc/string_view.h"
#include "semantics/sema.h"

namespace lir
{
    struct Function;
    struct Module;


    struct Loop_Point {
        lir::Label continue_label;
        lir::Label break_label;
    };

    using Loop_Point_Stack = mini::Array<Loop_Point>;

    struct Buildr /* intentional spelling */ {
        Module *mod;
        /* note: this will eventually stale when mod->globals reallocates. */
        Function *function;
        BlockId current_block;

        Loop_Point_Stack loop_point_stack;

        void add_global(lir::Global glob);

        /* create and add a parameter to the function */
        ValueId parameter(TypePtr type, mini::StringView name);

        void new_block();
        void end_block();

        Mini_Allocator allocator() const;
        const Semantic_Storage *store() const;

        const sema::Type &get_type_layout(sema::Type_Id type_id)
        {
            return store()->types.at_index(usize(type_id));
        }

        void push_loop_point(lir::Loop_Point point);
        Opt<lir::Loop_Point> pop_loop_point();

        ValueId get_const_true();
        ValueId get_const_false();

        ValueId create_integer(int64 value);
        ValueId create_boolean(bool value);
        ValueId create_cstring(mini::StringView value);
        ValueId create_cmp(lir::CmpOp op, ValueId first, ValueId second);

        void create_ret(lir::TypePtr type, ValueId value);

        usize new_local(mini::StringView name);

        std::optional<lir::Local> find_local(mini::StringView name);

        ValueId create_local_ref(Local local);
        ValueId create_glob_ref(mini::StringView name);
        ValueId create_alloca(mini::StringView name, lir::TypePtr type);
        void create_store(ValueId dst, lir::TypePtr type, ValueId value);
        ValueId create_deref(ValueId value);
        ValueId create_temporary(lir::TypePtr type);
        ValueId create_call(lir::TypePtr type, ValueId callee,
                            mini::Array<ValueId> args);

        lir::Label new_label();
        void put_label(lir::Label label);
        void jmp_if_eq(ValueId first, ValueId second, lir::Label label);
        void jmp_to_label(lir::Label label);
    };
} // namespace lir
