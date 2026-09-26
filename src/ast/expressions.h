#pragma once

#include "expr.h"
#include "misc.h"
#include "stmt.h"
#include "type.h"
#include "parser/token.h"

#include <mini.c/array.h>

struct ExprInteger {
    Expression base;
    int64      value;
};

struct ExprIdentifier {
    Expression base;
    Mini_StringView value;
};

struct AstFunctionParameter {
    Name      name;
    Typehint *typehint;
};

struct AstFunctionPrototype {
    using Parameters = MINI_ARRAY(AstFunctionParameter);
    Parameters parameters;
    Typehint *return_type;
};

struct ExprFunction {
    Expression base;

    AstFunctionPrototype prototype;
    Statement           *body;
    bool                 body_is_defined;
};

enum struct AstOperator : uint {
    Add = TOKEN_OP_Add,
    Sub = TOKEN_OP_Minus,
    Mul = TOKEN_OP_Star,
    Equals = TOKEN_OP_Equals,
    NotEquals = TOKEN_OP_NotEquals,
};

struct ExprBinaryOperation {
    Expression base;
    AstOperator op;
    ExpressionPointer left;
    ExpressionPointer right;
};

struct ExprUnaryOperation {
    Expression base;
    AstOperator op;
    ExpressionPointer expression;
};

struct AstFunctionCallArgument {
    bool is_positional;
    ExpressionPointer argument;
    /* some_function_name(:some_parameter_name argument) */
    Name name;
};

struct ExprFunctionCall {
    Expression base;
    ExpressionPointer callee;
    MINI_ARRAY(AstFunctionCallArgument) arguments;
};
