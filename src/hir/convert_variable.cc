#include "ast/statements.h"
#include "ast/expressions.h"
#include "hir/convert_variable.h"
#include "hir/convert_function.h"

hir::Statement *hir::convert_variable_stmt(hir::Context *ctx, const ::Statement *stmt)
{
    auto *variable     = (ast::stmt::Variable*)stmt;

    if (variable->is_initialized) {
        switch (variable->initializer->kind) {
        case EXPR_Function:         return hir::convert_function_stmt(ctx, variable);
        case EXPR_Struct:           return hir::convert_struct_decl(ctx, variable);
        case EXPR_Namespace:        return hir::convert_namespace(ctx, variable);
        default:                    break;
        }
    }

    const auto *symbol = ctx->symbols().find(variable->name.locus);
    MINI_ASSERT(symbol != nullptr,);

    hir::stmt::Variable hir_variable{};
    hir_variable.name = symbol->profile.canonical_name;
    hir_variable.mutability  = symbol->variable.mutability;
    hir_variable.type_id     = sema::Type_Id(*ctx->types().get_id(symbol->locus));
    if (variable->is_initialized) {
        hir_variable.initializer = hir::convert_expression(ctx, variable->initializer);
    }

    return ctx->new_stmt(hir::Statement::Kind::Variable, hir_variable);
}
