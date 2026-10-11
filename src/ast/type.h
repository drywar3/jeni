#pragma once

#include "misc.h"
#include "ast/expr.h"

typedef enum Typehint_Kind {
    TYPEHINT_Integer,
    TYPEHINT_Float,
    TYPEHINT_Char,
    TYPEHINT_String,
    TYPEHINT_Bool,
    TYPEHINT_Void,
    TYPEHINT_Array,
    TYPEHINT_Slice,
    TYPEHINT_Pointer,
    TYPEHINT_Function
} Typehint_Kind;

struct Typehint : Expression {
    Typehint_Kind kind;
    Typehint_Kind get_type_kind() const { return kind; }
    void set_type_kind(Typehint_Kind kind) { this->kind = kind; }
};

void typehint_destroy(Typehint *stmt, Mini_Allocator allocator);
