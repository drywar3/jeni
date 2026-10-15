#include "hir/convert.h"

hir::Statement *hir::convert_for_ever_stmt(hir::Context *ctx,
                                           const ast::stmt::For_Ever *for_ever)
{
    hir::stmt::Loop loop{};
    loop.condition = ctx->create_true_expr();
    loop.body      = hir::convert_statement(ctx, for_ever->body);
    return ctx->new_stmt(hir::Statement::Kind::Loop, loop);
}

hir::Statement *hir::convert_while_loop(hir::Context *ctx,
                                        const ast::stmt::While_Loop *loop_)
{
    hir::stmt::Loop loop{};
    loop.condition = hir::convert_expression(ctx, loop_->condition);
    loop.body      = hir::convert_statement(ctx, loop_->body);
    return ctx->new_stmt(hir::Statement::Kind::Loop, loop);
}
