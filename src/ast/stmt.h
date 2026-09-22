#pragma once

#include "locus.h"
#include <mini.c/allocator.h>

typedef enum {
    STMT_Variable,
    STMT_Assign,
    STMT_If,
    STMT_For,
    STMT_While,
    STMT_Forever,
    STMT_Block,
    STMT_Defer,
    STMT_Expr,
} StatementKind;

typedef struct {
    StatementKind kind;
    Locus         locus;
} Statement;

typedef Statement *StatementPointer;

#define ALLOC_STMT(allocator, kind, locus, derived)                     \
    ({                                                                  \
        StatementPointer statement = statement_alloc(allocator, kind, locus, sizeof(derived)); \
        typeof(derived) *derived_ptr = (typeof(derived) *)statement;   \
        *derived_ptr = derived;                                         \
        statement;\
    })


static inline Statement *statement_alloc(Mini_Allocator allocator,
                           StatementKind kind,
                           Locus locus,
                           usize size) {
    Statement *statement = (Statement*)MINI_ALLOC_MANY(allocator, char, size);
    statement->kind        = kind;
    statement->locus       = locus;
    return statement;
}

void statement_destroy(Statement *stmt, Mini_Allocator allocator);

const char *statement_name(StatementKind kind);
