#pragma once

#include <mini.cc/string_view.h>

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

        struct FunctionCall {
            Expression *callee;
            mini::Array<Expression *> arguments;
        };
    } // namespace expr

    struct Expression {
        enum struct Kind {
            Integer,
            Identifier,
            FunctionCall,
            CString,
        };

        Kind kind;
        sema::TypeId type_id;

        union {
            expr::Integer integer;
            expr::Identifier identifier;
            expr::FunctionCall function_call;
            expr::String string;
        } as;
    };
} // namespace hir
