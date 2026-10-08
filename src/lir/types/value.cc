#include "lir/types/value.h"

bool lir::Value::operator==(const Value &other) const
{
    if (kind != other.kind)
        return false;

    switch (kind) {
    case Kind::Integer:
        return integer == other.integer;
    case Kind::LocalRef:
    case Kind::ParamRef:
        return index == other.index;
    case Kind::Deref:
        return valueid == other.valueid;
    case Kind::GlobalRef:
        return ident == other.ident;
    case Kind::CString:
        return string == other.string;
    case Kind::Cmp:
        return cmp.op == other.cmp.op &&
            cmp.first == other.cmp.first &&
            cmp.second == other.cmp.second;
    default: MINI_UNREACHABLE();
    }
}

lir::ValueId lir::valuestore_index(ValueStorage *valuestore, Value value)
{
    usize n = 0;
    for (const Value &value_ : valuestore->iter()) {
        if (value_ == value) {
            return lir::ValueId(n);
        }
        n += 1;
    }
    valuestore->append(value);
    return lir::ValueId(n);
}
