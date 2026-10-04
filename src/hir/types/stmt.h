#pragma once

#include "hir/types/expr.h"
#include "semantics/entities/type.h"

#include <mini.cc/string_view.h>
#include <mini.cc/array.h>

namespace hir
{
    struct Statement;

    namespace stmt
    {
        struct Variable {
            mini::StringView name;
            Mutability       mutability;
            sema::TypeId     type_id;
            Expression      *initializer;
        };

        struct Function {
            struct Parameter {
                mini::StringView name;
                sema::TypeId     type_id;
            };

            using Parameters = MINI_ARRAY(Function::Parameter);

            struct Prototype {
                Function::Parameters parameters;
                sema::TypeId         return_type;
                bool                 is_variadic = false;
            };

            mini::StringView    name;
            Function::Prototype prototype;
            hir::Statement     *body;
            bool                body_is_defined;
        };

        struct Block {
            using Body = mini::Array<Statement*>;
            Body body;
        };
    } // namespace stmt

    struct Statement {
        enum Kind {
            Variable,
            Function,
            Block,
            Expr,
        };

        Kind kind;
        union {
            stmt::Variable variable;
            stmt::Function function;
            stmt::Block    block;
            hir::Expression *expr;
        } as;
    };
} // namespace hir
