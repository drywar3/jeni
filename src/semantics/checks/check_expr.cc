#include "ast/expr.h"
#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/checks/check_expr.h"
#include "semantics/type/ids.h"

void check_integer(SemanticContext *sema, ExprInteger *integer)
{
    Locus locus = integer->base.locus;
    sema::link_locus_to_type(sema, locus, sema::type_id::Int);
}

WorkerStatus sema::check_expression(SemanticContext *sema, void *data)
{
    auto *expr = (ExpressionPointer)data;
    switch (expr->kind) {
    case EXPR_Integer: {
        check_integer(sema, (ExprInteger *)expr);
        return WorkerStatus::Done;
    };
    default:
        MINI_UNREACHABLE("TODO");
    }
}
