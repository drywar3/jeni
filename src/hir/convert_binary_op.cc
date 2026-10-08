#include "hir/convert.h"
#include "hir/types/expr.h"

hir::Expression *
hir::convert_binary_op(hir::Context *ctx,
                       const ast::expr::Binary_Operation *binop)
{
    hir::Expression *left  = hir::convert_expression(ctx, binop->left);
    hir::Expression *right = hir::convert_expression(ctx, binop->right);
    if (binop->op == ast::Operator::Assign) {
        return ctx->new_expr(hir::Expression::Kind::Assign,
                             sema::Type_Id(*ctx->types().get_id(binop->locus)),
                             hir::expr::Assign{left, right});
    }

    return ctx->new_expr(hir::Expression::Kind::Binary_Operation,
                         sema::Type_Id(*ctx->types().get_id(binop->locus)),
                         hir::expr::Binary_Operation{left, right, binop->op});
}
