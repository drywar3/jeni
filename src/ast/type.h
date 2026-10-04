#pragma once

#include "misc.h"

typedef enum TypehintKind {
    TYPEHINT_Integer,
    TYPEHINT_Char,
    TYPEHINT_String,
    TYPEHINT_Bool,
    TYPEHINT_Void,
    TYPEHINT_Array,
    TYPEHINT_Slice,
    TYPEHINT_Pointer,
    TYPEHINT_Function
} TypehintKind;

typedef struct Typehint {
    TypehintKind kind;
    Locus locus;
} Typehint;

typedef Typehint *TypehintPointer;

#define ALLOC_TYPE(allocator, kind, locus, derived)                            \
    ({                                                                         \
        typeof(derived) derived_tmp = derived;                                 \
        TypehintPointer typehint =                                             \
            (TypehintPointer)MINI_ALLOC(allocator, typeof(derived_tmp));       \
        *((typeof(derived_tmp) *)typehint) = derived;                          \
        typehint_ctor(typehint, kind, locus);                                  \
        typehint;                                                              \
    })

static inline void typehint_ctor(TypehintPointer _this, TypehintKind kind,
                                 Locus locus)
{
    _this->kind  = kind;
    _this->locus = locus;
}

void typehint_destroy(Typehint *stmt, Mini_Allocator allocator);
