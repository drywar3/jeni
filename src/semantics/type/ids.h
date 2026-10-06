#pragma once

#include "semantics/entities/type.h"

namespace sema::type_id
{
    constexpr auto Error  = (TypeId)0;
    constexpr auto Void   = (TypeId)1;
    constexpr auto Int    = (TypeId)2;
    constexpr auto Uint   = (TypeId)3;
    constexpr auto Bool   = (TypeId)4;
    constexpr auto Char   = (TypeId)5;
    constexpr auto String = (TypeId)6;

    constexpr auto Int64 = (TypeId)7;

    constexpr auto _LAST_ = (TypeId)8;
} // namespace sema::type_id
