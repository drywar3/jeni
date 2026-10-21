#include "compeval/value.h"

compeval::Value::Value()
    : kind(Kind::Null)
{
}

compeval::Value::Value(int64 value)
    : kind(Kind::Int)
{
    integer = value;
}

compeval::Value::Value(bool  value)
    : kind(Kind::Bool)
{
    boolean = value;
}

compeval::Value::Value(double  value)
    : kind(Kind::Float)
{
    floating = value;
}

compeval::Value::Value(compeval::Value::Symbol_Id value)
    : kind(Kind::Symbol)
{
    symbol_id = value;
}
