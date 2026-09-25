#pragma once

#include "parser/locus.h"

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
        typeof(derived) derived_tmp  = derived;                         \
        ExpressionPointer expression = (ExpressionPointer)MINI_ALLOC(allocator, typeof(derived_tmp)); \
        *((typeof(derived_tmp)*)expression) = derived;\
        expression_ctor(expression, kind, locus);                       \
        expression;                                                     \
    })

static inline void expression_ctor(ExpressionPointer _this,
                                   ExpressionKind kind,
                                   Locus locus) {
    _this->kind  = kind;
    _this->locus = locus;
}

void expression_destroy(Expression *stmt, Mini_Allocator allocator);
