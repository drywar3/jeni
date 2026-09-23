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
        typeof(derived) derived_tmp  = derived;                         \
        StatementPointer statement = (StatementPointer)MINI_ALLOC(allocator, typeof(derived_tmp)); \
        *((typeof(derived_tmp)*)statement) = derived;\
        statement_ctor(statement, kind, locus);                       \
        statement;                                                     \
    })

static inline void statement_ctor(StatementPointer this,
                                  StatementKind kind,
                                  Locus locus) {
    this->kind  = kind;
    this->locus = locus;
}


void statement_destroy(Statement *stmt, Mini_Allocator allocator);

const char *statement_name(StatementKind kind);
