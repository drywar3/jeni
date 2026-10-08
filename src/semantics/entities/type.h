#pragma once

#include "ast/misc.h"
#include "mini.c/array.h"
#include "mini.c/string.h"
#include "mini.cc/array.h"
#include "parser/locus.h"

namespace sema
{
    struct Type_Kind {
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

        constexpr Type_Kind(V v) : kind(v) {}

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
    using Type_Storage = DenseMap<Locus, Type>;

    enum class Type_Id : usize {};

    struct Pointer_Type {
        Mutability mutability;
        Type_Id target_type;
    };

    struct Function_Type {
        using Parameters = mini::Array<Type_Id>;
        Parameters parameters;
        Type_Id return_type;
    };

    struct Type {
        Type_Kind kind;

        union {
            Function_Type function;
            Pointer_Type pointer;
        };

        bool operator==(const Type &other) const
        {
            if (kind != other.kind)
                return false;

            if (kind == Type_Kind::Error)
                return false;

            if (kind == Type_Kind::Function) {
                if (function.return_type != other.function.return_type)
                    return false;
                usize n = 0;
                for (const auto &param_type : function.parameters.iter()) {
                    const auto &other_type = other.function.parameters[n];
                    if (param_type != other_type)
                        return false;
                    n += 1;
                }
            } else if (kind == Type_Kind::Pointer) {
                return pointer.mutability == other.pointer.mutability &&
                       pointer.target_type == other.pointer.target_type;
            }

            return true;
        }

        /* for initialization in global contexts */
        constexpr Type() : kind(Type_Kind::Error) {}
        constexpr Type(Type_Kind kind) : kind(kind) {}

        static auto Function(Function_Type::Parameters parameters,
                             Type_Id return_type)
        {
            auto type     = Type(Type_Kind::Function);
            type.function = Function_Type{parameters, return_type};
            return type;
        }

        static auto Pointer(Mutability mutability, Type_Id type_id)
        {
            auto type    = Type(Type_Kind::Pointer);
            type.pointer = Pointer_Type{mutability, type_id};
            return type;
        }

        Mini_String display(Mini_Allocator a, Type_Storage &types) const;
    };
} // namespace sema
