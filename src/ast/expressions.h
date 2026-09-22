#pragma once

#include "expr.h"
#include "misc.h"
#include "stmt.h"
#include "type.h"

#include <mini.c/array.h>

typedef struct ExprInteger {
    Expression base;
    int64      value;
} ExprInteger;

typedef struct ExprIdentifier {
    Expression base;
    Mini_StringView value;
} ExprIdentifier;

typedef struct AstFunctionParameter {
    Name      name;
    Typehint *typehint;
} AstFunctionParameter;

typedef struct AstFunctionPrototype {
    MINI_ARRAY(AstFunctionParameter) parameters;
    Typehint *return_type;
} AstFunctionPrototype;

typedef struct ExprFunction {
    Expression base;
    AstFunctionPrototype prototype;
    Statement           *body;
} ExprFunction;

typedef enum AstOperator {
    AST_BINOP_Add = TOKEN_OP_Add,
    AST_BINOP_Sub = TOKEN_OP_Minus,
    AST_BINOP_Mul = TOKEN_OP_Star,
    AST_BINOP_Div = TOKEN_OP_Div,
    AST_BINOP_Gt  = TOKEN_OP_Greater,
    AST_BINOP_Lt  = TOKEN_OP_Less,
    AST_BINOP_Equals = TOKEN_OP_Equals,
    AST_BINOP_NotEquals = TOKEN_OP_NotEquals,
} AstOperator;

typedef struct ExprBinaryOperation {
    Expression base;
    AstOperator op;
    ExpressionPointer left;
    ExpressionPointer right;
} ExprBinaryOperation;

typedef struct ExprUnaryOperation {
    Expression base;
    AstOperator op;
    ExpressionPointer expression;
} ExprUnaryOperation;

typedef struct AstFunctionCallArgument {
    bool is_positional;
    ExpressionPointer argument;
    /* some_function_name(:some_parameter_name argument) */
    Name name;
} AstFunctionCallArgument;

typedef struct ExprFunctionCall {
    Expression base;
    ExpressionPointer callee;
    MINI_ARRAY(AstFunctionCallArgument) arguments;
} ExprFunctionCall;
