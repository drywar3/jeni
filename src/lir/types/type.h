#pragma once

#include <memory>
#include <mini.c/allocator.h>

template<typename T>
static inline T *Box(Mini_Allocator allocator, T &value)
{
    T *mem = MINI_ALLOC(allocator, T);
    *mem   = value;
    return mem;
}

namespace lir
{
    struct Type;
    using TypePtr = Type*;

    struct Type {
        enum Kind {
            Void,
            Int32,
            Uint32,
            String,
            Pointer,
        };

        Kind kind;

        union {
            TypePtr pointer_to;
        };

        static TypePtr PointerTo(Mini_Allocator allocator, TypePtr type)
        {
            Type ptr_type;
            ptr_type.kind = Kind::Pointer;
            ptr_type.pointer_to = type;
            return Box(allocator, ptr_type);
        }
    };

} // namespace lir
