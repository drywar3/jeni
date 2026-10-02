#pragma once

#include "lir/types/function.h"

namespace lir
{
    struct Global {
        using Function = ::lir::Function;
        struct Variable {
            lir::TypePtr     type;
            ValueId          initializer;
        };

        enum struct Kind {
            Variable,
            Function,
        };

        mini::StringView name;
        Kind kind;
        union {
            Variable variable;
            Function function;
        };
    };
} // namespace lir
