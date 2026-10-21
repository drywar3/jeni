#pragma once

#include "parser/token.h"

namespace parser
{
    struct State {
        enum Kind {
            Global,
            If_Condition,
            Typehint_Expression,
            Namespace_Definition,
        };

        Kind kind;
        Token begin;
        bool has_error;

        State(Kind kind) : kind(kind), has_error(false) {}
    };
} // namespace  parser
