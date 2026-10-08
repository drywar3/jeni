#include "hir/convert.h"
#include "hir/types/expr.h"

hir::Expression *hir::convert_binary_op(hir::Context *ctx,
                                            const ExprBinaryOperation *binop)
{
    hir::Expression *left  = hir::convert_expression(ctx, binop->left);
    hir::Expression *right = hir::convert_expression(ctx, binop->right);
    if (binop->op == AstOperator::Assign) {
        return ctx->new_expr(hir::Expression::Kind::Assign,
                             sema::TypeId(*ctx->types().get_id(binop->base.locus)),
                             hir::expr::Assign{left, right});
    }

    return ctx->new_expr(hir::Expression::Kind::BinaryOperation,
                         sema::TypeId(*ctx->types().get_id(binop->base.locus)),
                         hir::expr::BinaryOperation{left, right, binop->op});
}
