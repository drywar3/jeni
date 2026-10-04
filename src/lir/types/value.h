#pragma once

#include <mini.cc/array.h>
#include <mini.cc/string_view.h>

namespace lir
{
    enum struct ValueId {};
    struct Value {
        enum struct Kind {
            Integer,
            String,
            LocalRef,
            GlobalRef,
            ParamRef,
            Deref,
        };

        Kind kind;
        usize index;
        union {
            int64            integer;
            mini::StringView ident;
            mini::StringView string;
            ValueId          valueid;
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
            return Value{.kind = Kind::GlobalRef, .ident = ident };
        }

        static Value Deref(ValueId value)
        {
            return Value{.kind = Kind::Deref, .valueid = value };
        }

        bool operator==(const Value &other) const;
    };

    using ValueStorage = mini::Array<Value>;

    ValueStorage valuestore_init(Mini_Allocator allocator);
    void valuestore_destroy(ValueStorage *valuestore);

    ValueId valuestore_index(ValueStorage *valuestore, Value value);

    const Value *valuestore_get(const ValueStorage *valuestore, ValueId id);
    Value *valuestore_get(ValueStorage *valuestore, ValueId id);

} // namespace lir
