#include "hir/convert.h"
#include "hir/convert_variable.h"

static hir::Statement *convert_statement(hir::Context *ctx,
                                         const Statement *stmt);

hir::Context hir::ctx_init(Mini_Allocator allocator,
                           const SemanticStorage *store)
{
    return {allocator, store};
}

hir::Program hir::program_from_raw(Context *ctx, ::Program ast)
{
    auto *program = MINI_ARRAY_INIT(ctx->allocator, hir::Statement *);
    for (usize n = 0; n < mini_array_count(ast.ast); ++n) {
        const ::Statement *stmt = ast.ast[n];
        auto *hir_stmt          = hir::convert_statement(ctx, stmt);
        mini_array_append(program, hir_stmt);
    }
    return program;
}

hir::Statement *hir::convert_statement(hir::Context *ctx,
                                       const ::Statement *stmt)
{
    switch (stmt->kind) {
    case STMT_Variable: return convert_variable_stmt(ctx, stmt);
    case STMT_Block:    return convert_block(ctx, (StmtBlock*)stmt);
    default: MINI_UNREACHABLE();
    }
}
