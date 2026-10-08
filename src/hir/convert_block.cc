#include "hir/convert.h"
#include "hir/types/stmt.h"

hir::Statement *hir::convert_block(hir::Context *ctx, ast::stmt::Block *block)
{
    hir::stmt::Block hir_block;
    hir_block.body = hir::stmt::Block::Body(ctx->allocator);

    for (usize n = 0; n < mini_array_count(block->body); ++n) {
        const ::Statement *stmt = block->body[n];
        hir_block.body.append(hir::convert_statement(ctx, stmt));
    }

    auto *stmt = ctx->new_stmt(hir::Statement::Kind::Block, hir_block);
    return stmt;
}
