#include "ast/statements.h"
#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/type/resolver.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/checks/check_function_definition.h"

WorkerStatus check_function_prototype(SemanticContext *sema,
                                      AstFunctionPrototype *proto)
{
    MINI_ASSERT(proto != nullptr, );
    MINI_ASSERT(proto->parameters != nullptr, );

    /* ensure all the parameters are resolvable */
    for (usize n = 0; n < mini_array_count(proto->parameters); ++n) {
        AstFunctionParameter &parameter = proto->parameters[n];
        if (auto s = sema::resolve_typehint(sema, parameter.typehint,
                                            parameter.name.locus);
            s != WorkerStatus::Done) {
            return s;
        }
    }

    auto current_scope = sema->current_scope;

    for (usize n = 0; n < mini_array_count(proto->parameters); ++n) {
        AstFunctionParameter &parameter = proto->parameters[n];
        sema::TypeId id = sema::get_type_at_locus(sema, parameter.name.locus);

        sema::Symbol param_symbol{};
        param_symbol.name          = parameter.name.value;
        param_symbol.kind          = sema::SymbolKind::Variable;
        param_symbol.locus         = parameter.name.locus;
        param_symbol.scope_id      = current_scope;
        param_symbol.resolve_state = sema::SymbolState::Resolved;
        param_symbol.as.variable.type_id = id;
        sema::register_symbol_in(sema, current_scope, parameter.name.value,
                                 parameter.name.locus, param_symbol);
    }
    if (proto->return_type) {
        sema::resolve_typehint(sema, proto->return_type, std::nullopt);
    }
    return WorkerStatus::Done;
}

WorkerStatus sema::check_function_definition(SemanticContext *sema,
                                             StmtVariable *variable)
{
    MINI_ASSERT(variable != nullptr, );
    MINI_ASSERT(variable->initializer != nullptr, );
    MINI_ASSERT(variable->initializer->kind == EXPR_Function, );
    ExprFunction *function = (ExprFunction *)variable->initializer;
    sema::enter_scope(sema, sema::ScopeKind::Block);
    if (WorkerStatus check_proto =
            check_function_prototype(sema, &function->prototype);
        check_proto != WorkerStatus::Done) {
        return check_proto;
    }
    check_statement(sema, function->body);
    sema::leave_scope(sema);
    return WorkerStatus::Done;
}
