#pragma once

#include "ast/misc.h"
#include "parser/locus.h"
#include "semantics/store.h"
#include "hir/types/profile.h"
#include "semantics/entities/type_id.h"
#include "semantics/entities/symbol_id.h"

#include <mini.c/array.h>
#include <mini.cc/array.h>
#include <mini.c/string.h>

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
            Uint8,
            _UnsignedIntEnd,
            String,
            Char,
            Void,
            Bool,
            Array,
            Pointer,
            Slice,
            Function,
            Namespace,
            Struct,
            Float32,
            Float64,
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

        bool is_some_type() const;
    };

    struct Type;

    struct Pointer_Type {
        Mutability mutability;
        Type_Id target_type;
    };

    struct Function_Type {
        using Parameters = mini::Array<Type_Id>;
        Parameters parameters;
        Type_Id return_type;
    };

    struct Struct_Type {
        struct Field_Data {
            Name name;
            sema::Type_Id type_id;
        };

        using Fields = mini::Array<Field_Data>;

        Name   name;
        Fields fields;

        Opt<Field_Data> get_field_data(mini::StringView field_name) const;
    };

    struct Array_Type {
        sema::Type_Id element_type;
        // this should work for now.
        usize         element_count;
    };

    struct Type {
        Type_Kind kind;
        Opt<sema::Symbol_Id> symbol_id;

        union {
            Function_Type function;
            Pointer_Type  pointer;
            Struct_Type   strukt;
            Array_Type    array;
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
            } else if (kind == Type_Kind::Struct) {
                return strukt.name.value == other.strukt.name.value &&
                    strukt.name.locus == other.strukt.name.locus;
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

        static auto Struct(Name name, Struct_Type::Fields fields)
        {
            auto type    = Type(Type_Kind::Struct);
            type.strukt = Struct_Type{name, fields};
            return type;
        }

        static auto Array(usize element_count, sema::Type_Id element_type)
        {
            auto type  = Type(Type_Kind::Array);
            type.array = Array_Type{element_type, element_count};
            return type;
        }

        Mini_String display(Mini_Allocator a, Semantic_Storage &store) const;
    };
} // namespace sema
