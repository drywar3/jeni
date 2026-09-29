#pragma once

#include <mini.cc/string_view.h>

namespace hir
{
    namespace expr
    {
        struct Integer {
            int64 value;
        };

        struct Identifier {
            mini::StringView value;
        };
    } // namespace expr

    struct Expression {
        enum struct Kind {
            Integer,
            Identifier,
        };

        Kind kind;
        union {
            expr::Integer    integer;
            expr::Identifier identifier;
        };
    };
} // namespace hir
