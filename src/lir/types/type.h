#pragma once

#include "ast/misc.h"

#include <new>
#include <memory>
#include <mini.cc/array.h>
#include <mini.c/allocator.h>

template <typename T> static inline T *Box(Mini_Allocator allocator, T &value)
{
    T *mem = MINI_ALLOC(allocator, T);
    new (mem) T(std::move(value));
    return mem;
}

namespace lir
{
    struct Type;
    using TypePtr = Type *;

    enum struct Type_Id : usize {};

    struct Type {
        struct Pointer {
            TypePtr inner;
            Mutability mutability;
        };

        struct Function {
            TypePtr return_type;
            mini::Array<TypePtr> parameters{};
        };

        enum class Kind {
            Void,
            Int8,
            Int32,
            Int64,
            Uint32,
            Float32,
            Float64,
            String,
            Pointer,
            Function,
            Struct,
        };

        Kind kind;

        union {
            Pointer pointer;
            Function function;
            lir::Type_Id type_id;
        };

        static TypePtr PointerTo(Mini_Allocator allocator,
                                 Mutability mutability, TypePtr type)
        {
            Type ptr_type{};
            ptr_type.kind               = Kind::Pointer;
            ptr_type.pointer.inner      = type;
            ptr_type.pointer.mutability = mutability;
            return Box(allocator, ptr_type);
        }

        static TypePtr FunctionPointer(Mini_Allocator allocator,
                                       TypePtr return_type,
                                       mini::Array<TypePtr> params)
        {
            Type fn_type{};
            fn_type.kind = Kind::Function;
            fn_type.function.return_type = return_type;
            fn_type.function.parameters  = params;
            return Box(allocator, fn_type);
        }

        static TypePtr Struct(Mini_Allocator allocator, lir::Type_Id type_id)
        {
            Type struct_type{};
            struct_type.kind = Kind::Struct;
            struct_type.type_id = type_id;
            return Box(allocator, struct_type);
        }
    };

    struct Type_Member {
        mini::StringView name; // name of the member
        lir::TypePtr     type; // type of the member
    };

    struct Type_Builder {
        using Members = mini::Array<Type_Member>;

        mini::StringView name; // name of the type
        Members          members;

        Type_Builder(Mini_Allocator allocator, mini::StringView name);

        void add_member(lir::Type_Member member);

        lir::Type_Member *get_member(mini::StringView name);
        const lir::Type_Member *get_member(mini::StringView name) const;
    };

    using Type_Storage = mini::Array<Type_Builder>;
} // namespace lir
