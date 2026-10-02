#pragma once

#include <mini.cc/array.h>
#include <mini.cc/string_view.h>

namespace lir
{
    struct Value {
        enum struct Kind {
            Integer,
            String,
            LocalRef,
            ParamRef,
        };

        Kind kind;
        usize index;
        union {
            int64            integer;
            mini::StringView ident;
            mini::StringView string;
        };

        static Value Integer(int64 value)
        {
            return Value{.kind = Kind::Integer, .integer = value};
        }

        static Value LocalRef(usize index)
        {
            return Value{.kind = Kind::LocalRef, .index = index };
        }

        static Value ParamRef(usize index)
        {
            return Value{.kind = Kind::ParamRef, .index = index };
        }


        static Value GlobalRef(mini::StringView ident)
        {
            return Value{.kind = Kind::LocalRef, .ident = ident };
        }

        bool operator==(const Value &other) const;
    };
    enum struct ValueId {};

    using ValueStorage = mini::Array<Value>;

    ValueStorage valuestore_init(Mini_Allocator allocator);
    void valuestore_destroy(ValueStorage *valuestore);

    ValueId valuestore_index(ValueStorage *valuestore, Value value);

    const Value *valuestore_get(const ValueStorage *valuestore, ValueId id);
    Value *valuestore_get(ValueStorage *valuestore, ValueId id);

} // namespace lir
