#pragma once

#include "type.h"
#include "expr.h"

typedef struct TypePointer {
    Typehint base;
    TypehintPointer typehint;
} TypePointer;

typedef struct TypeArray {
    Typehint base;
    ExpressionPointer element_count;
    TypehintPointer   element_type;
} TypeArray;

typedef struct TypeInteger {
    Typehint base;
    enum {
        AST_TYPE_INT_Int,
        AST_TYPE_INT_Uint,
        AST_TYPE_INT_Usize,
        AST_TYPE_INT_Isize,
    } kind;
} TypeInteger;
