#pragma once

#include "parser/locus.h"
#include <mini.c/allocator.h>

enum Statement_Kind {
    STMT_Variable,
    STMT_Assign,
    STMT_If,
    STMT_For,
    STMT_While,
    STMT_Forever,
    STMT_Block,
    STMT_Defer,
    STMT_Expr,
    STMT_Return,
    STMT_For_Ever,
    STMT_Break,
    STMT_Continue,
};

struct Statement {
    Statement_Kind kind;
    Locus locus;

    void set_stmt_kind(Statement_Kind kind) { this->kind = kind; }

    Statement_Kind get_stmt_kind() const { return kind; }
};

typedef Statement *StatementPointer;

void statement_destroy(Statement *stmt, Mini_Allocator allocator);

const char *statement_name(Statement_Kind kind);
