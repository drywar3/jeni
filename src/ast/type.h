#pragma once

#include "misc.h"

typedef enum TypehintKind {
    TYPEHINT_Integer,
    TYPEHINT_Char,
    TYPEHINT_Bool,
    TYPEHINT_Void,
    TYPEHINT_Array,
    TYPEHINT_Slice,
    TYPEHINT_Pointer,
    TYPEHINT_Function
} TypehintKind;

typedef struct Typehint {
    TypehintKind kind;
    Locus        locus;
    Mutability   mutability;
} Typehint;

typedef Typehint *TypehintPointer;

#define ALLOC_TYPE(allocator, kind, locus, mut, derived)                \
    ({                                                                  \
        TypehintPointer typehint = typehint_alloc(allocator, kind, locus, mut, sizeof(derived)); \
        typeof(derived) *derived_ptr = (typeof(derived) *)typehint;     \
        *derived_ptr = derived;                                         \
        typehint;\
    })

static inline Typehint *typehint_alloc(Mini_Allocator allocator,
                                       TypehintKind kind,
                                       Locus locus,
                                       Mutability mutability,
                                       usize size) {
    Typehint *typehint   = (Typehint *)MINI_ALLOC_MANY(allocator, char, size);
    typehint->kind       = kind;
    typehint->locus      = locus;
    typehint->mutability = mutability;
    return typehint;
}
void typehint_destroy(Typehint *stmt, Mini_Allocator allocator);
