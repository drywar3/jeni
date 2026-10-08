#include "hir/convert.h"

hir::Statement *hir::convert_if_stmt(hir::Context *ctx,
                                     const StmtIf *if_stmt)
{
    hir::stmt::If if_{};
    if_.branches  = {ctx->allocator};
    if_.condition = hir::convert_expression(ctx, if_stmt->condition);
    if_.then      = hir::convert_statement(ctx, if_stmt->then);

    for (const auto &branch_ : if_stmt->branches.iter()) {
        hir::stmt::If::Branch branch{};
        branch.condition = hir::convert_expression(ctx, branch_.condition);
        branch.then      = hir::convert_statement(ctx, branch_.then);
        if_.branches.append(branch);
    }

    if (if_stmt->else_) {
        if_.else_ = hir::convert_statement(ctx, if_stmt->else_);
    }

    return ctx->new_stmt(hir::Statement::Kind::If, if_);
}
