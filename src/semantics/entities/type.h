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
            Function,
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

    enum class TypeId : usize {};

    struct FunctionType {
        using Parameters = MINI_ARRAY(TypeId);
        Parameters parameters;
        TypeId return_type;
    };

    struct Type {
        TypeKind kind;

        union {
            FunctionType function;
        };

        bool operator==(const Type &other) const { return kind == other.kind; }

        /* for initialization in global contexts */
        constexpr Type() : kind(TypeKind::Error) {}
        constexpr Type(TypeKind kind) : kind(kind) {}

        static auto Function(FunctionType::Parameters parameters, TypeId return_type) {
            auto type = Type(TypeKind::Function);
            type.function = FunctionType{parameters, return_type};
            return type;
        }

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

    struct TypeInfo {
        TypeId id;
        Mutability mutability;
    };

} // namespace sema
