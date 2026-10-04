#pragma once

#include "ast/misc.h"
#include <memory>
#include <mini.c/allocator.h>
#include <new>

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

        enum class Kind {
            Void,
            Int8,
            Int32,
            Uint32,
            String,
            Pointer,
        };

        Kind kind;

        union {
            Pointer pointer;
        };

        static TypePtr PointerTo(Mini_Allocator allocator,
                                 Mutability mutability, TypePtr type)
        {
            Type ptr_type;
            ptr_type.kind               = Kind::Pointer;
            ptr_type.pointer.inner      = type;
            ptr_type.pointer.mutability = mutability;
            return Box(allocator, ptr_type);
        }
    };

} // namespace lir
