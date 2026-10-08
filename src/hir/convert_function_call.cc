#include "ast/expr.h"
#include "hir/convert.h"
#include "hir/types/expr.h"
#include "mini.c/mini_def.h"
#include "mini.cc/array.h"
#include "semantics/entities/type.h"

hir::Expression *
hir::convert_function_call(hir::Context *ctx,
                           const ast::expr::Function_Call *call)
{
    hir::expr::Function_Call function_call;

    const ::Expression *callee = call->callee;
    const auto &arguments      = call->arguments;

    sema::Type_Id type_id = sema::Type_Id(*ctx->types().get_id(call->locus));

    function_call.callee    = hir::convert_expression(ctx, callee);
    function_call.arguments = mini::Array<hir::Expression *>(ctx->allocator);

    for (const auto &argument : arguments.iter()) {
        MINI_ASSERT(argument.is_positional,
                    "TODO: handle non-positional arguments");
        function_call.arguments.append(
            hir::convert_expression(ctx, argument.argument));
    }

    return ctx->new_expr(hir::Expression::Kind::Function_Call, type_id,
                         function_call);
}
