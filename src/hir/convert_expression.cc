#include "ast/expr.h"
#include "ast/expressions.h"
#include "hir/convert.h"
#include "hir/types/expr.h"
#include "mini.cc/string_view.h"

hir::Expression *hir::convert_expression(hir::Context *ctx,
                                         const ::Expression *expression)
{
    switch (expression->kind) {
    case EXPR_Integer: {
        const ExprInteger *integer = (const ExprInteger *)expression;
        hir::expr::Integer hir_integer;
        hir_integer.value = integer->value;
        return ctx->new_expr(
            hir::Expression::Kind::Integer,
            sema::TypeId(*ctx->types().get_id(expression->locus)), hir_integer);
    } break;
    case EXPR_CString: {
        const ExprString *string = (const ExprString *)expression;
        hir::expr::String hir_string;
        hir_string.value = string->value.substr(1, string->value.base().length - 1);
        return ctx->new_expr(
            hir::Expression::Kind::CString,
            sema::TypeId(*ctx->types().get_id(expression->locus)), hir_string);
    } break;
    case EXPR_Identifier: {
        const ExprIdentifier *ident = (const ExprIdentifier *)expression;
        hir::expr::Identifier hir_ident;
        hir_ident.value = ident->value;
        return ctx->new_expr(
            hir::Expression::Kind::Identifier,
            sema::TypeId(*ctx->types().get_id(expression->locus)), hir_ident);
    } break;
    case EXPR_FunctionCall:
        return convert_function_call(ctx, (const ExprFunctionCall *)expression);
    default:
        MINI_UNREACHABLE();
    }
}
