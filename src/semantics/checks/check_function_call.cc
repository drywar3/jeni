#include "semantics/checks/check_function_call.h"
#include "diagnostic.h"
#include "semantics/checks/check_expr.h"
#include "semantics/entities/type.h"
#include "semantics/impl.h"
#include "semantics/worker.h"
#include "semantics/type/coercer.h"

#include <mini.cc/array.h>

static bool
ensure_argument_count_is_sufficient(SemanticContext *sema,
                                    usize argument_count,
                                    const sema::FunctionCallSchema &schema,
                                    Locus locus)
{
    const auto &arity = schema.arity;

    /* must have at least [arity.min] arguments regardless of variadic status */
    if (argument_count < arity.min) {
        Mini_String message = mini_string_build(
            sema->allocator,
            "expected a minimum of `%zu` argument[s]", arity.min
        );

        Diagnostic diag = diag_create(
            Severity::Error,
            locus,
            mini_string_build(sema->allocator, "not expecting `%zu` argument[s]", argument_count),
            message
        );

        const auto &symbol = sema->symbols().at_index(usize(schema.symbol_id));
        diag = diag_add_label(diag, Label{"defined here", symbol.locus});
        sema::report(sema, diag);
        return false;
    }

    /* if variadic then any argument count >= arity.min is valid */
    if (schema.is_variadic) {
        return true;
    }

    /* for non-variadic, check upper bound */
    if (argument_count > arity.max) {
        Mini_String message;
        if (arity.max == 0) {
            message = (Mini_String)"expected zero arguments";
        } else {
            message = mini_string_build(
                sema->allocator,
                "expected a maximum of `%zu` argument[s]", arity.max
            );
        }

        Diagnostic diag = diag_create(
            Severity::Error,
            locus,
            mini_string_build(sema->allocator, "not expecting `%zu` argument[s]", argument_count),
            message
        );

        const auto &symbol = sema->symbols().at_index(usize(schema.symbol_id));
        diag = diag_add_label(diag, Label{"defined here", symbol.locus});
        sema::report(sema, diag);
        return false;
    }

    return true;
}

/* todo: guard against reentrance */
WorkerStatus sema::check_function_call(SemanticContext *sema,
                                       ExprFunctionCall *call)
{
    Locus locus = call->base.locus;
    sema::link_locus_to_scope(sema, locus, sema->current_scope);

    Expression *callee = call->callee;
    auto &arguments    = call->arguments;


    WorkerStatus status = WorkerStatus::Done;
    if (auto s = sema::check_expression(sema, callee);
        s == WorkerStatus::Pending)
        return s;
    else
        status = s;

    sema::Type &callee_type = *sema->types().find(callee->locus);
    if (callee_type.kind != sema::TypeKind::Function && status != WorkerStatus::Failed)  {
        sema::report(
            sema,
            diag_create(Severity::Error, callee->locus, "invalid function call",
                        mini_string_build(sema->allocator,
                                          "type `%s` is not callable",
                                          callee_type.display(sema->allocator,
                                                              sema->types()))));
        status = WorkerStatus::Failed;
    }

    sema::link_locus_to_type(sema, call->base.locus, callee_type.function.return_type);
    sema::SymbolId callee_id = sema::SymbolId(*sema->symbols().get_id(callee->locus));
    sema::FunctionCallSchema schema = *sema->get_call_schema(callee_id);

    usize  argument_count = arguments.count();

    if (!ensure_argument_count_is_sufficient(sema, argument_count, schema, call->base.locus))
        status = WorkerStatus::Failed;

    for (usize index = 0; index < arguments.count(); ++index) {
        const auto &arg = arguments[index];
        Expression *value = arg.argument;

        if (auto s = sema::check_expression(sema, value);
            s != WorkerStatus::Done) {
            if (s == WorkerStatus::Pending)
                return s;
            status = s;
        }

        if (index <= schema.arity.max && !status.is_failed()) {
            sema::TypeId expected_type =
                callee_type.function.parameters[index];
            sema::TypeId recieved_type =
                sema::get_type_at_locus(sema, value->locus);
            if (!sema::coerce_type_into(sema, expected_type, recieved_type,
                                        schema.get_parameter_at_index(index).locus,
                                        value->locus,
                                        true))
                status = WorkerStatus::Failed;
        }
    }

    return status;
}
