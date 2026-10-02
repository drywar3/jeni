#pragma once

#include <mini.cc/string_view.h>

#include "semantics/entities/type.h"

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
        sema::TypeId type_id;

        union {
            expr::Integer    integer;
            expr::Identifier identifier;
        } as;
    };
} // namespace hir
