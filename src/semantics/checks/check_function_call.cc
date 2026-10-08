#include "semantics/checks/check_function_call.h"
#include "diagnostic.h"
#include "semantics/checks/check_expr.h"
#include "semantics/entities/type.h"
#include "semantics/impl.h"
#include "semantics/type/coercer.h"
#include "semantics/type/ids.h"
#include "semantics/worker.h"

#include <mini.cc/array.h>

static bool ensure_argument_count_is_sufficient(
    Semantic_Context *sema, usize argument_count,
    const sema::Function_Call_Schema &schema, Locus locus)
{
    const auto &arity = schema.arity;

    /* must have at least [arity.min] arguments regardless of variadic status */
    if (argument_count < arity.min) {
        Mini_String message = mini_string_build(
            sema->allocator, "expected at least, %zu argument[s]", arity.min);

        Diagnostic diag =
            diag_create(Severity::Error, locus,
                        mini_string_build(sema->allocator,
                                          argument_count == 0
                                              ? "not expecting zero arguments"
                                              : "not expecting %zu argument[s]",
                                          argument_count),
                        message);

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
            message = (Mini_String) "expected zero arguments";
        } else {
            message = mini_string_build(sema->allocator,
                                        "expected at most, %zu argument[s]",
                                        arity.max);
        }

        Diagnostic diag = diag_create(
            Severity::Error, locus,
            mini_string_build(sema->allocator, "not expecting %zu argument[s]",
                              argument_count),
            message);

        const auto &symbol = sema->symbols().at_index(usize(schema.symbol_id));
        diag = diag_add_label(diag, Label{"defined here", symbol.locus});
        sema::report(sema, diag);
        return false;
    }

    return true;
}

/* todo: guard against reentrance */
Worker_Status sema::check_function_call(Semantic_Context *sema,
                                        ast::expr::Function_Call *call)
{
    Locus locus = call->locus;
    sema::link_locus_to_scope(sema, locus, sema->current_scope);

    Expression *callee = call->callee;
    auto &arguments    = call->arguments;

    Worker_Status status = Worker_Status::Done;

    Worker_Status callee_status = sema::check_expression(sema, callee);
    if (callee_status == Worker_Status::Pending) {
        return callee_status;
    } else if (callee_status == Worker_Status::Failed) {
        sema::link_locus_to_type(sema, call->locus, sema::type_id::Error);
        return callee_status;
    }

    sema::Type &callee_type = *sema->types().find(callee->locus);
    if (callee_type.kind != sema::Type_Kind::Function) {
        sema::report(
            sema,
            diag_create(Severity::Error, callee->locus, "invalid function call",
                        mini_string_build(sema->allocator,
                                          "type `%s` is not callable",
                                          callee_type.display(sema->allocator,
                                                              sema->types()))));
        sema::link_locus_to_type(sema, call->locus, sema::type_id::Error);
        return Worker_Status::Failed;
    }

    sema::link_locus_to_type(sema, call->locus,
                             callee_type.function.return_type);

    auto callee_sym_id_opt = sema->symbols().get_id(callee->locus);
    if (!callee_sym_id_opt.has_value()) {
        return Worker_Status::Failed;
    }

    sema::Symbol_Id callee_id         = sema::Symbol_Id(*callee_sym_id_opt);
    sema::Function_Call_Schema schema = *sema->get_call_schema(callee_id);

    usize argument_count = arguments.count();
    if (!ensure_argument_count_is_sufficient(sema, argument_count, schema,
                                             call->locus)) {
        status = Worker_Status::Failed;
    }

    for (usize index = 0; index < arguments.count(); ++index) {
        const auto &arg   = arguments[index];
        Expression *value = arg.argument;

        Worker_Status arg_status = sema::check_expression(sema, value);
        if (arg_status == Worker_Status::Pending) {
            return arg_status; // YIELD IMMEDIATELY without polluting status
                               // flags
        }
        if (arg_status == Worker_Status::Failed) {
            status = Worker_Status::Failed;
            continue;
        }

        if (index <= schema.arity.max && !schema.is_variadic &&
            !status.is_failed()) {
            sema::Type_Id expected_type =
                schema.get_parameter_at_index(index).type_id;
            sema::Type_Id recieved_type =
                sema::get_type_at_locus(sema, value->locus);
            if (!sema::coerce_type_into(
                    sema, expected_type, recieved_type,
                    schema.get_parameter_at_index(index).locus, value->locus,
                    true)) {
                status = Worker_Status::Failed;
            }
        }
    }

    return status;
}
