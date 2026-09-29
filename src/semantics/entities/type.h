#pragma once

#include "ast/misc.h"
#include "parser/locus.h"

namespace sema
{
    struct TypeKind {
        enum V {
            Error,
            _SignedInt,
            Int,
            _SignedIntEnd,
            _UnsignedInt,
            Uint,
            _UnsignedIntEnd,
            String,
            Char,
            Void,
            Bool,
            Array,
            Pointer,
            Slice,
        } kind;

        constexpr TypeKind(V v) : kind(v) {}

        operator V() const { return kind; }

        bool is_signed_integer() const
        {
            return kind > V::_SignedInt && kind < V::_SignedIntEnd;
        }

        bool is_unsigned_integer() const
        {
            return kind > V::_UnsignedInt && kind < V::_UnsignedIntEnd;
        }
    };

    struct Type;
    using TypeStorage = DenseMap<Locus, Type>;

    struct Type {
        TypeKind kind;

        union {

        } data;

        bool operator==(const Type &other) const { return kind == other.kind; }

        /* for initialization in global contexts */
        constexpr Type() : kind(TypeKind::Error) {}
        constexpr Type(TypeKind kind) : kind(kind) {}

        Mini_String display(Mini_Allocator a, TypeStorage &types) const
        {
            switch (kind) {
            case TypeKind::String:
                return mini_string_build(a, "string");
            case TypeKind::Int:
                return mini_string_build(a, "int");
            case TypeKind::Uint:
                return mini_string_build(a, "uint");
            case TypeKind::Error:
                return mini_string_build(a, "!error!");
            default:
                MINI_UNREACHABLE();
            }
        }
    };

    enum class TypeId : usize {};

    struct TypeInfo {
        TypeId id;
        Mutability mutability;
    };

} // namespace sema
