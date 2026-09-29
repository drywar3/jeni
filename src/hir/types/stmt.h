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
        };
    } // namespace stmt

    struct Statement {
        enum struct Kind {
            Variable,
            Function,
        };

        Kind kind;
        union {
            stmt::Variable variable;
        } as;
    };
} // namespace hir
