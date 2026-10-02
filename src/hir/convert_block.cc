#include "hir/convert.h"
#include "hir/types/stmt.h"

hir::Statement *hir::convert_block(hir::Context *ctx, StmtBlock *block)
{
    hir::stmt::Block hir_block;
    hir_block.body = MINI_ARRAY_INIT(ctx->allocator, hir::Statement *);

    for (usize n = 0; n < mini_array_count(block->body); ++n) {
        const ::Statement *stmt = block->body[n];
        mini_array_append(hir_block.body, hir::convert_statement(ctx, stmt));
    }

    return ctx->new_stmt(hir::Statement::Kind::Block, block);
}
