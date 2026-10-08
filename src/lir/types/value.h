#pragma once

#include <mini.cc/array.h>
#include <mini.cc/string_view.h>

namespace lir
{
    enum struct ValueId {};
    enum struct CmpOp {
        Equals,
        NotEquals,
        LessThan,
        GreaterThan,
        LessThanEquals,
        GreaterThanEquals,

        /* todo: hack */
        Sub,
        Add,
        Mul,
        Div,
    };

    struct Value {

        struct Cmp {
            CmpOp op;
            ValueId first;
            ValueId second;
        };

        enum struct Kind {
            Integer,
            String,
            CString,
            LocalRef,
            GlobalRef,
            ParamRef,
            Deref,
            True,
            False,
            Cmp,
        };

        Kind kind;
        usize index;
        union {
            int64            integer;
            mini::StringView ident;
            mini::StringView string;
            ValueId          valueid;
            Cmp cmp;
        };

        static Value True()
        {
            return Value{.kind = Kind::True};
        }

        static Value False()
        {
            return Value{.kind = Kind::False};
        }

        static Value Integer(int64 value)
        {
            return Value{.kind = Kind::Integer, .integer = value};
        }

        static Value CString(mini::StringView value)
        {
            return Value{.kind = Kind::CString, .string = value};
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

        static Value Cmp(CmpOp op, ValueId first, ValueId second)
        {
            return Value{.kind = Kind::Cmp, .cmp = {op, first, second}};
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
