#pragma once

#include "misc/map.h"
#include "semantics/entities/symbol.h"

namespace compeval
{
    struct Call_Frame {
        using Env = HashMap<sema::Symbol_Id, compeval::Value>;
        Env env;
    };
} // namespace compeval
