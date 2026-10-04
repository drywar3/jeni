#include "ast/expr.h"
#include "hir/convert.h"
#include "hir/types/expr.h"
#include "mini.c/mini_def.h"
#include "mini.cc/array.h"
#include "semantics/entities/type.h"

hir::Expression *hir::convert_function_call(hir::Context *ctx,
                                            const ExprFunctionCall *call)
{
    hir::expr::FunctionCall function_call;

    const ::Expression *callee = call->callee;
    const auto &arguments      = call->arguments;

    sema::TypeId type_id = sema::TypeId(*ctx->types().get_id(call->base.locus));

    function_call.callee = hir::convert_expression(ctx, callee);
    function_call.arguments = mini::Array<hir::Expression*>(ctx->allocator);

    for (const auto &argument : arguments.iter()) {
        MINI_ASSERT(argument.is_positional, "TODO: handle non-positional arguments");
        function_call.arguments.append(hir::convert_expression(ctx, argument.argument));
    }

    return ctx->new_expr(hir::Expression::Kind::FunctionCall, type_id, function_call);
}
