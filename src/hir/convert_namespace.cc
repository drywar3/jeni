#include "hir/convert.h"

hir::Statement *hir::convert_namespace(hir::Context *ctx, ast::stmt::Variable *var)
{
    auto *ns = (ast::expr::Namespace *)var->initializer;
    auto *block = hir::convert_statement(ctx, ns->definitions);
    block->kind = hir::Statement::Group;
    return block;
}
