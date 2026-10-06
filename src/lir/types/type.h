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
            String,
            Pointer,
            Function,
        };

        Kind kind;

        union {
            Pointer pointer;
            Function function;
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
    };

} // namespace lir
