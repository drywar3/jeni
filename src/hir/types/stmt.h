#pragma once

#include "hir/types/expr.h"
#include "semantics/entities/type.h"

#include <mini.cc/string_view.h>

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
            };

            mini::StringView    name;
            Function::Prototype prototype;
            hir::Statement     *body;
        };

        struct Block {
            using Body = MINI_ARRAY(Statement*);
            Body body;
        };
    } // namespace stmt

    struct Statement {
        enum struct Kind {
            Variable,
            Function,
            Block,
        };

        Kind kind;
        union {
            stmt::Variable variable;
            stmt::Function function;
            stmt::Block    block;
        } as;
    };
} // namespace hir
