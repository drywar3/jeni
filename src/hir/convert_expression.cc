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
        const auto *integer = (const ast::expr::Integer *)expression;
        const auto type_id  = ctx->types().get_id(expression->locus);
        MINI_ASSERT(type_id.has_value(), "missing type link");
        hir::expr::Integer hir_integer;
        hir_integer.value = integer->value;
        return ctx->new_expr(hir::Expression::Kind::Integer,
                             sema::Type_Id(*type_id), hir_integer);
    } break;
    case EXPR_CString: {
        const ast::expr::String *string = (const ast::expr::String *)expression;
        hir::expr::String hir_string;
        hir_string.value =
            string->value.substr(1, string->value.base().length - 1);
        return ctx->new_expr(
            hir::Expression::Kind::CString,
            sema::Type_Id(*ctx->types().get_id(expression->locus)), hir_string);
    } break;
    case EXPR_Identifier: {
        const ast::expr::Identifier *ident =
            (const ast::expr::Identifier *)expression;
        hir::expr::Identifier hir_ident;
        hir_ident.value = ident->value;
        return ctx->new_expr(
            hir::Expression::Kind::Identifier,
            sema::Type_Id(*ctx->types().get_id(expression->locus)), hir_ident);
    } break;
    case EXPR_Function_Call:
        return convert_function_call(
            ctx, (const ast::expr::Function_Call *)expression);
    case EXPR_Binop:
        return convert_binary_op(
            ctx, (const ast::expr::Binary_Operation *)expression);
    default:
        MINI_UNREACHABLE();
    }
}
