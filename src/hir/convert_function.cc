#include "hir/types/stmt.h"
#include "ast/expressions.h"
#include "semantics//type/ids.h"
#include "hir/convert_function.h"

static hir::stmt::Function::Prototype convert_function_prototype(hir::Context *ctx, const AstFunctionPrototype *prototype)
{
    hir::stmt::Function::Prototype hir_prototype;
    hir_prototype.parameters = MINI_ARRAY_INIT(ctx->allocator, hir::stmt::Function::Parameter);

    for (usize n = 0; n < mini_array_count(prototype->parameters); ++n) {
        const AstFunctionParameter &parameter = prototype->parameters[n];

        hir::stmt::Function::Parameter hir_parameter;
        hir_parameter.name    = parameter.name.value;
        hir_parameter.type_id = (sema::TypeId)*ctx->types().get_id(parameter.name.locus);
        mini_array_append(hir_prototype.parameters, hir_parameter);
    }

    if (prototype->return_type)
        hir_prototype.return_type = (sema::TypeId)*ctx->types().get_id(prototype->return_type->locus);
    else
        hir_prototype.return_type = sema::type_id::Void;

    return hir_prototype;
}

hir::Statement *hir::convert_function_stmt(hir::Context *ctx, const StmtVariable *var)
{
    const auto initializer        = var->initializer;
    const ExprFunction *function  = (const ExprFunction *)initializer;

    hir::stmt::Function hir_function;
    hir_function.prototype = convert_function_prototype(ctx, &function->prototype);
    hir_function.name      = var->name.value;
    hir_function.body      = convert_statement(ctx, function->body);

    return ctx->new_stmt(hir::Statement::Kind::Function, hir_function);
}
