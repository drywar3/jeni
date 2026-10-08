#pragma once

#include "expr.h"
#include "misc.h"
#include "stmt.h"
#include "type.h"

#include <mini.cc/array.h>

namespace ast
{

    struct If_Branch {
        Expression *condition;
        Statement *then;
    };

    namespace stmt
    {
        struct Variable : Statement {
            Name name;
            Mutability mutability;
            bool type_is_defined;
            Typehint *typehint;
            bool is_initialized;
            Expression *initializer;
        };

        struct Block : Statement {
            using Body = MINI_ARRAY(StatementPointer);
            Body body;
        };

        struct If : Statement {
            using Branches = mini::Array<ast::If_Branch>;
            Expression *condition;
            Statement *then;
            Branches branches;
            Statement *else_;
        };

        struct Return : Statement {
            Expression *value;
        };

        struct For_Ever : Statement {
            Statement *body;
        };

        struct Break : Statement {};

        template <typename Derived_Statement>
        static inline Statement *
        alloc_statement(Mini_Allocator allocator, Statement_Kind kind,
                        Locus locus, Derived_Statement s)
        {
            Statement *statement =
                (Statement *)MINI_ALLOC(allocator, Derived_Statement);
            new (statement) Derived_Statement(std::move(s));
            statement->kind  = kind;
            statement->locus = locus;
            return statement;
        }

    } // namespace stmt
} // namespace ast
