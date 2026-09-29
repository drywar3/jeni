#include "ast/statements.h"
#include "hir/convert_variable.h"

hir::Statement *hir::convert_variable_stmt(hir::Context *ctx, const ::Statement *stmt)
{
    auto *variable     = (StmtVariable*)stmt;
    const auto *symbol = ctx->symbols().find(variable->name.locus);

    MINI_ASSERT(symbol != nullptr,);

    hir::stmt::Variable hir_variable;
    hir_variable.name = variable->name.value;
    hir_variable.mutability = symbol->as.variable.mutability;
    hir_variable.type_id    = *symbol->as.variable.type_id;

    return ctx->new_stmt(hir::Statement::Kind::Variable, hir_variable);
}
