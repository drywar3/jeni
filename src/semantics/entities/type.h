#pragma once

#include "ast/misc.h"
#include "parser/locus.h"

namespace sema
{
    enum struct TypeKind {
        Int,
        Uint,
        String,
        Char,
        Void,
        Bool,
        Array,
        Pointer,
        Slice,
    };

    struct Type {
        TypeKind kind;

        union {

        } data;

        bool operator==(const Type &other) const { return kind == other.kind; }
    };

    enum class TypeId : usize {};

    struct TypeInfo {
        TypeId id;
        Mutability mutability;
    };

    using TypeStorage = DenseMap<Locus, Type>;
} // namespace sema
