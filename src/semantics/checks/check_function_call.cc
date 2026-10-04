#include "semantics/checks/check_function_call.h"
#include "diagnostic.h"
#include "semantics/checks/check_expr.h"
#include "semantics/entities/type.h"
#include "semantics/impl.h"
#include "semantics/worker.h"

#include <mini.cc/array.h>

WorkerStatus sema::check_function_call(SemanticContext *sema,
                                       ExprFunctionCall *call)
{
    Locus locus = call->base.locus;
    sema::link_locus_to_scope(sema, locus, sema->current_scope);

    Expression *callee = call->callee;
    auto *arguments    = call->arguments;


    WorkerStatus status = WorkerStatus::Done;
    if (auto s = sema::check_expression(sema, callee);
        s == WorkerStatus::Pending)
        return s;
    else
        status = s;

    sema::Type &callee_type = *sema->types().find(callee->locus);
    sema::link_locus_to_type(sema, call->base.locus,
                             callee_type.function.return_type);

    if (callee_type.kind != sema::TypeKind::Function && status != WorkerStatus::Failed)  {
        sema::report(
            sema,
            diag_create(DIAG_Error, callee->locus, "invalid function call",
                        mini_string_build(sema->allocator,
                                          "type `%s` is not callable",
                                          callee_type.display(sema->allocator,
                                                              sema->types()))));
        status = WorkerStatus::Failed;
    }

    for (const auto &arg : mini::iterate(arguments)) {
        Expression *value = arg.argument;
        if (auto s = sema::check_expression(sema, value);
            s != WorkerStatus::Done) {
            if (s == WorkerStatus::Pending)
                return s;
            status = s;
        }
    }

    return status;
}
