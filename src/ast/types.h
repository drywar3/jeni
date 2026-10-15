#pragma once

#include "ast/misc.h"
#include "type.h"
#include "expr.h"


namespace ast
{
    namespace type
    {
        struct String    : Typehint {};
        struct Char      : Typehint {};
        struct Void      : Typehint {};
        struct Boolean   : Typehint {};
        struct Float  : Typehint {
            enum Width {
                Float32,
                Float64,
            } width;
        };

        struct Pointer : Typehint {
            Mutability mutability;
            Expression  *typehint;
        };

        struct Array : Typehint {
            Expression *element_count;
            Expression *element_type;
        };

        struct Slice : Typehint {
            Expression *element_type;
        };

        struct Integer : Typehint {
            enum Kind {
                Int,
                Uint,
                Uint8,
                Usize,
                Isize,
                Int64,
            } kind;
        };


        template <typename Derived_Typehint>
        static inline Typehint *
        alloc_typehint(Mini_Allocator allocator, Typehint_Kind kind,
                       Locus locus, Derived_Typehint e)
        {
            Typehint *expression =
                (Typehint *)MINI_ALLOC(allocator, Derived_Typehint);
            new (expression) Derived_Typehint(std::move(e));
            expression->kind  = kind;
            expression->locus = locus;
            return expression;
        }


    } // namespace type
} // namespace ast
