#pragma once

#include "hir/types/expr.h"
#include "semantics/entities/type.h"

#include <mini.cc/array.h>
#include <mini.cc/string_view.h>

namespace hir
{
    struct Statement;

    namespace stmt
    {
        struct Variable {
            mini::StringView name;
            Mutability mutability;
            sema::Type_Id type_id;
            Expression *initializer;
        };

        struct Function {
            struct Parameter {
                mini::StringView name;
                sema::Type_Id type_id;
            };

            using Parameters = MINI_ARRAY(Function::Parameter);

            struct Prototype {
                Function::Parameters parameters;
                sema::Type_Id return_type;
                bool is_variadic = false;
            };

            mini::StringView name;
            Function::Prototype prototype;
            hir::Statement *body;
            bool body_is_defined;
        };

        struct Block {
            using Body = mini::Array<Statement *>;
            Body body;
        };

        struct If {
            struct Branch {
                Expression *condition;
                Statement *then;
            };

            using Branches = mini::Array<Branch>;

            Expression *condition;
            Branches branches;
            Statement *then;
            Statement *else_;
        };

        struct Return {
            sema::Type_Id type_id;
            Expression *value;
        };

        struct Break {};

        struct Loop {
            Expression *condition;
            Statement  *body;
        };
    } // namespace stmt

    struct Statement {
        enum Kind {
            Variable,
            Function,
            Block,
            Expr,
            If,
            Return,
            Loop,
            Break,
        };

        Kind kind;
        union {
            stmt::Variable variable;
            stmt::Function function;
            stmt::Block block;
            stmt::If if_;
            stmt::Return ret;
            stmt::Break  break_;
            stmt::Loop loop;
            hir::Expression *expr;
        } as;
    };
} // namespace hir
