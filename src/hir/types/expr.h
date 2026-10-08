#pragma once

#include <mini.cc/string_view.h>

#include "mini.cc/array.h"
#include "ast/expressions.h"
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

        struct FunctionCall {
            Expression *callee;
            mini::Array<Expression *> arguments;
        };

        struct Assign {
            Expression *receiver;
            Expression *value;
        };

        struct BinaryOperation {
            Expression *left;
            Expression *right;
            AstOperator op;
        };
    } // namespace expr

    struct Expression {
        enum struct Kind {
            Integer,
            Identifier,
            FunctionCall,
            CString,
            BinaryOperation,
            Assign,
        };

        Kind kind;
        sema::TypeId type_id;

        union {
            expr::Integer integer;
            expr::Identifier identifier;
            expr::FunctionCall function_call;
            expr::String string;
            expr::BinaryOperation binop;
            expr::Assign assign;
        } as;
    };
} // namespace hir
