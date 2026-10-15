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
    case EXPR_Float: {
        const auto *float_ = (const ast::expr::Float *)expression;
        const auto type_id  = ctx->types().get_id(expression->locus);
        MINI_ASSERT(type_id.has_value(), "missing type link");
        hir::expr::Float hir_float;
        hir_float.value = float_->value;
        return ctx->new_expr(hir::Expression::Kind::Float,
                             sema::Type_Id(*type_id), hir_float);
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
    case EXPR_Subscript: {
        auto *subscript = (const ast::expr::Subscript *)expression;
        auto expr       = hir::convert_expression(ctx, subscript->expr);
        auto index      = hir::convert_expression(ctx, subscript->index);
        auto type_id = sema::Type_Id(*ctx->types().get_id(expression->locus));
        return ctx->new_expr(
            hir::Expression::Kind::Dereference, type_id,
            hir::expr::Dereference{ctx->new_expr(
                hir::Expression::Kind::Binary_Operation,
                type_id,
                hir::expr::Binary_Operation{expr, index, ast::Operator::Add})});
    } break;
    case EXPR_Member_Access:
        return convert_member_access(ctx, (const ast::expr::Member_Access *)expression);
    case EXPR_Dereference: {
        auto *deref = (ast::expr::Dereference *)expression;
        auto *value = hir::convert_expression(ctx, deref->value);
        auto type_id = sema::Type_Id(*ctx->types().get_id(expression->locus));
        return ctx->new_expr(hir::Expression::Kind::Dereference,
                             type_id, hir::expr::Dereference{value});
    };
    case EXPR_Address_Of: {
        auto *addr_of = (ast::expr::AddressOf *)expression;
        auto *value   = hir::convert_expression(ctx, addr_of->value);
        auto type_id = sema::Type_Id(*ctx->types().get_id(expression->locus));
        return ctx->new_expr(hir::Expression::Kind::AddressOf,
                             type_id, hir::expr::AddressOf{value});
    };
    case EXPR_Unary: {
        auto *unary  = (ast::expr::Unary_Operation *)expression;
        auto *value  = hir::convert_expression(ctx, unary->expression);
        auto type_id = sema::Type_Id(*ctx->types().get_id(expression->locus));

        if (unary->op == ast::Operator::Not) {
            return ctx->new_expr(hir::Expression::Kind::Binary_Operation,
                                 type_id, hir::expr::Binary_Operation{.op = ast::Operator::NotEquals,
                                                                      .left = value, .right = ctx->create_true_expr()});
        }

        MINI_UNREACHABLE();
    };
    default:
        MINI_UNREACHABLE();
    }
}
