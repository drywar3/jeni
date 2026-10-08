#pragma once

#include "semantics/entities/type.h"

namespace sema::type_id
{
    constexpr auto Error  = (Type_Id)0;
    constexpr auto Void   = (Type_Id)1;
    constexpr auto Int    = (Type_Id)2;
    constexpr auto Uint   = (Type_Id)3;
    constexpr auto Bool   = (Type_Id)4;
    constexpr auto Char   = (Type_Id)5;
    constexpr auto String = (Type_Id)6;

    constexpr auto Int64 = (Type_Id)7;

    constexpr auto _LAST_ = (Type_Id)8;
} // namespace sema::type_id
