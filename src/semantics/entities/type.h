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
            Int64,
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

    struct PointerType {
        Mutability mutability;
        TypeId target_type;
    };

    struct FunctionType {
        using Parameters = mini::Array<TypeId>;
        Parameters parameters;
        TypeId return_type;
    };

    struct Type {
        TypeKind kind;

        union {
            FunctionType function;
            PointerType pointer;
        };

        bool operator==(const Type &other) const
        {
            if (kind != other.kind)
                return false;

            if (kind == TypeKind::Error)
                return false;

            if (kind == TypeKind::Function) {
                if (function.return_type != other.function.return_type)
                    return false;
                usize n = 0;
                for (const auto &param_type :
                         function.parameters.iter()) {
                    const auto &other_type = other.function.parameters[n];
                    if (param_type != other_type)
                        return false;
                    n += 1;
                }
            } else if (kind == TypeKind::Pointer) {
                return pointer.mutability == other.pointer.mutability &&
                       pointer.target_type == other.pointer.target_type;
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

        static auto Pointer(Mutability mutability, TypeId type_id)
        {
            auto type    = Type(TypeKind::Pointer);
            type.pointer = PointerType{mutability, type_id};
            return type;
        }

        Mini_String display(Mini_Allocator a, TypeStorage &types) const;
    };

    struct TypeInfo {
        TypeId id;
        Mutability mutability;
    };

} // namespace sema
