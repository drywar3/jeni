#include "hir/convert.h"

hir::Statement *hir::convert_for_ever_stmt(hir::Context *ctx,
                                           const ast::stmt::For_Ever *for_ever)
{
    hir::stmt::Loop loop{};
    loop.condition = ctx->create_true_expr();
    loop.body      = hir::convert_statement(ctx, for_ever->body);
    return ctx->new_stmt(hir::Statement::Kind::Loop, loop);
}
