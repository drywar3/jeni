#pragma once

#include "ast/stmt.h"
#include "parser/locus.h"

enum Expression_Kind {
    EXPR_Identifier,
    EXPR_Integer,
    EXPR_String,
    EXPR_CString,
    EXPR_Bool,
    EXPR_Float,
    EXPR_Initializer,
    EXPR_Binop,
    EXPR_Unary,
    EXPR_Cast,
    EXPR_Function,
    EXPR_Function_Call,
    EXPR_Error,
};

struct Expression : Statement {
    Expression_Kind kind;
    // Locus locus;

    bool is(Expression_Kind kind) const { return this->kind == kind; }

    bool is_error() const { return is(EXPR_Error); }
};

typedef Expression *ExpressionPointer;

void expression_destroy(Expression *stmt, Mini_Allocator allocator);
