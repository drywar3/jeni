#pragma once

#include <mini.cc/string_view.h>

#include "ast/expressions.h"
#include "mini.cc/array.h"
#include "semantics/entities/type.h"

namespace hir
{
    struct Expression;
    namespace expr
    {
        struct Integer {
            int64 value;
        };

        struct Identifier {
            mini::StringView value;
        };

        struct String {
            mini::StringView value;
        };

        struct Function_Call {
            Expression *callee;
            mini::Array<Expression *> arguments;
        };

        struct Assign {
            Expression *receiver;
            Expression *value;
        };

        struct Binary_Operation {
            Expression *left;
            Expression *right;
            ast::Operator op;
        };

        struct Boolean {
            bool value;
        };
    } // namespace expr

    struct Expression {
        enum struct Kind {
            Integer,
            Identifier,
            Function_Call,
            CString,
            Binary_Operation,
            Assign,
            Boolean,
        };

        Kind kind;
        sema::Type_Id type_id;

        union {
            expr::Integer integer;
            expr::Identifier identifier;
            expr::Function_Call function_call;
            expr::String string;
            expr::Binary_Operation binop;
            expr::Assign assign;
            expr::Boolean boolean;
        } as;
    };
} // namespace hir
