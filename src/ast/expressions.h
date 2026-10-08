#pragma once

#include "expr.h"
#include "mini.c/string_view.h"
#include "misc.h"
#include "stmt.h"
#include "type.h"
#include "parser/token.h"

#include <mini.cc/array.h>

struct ExprInteger {
    Expression base;
    int64 value;
};

struct ExprIdentifier {
    Expression base;
    Mini_StringView value;
};

struct ExprString {
    Expression base;
    mini::StringView value;
};

struct AstFunctionParameter {
    Name name;
    Typehint *typehint;
    Expression *default_expression;
};

struct AstFunctionPrototype {
    using Parameters = mini::Array<AstFunctionParameter>;

    Parameters parameters;
    Typehint *return_type;
    /* consider moving this to its own struct because the
     * variadic mark carry a name also
     *
     * ```
     * foobar :: func(x : int, args:...) {
     * ```
     *
     * but it is all still speculations. for now this is ok.
     */
    bool      is_variadic = false;
};

struct ExprFunction {
    Expression base;

    AstFunctionPrototype prototype;
    Statement *body;
    bool body_is_defined;
};

enum struct AstOperator : uint {
    Add               = TOKEN_OP_Add,
    Sub               = TOKEN_OP_Minus,
    Mul               = TOKEN_OP_Star,
    Div               = TOKEN_OP_Div,
    Equals            = TOKEN_OP_Equals,
    NotEquals         = TOKEN_OP_NotEquals,
    Assign            = TOKEN_OP_Assign,
    LessThanEquals    = TOKEN_OP_LessEq,
    GreaterThanEquals = TOKEN_OP_GreaterEq,
    LessThan          = TOKEN_OP_Less,
    GreaterThan       = TOKEN_OP_Greater,
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
    bool is_positional = true;
    ExpressionPointer argument;
    /* some_function_name(:some_parameter_name argument) */
    Name name;
};

struct ExprFunctionCall {
    Expression base;
    ExpressionPointer callee;
    mini::Array<AstFunctionCallArgument> arguments;
};

struct ExprError{ Expression base; };
