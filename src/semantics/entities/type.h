#pragma once

#include "ast/misc.h"
#include "mini.c/array.h"
#include "mini.c/string.h"
#include "mini.cc/array.h"
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

        bool operator==(const Type &other) const
        {
            if (kind != other.kind)
                return false;

            if (kind == TypeKind::Error) return false;

            if (kind == TypeKind::Function) {
                if (function.return_type != other.function.return_type)
                    return false;
                usize n = 0;
                for (const auto &param_type :
                     mini::iterate(function.parameters)) {
                    const auto &other_type = other.function.parameters[n];
                    if (param_type != other_type)
                        return false;
                    n += 1;
                }
            }

            return true;
        }

        /* for initialization in global contexts */
        constexpr Type() : kind(TypeKind::Error) {}
        constexpr Type(TypeKind kind) : kind(kind) {}

        static auto Function(FunctionType::Parameters parameters,
                             TypeId return_type)
        {
            auto type     = Type(TypeKind::Function);
            type.function = FunctionType{parameters, return_type};
            return type;
        }

        Mini_String display(Mini_Allocator a, TypeStorage &types) const
        {
            switch (kind) {
            case TypeKind::Void:
                return mini_string_build(a, "void");

            case TypeKind::String:
                return mini_string_build(a, "string");
            case TypeKind::Int:
                return mini_string_build(a, "int");
            case TypeKind::Uint:
                return mini_string_build(a, "uint");
            case TypeKind::Function: {
                Mini_String output = mini_string_build(a, "func(");
                for (usize n = 0; n < mini_array_count(function.parameters);
                     ++n) {
                    if (n != 0)
                        mini_string_append_string(&output, ", ");
                    mini_string_append_string(
                        &output, types.at_index(usize(function.parameters[n]))
                                     .display(a, types));
                }
                mini_string_append_fmt(
                    &output, ") -> %s",
                    types.at_index(usize(function.return_type))
                        .display(a, types));
                return output;
            } break;
            case TypeKind::Error:
                return mini_string_build(a, "!error!");
            default:
                MINI_UNREACHABLE("%d", kind.kind);
            }
        }
    };

    struct TypeInfo {
        TypeId id;
        Mutability mutability;
    };

} // namespace sema
