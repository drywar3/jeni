#include "semantics/checks/check_function_definition.h"
#include "ast/expressions.h"
#include "ast/statements.h"
#include "mini.cc/scope_guard.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/entities/scope.h"
#include "semantics/entities/symbol.h"
#include "semantics/impl.h"
#include "semantics/type/ids.h"
#include "semantics/type/resolver.h"
#include "semantics/worker.h"

#include <mini.cc/dtor.h>

static void generate_function_call_schema(Semantic_Context *sema,
                                          ast::Function_Prototype *proto,
                                          sema::Symbol_Id symbol_id)
{
    sema::Function_Call_Schema schema{sema->allocator};

    schema.arity.min   = 0;
    schema.arity.max   = proto->parameters.count();
    schema.symbol_id   = symbol_id;
    schema.is_variadic = proto->is_variadic;

    bool seen_default_param = false;

    for (usize n = 0; n < proto->parameters.count(); ++n) {
        const auto &parameter = proto->parameters[n];

        sema::Parameter_Spec param_spec{};
        param_spec.locus =
            locus_merge(parameter.name.locus, parameter.typehint->locus);
        param_spec.type_id =
            sema::get_type_at_locus(sema, parameter.name.locus);
        param_spec.index              = n;
        param_spec.default_expression = parameter.default_expression;

        schema.parameter_names[n] = parameter.name.value;

        if (param_spec.default_expression != nullptr) {
            seen_default_param = true;
        } else {
            if (seen_default_param) {
                Diagnostic diag =
                    diag_create(Severity::Error, parameter.name.locus,
                                "default argument missing",
                                "parameters without default expressions cannot "
                                "follow parameters that have one");
                sema::report(sema, diag);
            }
            schema.arity.min += 1;
        }

        if (schema.parameters.contains(parameter.name.value)) {
            Diagnostic diag = diag_create(
                Severity::Error, parameter.name.locus,
                mini_string_build(sema->allocator,
                                  "redefinition of parameter `%.*s`",
                                  SVARG(parameter.name.value)),
                "parameter with this name already declared");
            sema::report(sema, diag);
        } else {
            schema.parameters[parameter.name.value] = param_spec;
        }
    }

    sema->set_call_schema(symbol_id, schema);
}

Worker_Status check_function_prototype(Semantic_Context *sema,
                                       ast::Function_Prototype *proto,
                                       sema::Symbol_Id symbol_id)
{
    MINI_ASSERT(proto != nullptr, );

    /* ensure all the parameters are resolvable */
    for (usize n = 0; n < proto->parameters.count(); ++n) {
        ast::Function_Parameter &parameter = proto->parameters[n];
        if (auto s = sema::resolve_typehint(sema, parameter.typehint,
                                            parameter.name.locus);
            s != Worker_Status::Done) {
            return s;
        }
    }

    if (proto->return_type) {
        Worker_Status status = sema::resolve_typehint(
            sema, proto->return_type, proto->return_type->locus);
        if (status != Worker_Status::Done)
            return status;
    }

    auto current_scope = sema->current_scope;

    auto parameter_type_ids = sema::Function_Type::Parameters(sema->allocator);

    for (usize n = 0; n < proto->parameters.count(); ++n) {
        ast::Function_Parameter &parameter = proto->parameters[n];
        sema::Type_Id id = sema::get_type_at_locus(sema, parameter.name.locus);
        parameter_type_ids.append(id);
        sema::Symbol param_symbol{};
        param_symbol.name     = parameter.name.value;
        param_symbol.kind     = sema::Symbol_Kind::Variable;
        param_symbol.locus    = parameter.name.locus;
        param_symbol.scope_Id = current_scope;
        param_symbol.set_state(sema::Symbol_State::Resolved);
        param_symbol.variable.type_id    = id;
        param_symbol.variable.mutability = Mutability::Constant;
        sema::register_symbol_in(sema, current_scope, parameter.name.value,
                                 parameter.name.locus, param_symbol);
    }

    sema::Type_Id return_type = sema::type_id::Void;
    if (proto->return_type)
        return_type = sema::get_type_at_locus(sema, proto->return_type->locus);

    sema::Type_Id function_type = sema::register_or_get_type(
        sema, sema::Type::Function(parameter_type_ids, return_type));

    /* refresh the symbol pointer using symbol_id in case register_symbol_in
     * reallocated storage */
    sema::Symbol *symbol     = sema->symbols().at_index_ptr(usize(symbol_id));
    symbol->variable.type_id = function_type;
    sema::link_locus_to_type(sema, symbol->locus, function_type);
    generate_function_call_schema(sema, proto, symbol_id);
    return Worker_Status::Done;
}

Worker_Status sema::check_function_definition(Semantic_Context *sema,
                                              ast::stmt::Variable *variable,
                                              bool is_resumption)
{
    MINI_ASSERT(variable != nullptr, );
    MINI_ASSERT(variable->initializer != nullptr, );
    MINI_ASSERT(variable->initializer->kind == EXPR_Function, );

    auto symbol_opt = sema::eagerly_find_symbol_in(sema, sema->current_scope,
                                                   variable->name.value);
    MINI_ASSERT(symbol_opt.has_value(), );

    sema::Symbol_Proxy symbol = *symbol_opt;

    symbol->set_state(sema::Symbol_State::Resolving);

    ast::expr::Function *function =
        (ast::expr::Function *)variable->initializer;
    sema::Symbol_Id id = *sema::eagerly_get_id_of_symbol(
        sema, sema->current_scope, symbol->name);

    sema::enter_scope(sema, sema::Scope_Kind::Function);
    auto scope_guard = mini::ScopeGuard([&]() { sema::leave_scope(sema); });

    if (Worker_Status check_proto =
            check_function_prototype(sema, &function->prototype, id);
        check_proto != Worker_Status::Done) {
        symbol->set_state(check_proto == Worker_Status::Failed
                              ? sema::Symbol_State::Failed
                              : sema::Symbol_State::Unresolved);
        return check_proto;
    }

    sema::find_first_scope_of(
        sema, sema::Scope_Kind::Function, [function](auto *sema, auto &scope) {
            scope.function.return_type =
                function->prototype.return_type
                    ? sema::get_type_at_locus(
                          sema, function->prototype.return_type->locus)
                    : sema::type_id::Void;
        });

    /* checking function prototype might have relocated the symbol
     * so i refresh the variable here (just in case) */
    symbol = sema::get_symbol_by_id(sema, id);
    symbol->set_state(sema::Symbol_State::Resolved);

    sema->wake_up_workers(id);

    if (function->body_is_defined) {
        auto s = check_statement(sema, function->body, is_resumption);
        if (s == Worker_Status::Failed)
            symbol->set_state(sema::Symbol_State::Failed);
        else if (s == Worker_Status::Pending) {
            // symbol->resolve_state = sema::Symbol_State::Unresolved;
            for (const auto symbold_id : mini::iterate(s.waiting_on)) {
                sema->register_worker(
                    symbold_id,
                    Worker(function->body, check_statement,
                           s.working_scope.value_or(sema->current_scope)));
            }
            return Worker_Status::Pending;
        }
        return s;
    }

    return Worker_Status::Done;
}
