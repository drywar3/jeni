#pragma once

#include <mini.c/mini_def.h>

namespace compeval
{
    struct Value {
        enum class Symbol_Id : usize {};

        enum Kind {
            Null,
            Int,
            Bool,
            String,
            Array,
            Float,
            Symbol,
        };

        Kind kind;

        union {
            int64      integer;
            double     floating;
            bool       boolean;
            Symbol_Id  symbol_id;
        };

        Value();
        Value(int64  value);
        Value(double  value);
        Value(bool   value);
        Value(Symbol_Id value);
    };
} // namespace compeval
