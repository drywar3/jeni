#include "ast/statements.h"
#include "ast/expressions.h"
#include "hir/convert_variable.h"
#include "hir/convert_function.h"

hir::Statement *hir::convert_variable_stmt(hir::Context *ctx, const ::Statement *stmt)
{
    auto *variable     = (ast::stmt::Variable*)stmt;
    if (variable->is_initialized &&
        variable->initializer->is(EXPR_Function))
        return hir::convert_function_stmt(ctx, variable);

    const auto *symbol = ctx->symbols().find(variable->name.locus);

    MINI_ASSERT(symbol != nullptr,);

    hir::stmt::Variable hir_variable;
    hir_variable.name = variable->name.value;
    hir_variable.mutability  = symbol->variable.mutability;
    hir_variable.type_id     = *symbol->variable.type_id;
    if (variable->is_initialized) {
        hir_variable.initializer = hir::convert_expression(ctx, variable->initializer);
    }

    return ctx->new_stmt(hir::Statement::Kind::Variable, hir_variable);
}
