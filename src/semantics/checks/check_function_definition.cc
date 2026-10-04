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

static void generate_function_call_schema(SemanticContext *sema,
                                          AstFunctionPrototype *proto,
                                          sema::SymbolId symbol_id)
{
    sema::FunctionCallSchema schema{sema->allocator};

    schema.arity.min = 0;
    schema.arity.max = proto->parameters.count();
    schema.symbol_id = symbol_id;
    schema.is_variadic = proto->is_variadic;

    bool seen_default_param = false;

    for (usize n = 0; n < proto->parameters.count(); ++n) {
        const auto &parameter = proto->parameters[n];

        sema::ParameterSpec param_spec{};
        param_spec.locus   = locus_merge(parameter.name.locus, parameter.typehint->locus);
        param_spec.type_id = sema::get_type_at_locus(sema, parameter.name.locus);
        param_spec.index   = n;
        param_spec.default_expression = parameter.default_expression;

        schema.parameter_names[n] = parameter.name.value;

        if (param_spec.default_expression != nullptr) {
            seen_default_param = true;
        } else {
            if (seen_default_param) {
                Diagnostic diag = diag_create(
                    Severity::Error,
                    parameter.name.locus,
                    "default argument missing",
                    "parameters without default expressions cannot follow parameters that have one"
                );
                sema::report(sema, diag);
            }
            schema.arity.min += 1;
        }

        if (schema.parameters.contains(parameter.name.value)) {
            Diagnostic diag = diag_create(
                Severity::Error,
                parameter.name.locus,
                mini_string_build(sema->allocator, "redefinition of parameter `%s`", parameter.name.value.data),
                "parameter with this name already declared"
            );
            sema::report(sema, diag);
        } else {
            schema.parameters[parameter.name.value] = param_spec;
        }
    }

    sema->set_call_schema(symbol_id, schema);
}

WorkerStatus check_function_prototype(SemanticContext *sema,
                                      AstFunctionPrototype *proto,
                                      sema::Symbol *symbol,
                                      sema::SymbolId symbol_id)
{
    MINI_ASSERT(proto != nullptr, );

    /* ensure all the parameters are resolvable */
    for (usize n = 0; n < proto->parameters.count(); ++n) {
        AstFunctionParameter &parameter = proto->parameters[n];
        if (auto s = sema::resolve_typehint(sema, parameter.typehint,
                                            parameter.name.locus);
            s != WorkerStatus::Done) {
            return s;
        }
    }

    if (proto->return_type) {
        WorkerStatus status = sema::resolve_typehint(sema, proto->return_type,
                                                     proto->return_type->locus);
        if (status != WorkerStatus::Done)
            return status;
    }

    auto current_scope = sema->current_scope;

    MINI_ARRAY(sema::TypeId)
    parameter_type_ids = MINI_ARRAY_INIT(sema->allocator, sema::TypeId);

    for (usize n = 0; n < proto->parameters.count(); ++n) {
        AstFunctionParameter &parameter = proto->parameters[n];
        sema::TypeId id = sema::get_type_at_locus(sema, parameter.name.locus);
        mini_array_append(parameter_type_ids, id);

        sema::Symbol param_symbol{};
        param_symbol.name                = parameter.name.value;
        param_symbol.kind                = sema::SymbolKind::Variable;
        param_symbol.locus               = parameter.name.locus;
        param_symbol.scope_id            = current_scope;
        param_symbol.resolve_state       = sema::SymbolState::Resolved;
        param_symbol.as.variable.type_id = id;
        sema::register_symbol_in(sema, current_scope, parameter.name.value,
                                 parameter.name.locus, param_symbol);
    }

    sema::TypeId return_type = sema::type_id::Void;
    if (proto->return_type)
        return_type = sema::get_type_at_locus(sema, proto->return_type->locus);

    sema::TypeId function_type = sema::register_or_get_type(
        sema, sema::Type::Function(parameter_type_ids, return_type));

    symbol->as.variable.type_id = function_type;
    sema::link_locus_to_type(sema, symbol->locus, function_type);
    generate_function_call_schema(sema, proto, symbol_id);
    return WorkerStatus::Done;

}

WorkerStatus sema::check_function_definition(SemanticContext *sema,
                                             StmtVariable *variable,
                                             bool is_resumption)
{
    MINI_ASSERT(variable != nullptr, );
    MINI_ASSERT(variable->initializer != nullptr, );
    MINI_ASSERT(variable->initializer->kind == EXPR_Function, );

    auto *symbol =
        sema::find_symbol_in(sema, sema->current_scope, variable->name.value);

    symbol->resolve_state = sema::SymbolState::Resolving;

    ExprFunction *function = (ExprFunction *)variable->initializer;
    sema::SymbolId id = *sema::eagerly_get_id_of_symbol(
                                                        sema, sema->current_scope, symbol->name);

    sema::enter_scope(sema, sema::ScopeKind::Block);
    auto scope_guard = mini::ScopeGuard([&]() { sema::leave_scope(sema); });
    if (WorkerStatus check_proto =
        check_function_prototype(sema, &function->prototype, symbol, id);
        check_proto != WorkerStatus::Done) {
        return check_proto;
    }

    symbol->resolve_state = sema::SymbolState::Resolved;

    sema->wake_up_workers(id);

    if (function->body_is_defined) {
        auto s = check_statement(sema, function->body, is_resumption);

        if (s == WorkerStatus::Failed)
            symbol->resolve_state = sema::SymbolState::Failed;
        else if (s == WorkerStatus::Pending) {
            // symbol->resolve_state = sema::SymbolState::Unresolved;
            for (const auto symbold_id : mini::iterate(s.waiting_on)) {
                sema->register_worker(symbold_id,
                                      Worker{function->body, check_statement});
            }
            return WorkerStatus::Pending;
        }
        return s;
    }

    return WorkerStatus::Done;
}
