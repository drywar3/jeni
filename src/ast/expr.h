#pragma once

#include "locus.h"

typedef enum {
    EXPR_Identifier,
    EXPR_Integer,
    EXPR_String,
    EXPR_Bool,
    EXPR_Float,
    EXPR_Initializer,
    EXPR_Binop,
    EXPR_Unary,
    EXPR_Cast,
    EXPR_Function,
    EXPR_FunctionCall,
} ExpressionKind;

typedef struct {
    ExpressionKind kind;
    Locus         locus;
} Expression;

typedef Expression *ExpressionPointer;

#define ALLOC_EXPR(allocator, kind, locus, derived)                     \
    ({                                                                  \
        ExpressionPointer expression = expression_alloc(allocator, kind, locus, sizeof(derived)); \
        typeof(derived) *derived_ptr = (typeof(derived) *)expression;     \
        *derived_ptr = derived;                                         \
        expression;\
    })

static inline Expression *expression_alloc(Mini_Allocator allocator,
                                           ExpressionKind kind,
                                           Locus locus,
                                           usize size) {
    Expression *expression = (Expression*)MINI_ALLOC_MANY(allocator, char, size);
    expression->kind        = kind;
    expression->locus       = locus;
    return expression;
}

void expression_destroy(Expression *stmt, Mini_Allocator allocator);
