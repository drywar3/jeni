#include "hir/types/expr.h"
#include "lir/lir.h"
#include "lir/types/buildr.h"
#include "lir/types/type.h"
#include "lir/types/value.h"

lir::ValueId lir::lower_function_call(lir::Buildr *b,
                                      const hir::Expression *expression)
{
    const hir::expr::Function_Call &call = expression->as.function_call;
    lir::TypePtr type                    = lower_type(b, expression->type_id);

    ValueId callee = lir::lower_expression(b, call.callee);

    auto arguments = mini::Array<lir::ValueId>(b->allocator());
    for (const auto *argument : call.arguments.iter()) {
        ValueId lowered_argument = lir::lower_expression(b, argument);
        arguments.append(lowered_argument);
    }

    return b->create_call(type, callee, arguments);
}
