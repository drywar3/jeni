#include "hir/convert_function.h"
#include "ast/expressions.h"
#include "hir/types/stmt.h"
#include "semantics//type/ids.h"

static hir::stmt::Function::Prototype
convert_function_prototype(hir::Context *ctx,
                           const ast::Function_Prototype *prototype)
{
    hir::stmt::Function::Prototype hir_prototype;
    hir_prototype.is_variadic = prototype->is_variadic;
    hir_prototype.parameters =
        MINI_ARRAY_INIT(ctx->allocator, hir::stmt::Function::Parameter);

    for (usize n = 0; n < prototype->parameters.count(); ++n) {
        const ast::Function_Parameter &parameter = prototype->parameters[n];

        hir::stmt::Function::Parameter hir_parameter;
        hir_parameter.name = parameter.name.value;
        hir_parameter.type_id =
            (sema::Type_Id)*ctx->types().get_id(parameter.name.locus);
        mini_array_append(hir_prototype.parameters, hir_parameter);
    }

    if (prototype->return_type)
        hir_prototype.return_type =
            (sema::Type_Id)*ctx->types().get_id(prototype->return_type->locus);
    else
        hir_prototype.return_type = sema::type_id::Void;

    return hir_prototype;
}

hir::Statement *hir::convert_function_stmt(hir::Context *ctx,
                                           const ast::stmt::Variable *var)
{
    const auto initializer = var->initializer;
    const auto *function   = (const ast::expr::Function *)initializer;

    hir::stmt::Function hir_function{};
    hir_function.prototype =
        convert_function_prototype(ctx, &function->prototype);
    hir_function.name            = var->name.value;
    hir_function.body_is_defined = function->body_is_defined;

    if (function->body_is_defined) {
        hir_function.body = convert_statement(ctx, function->body);
    }

    return ctx->new_stmt(hir::Statement::Kind::Function, hir_function);
}
