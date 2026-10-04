#include "semantics/checks/check_expr.h"
#include "ast/expr.h"
#include "ast/expressions.h"
#include "ast/misc.h"
#include "mini.c/string_view.h"
#include "semantics/checks/check_function_call.h"
#include "semantics/impl.h"
#include "semantics/type/ids.h"
#include "semantics/type/resolver.h"
#include "semantics/worker.h"

static WorkerStatus check_identifier(SemanticContext *sema,
                                     ExprIdentifier *ident);
static void check_integer(SemanticContext *sema, ExprInteger *integer)
{
    Locus locus = integer->base.locus;
    sema::link_locus_to_type(sema, locus, sema::type_id::Int);
}

WorkerStatus sema::check_expression(SemanticContext *sema, void *data)
{
    auto *expr = (Expression *)data;
    switch (expr->kind) {
    case EXPR_Integer: {
        check_integer(sema, (ExprInteger *)expr);
        return WorkerStatus::Done;
    };
    case EXPR_Identifier:
        return check_identifier(sema, (ExprIdentifier *)expr);
    case EXPR_FunctionCall:
        return check_function_call(sema, (ExprFunctionCall *)expr);
    case EXPR_CString: {
        Locus locus  = expr->locus;
        auto type_id = sema::register_or_get_type(
            sema,
            sema::Type::Pointer(Mutability::MUT_Constant, sema::type_id::Char));
        sema::link_locus_to_type(sema, locus, type_id);
        return WorkerStatus::Done;
    }
    default:
        MINI_UNREACHABLE("TODO: %d\n", expr->kind);
    }
}

static WorkerStatus check_identifier(SemanticContext *sema,
                                     ExprIdentifier *ident)
{
    Locus locus           = ident->base.locus;
    mini::StringView name = ident->value;

    sema::ScopeId current_scope = sema->current_scope;
    sema::link_locus_to_scope(sema, locus, current_scope);

    if (auto symbol = sema::eagerly_find_symbol_in(sema, current_scope, name);
        symbol != nullptr) {
        /* todo: switch instead? */
        if (symbol->resolve_state == sema::SymbolState::Unresolved) {
            WorkerStatus status = WorkerStatus::Pending;
            status.wait_for(
                *sema::eagerly_get_id_of_symbol(sema, current_scope, name));
            return status;
        } else if (symbol->resolve_state == sema::SymbolState::Resolving) {
            auto diag = diag_create(Severity::Error, locus,
                                    "cyclic dependency detected", "here");
            sema::report(sema, diag);
            return WorkerStatus::Failed;
        }

        sema::link_locus_to_type(sema, locus, *symbol->as.variable.type_id);
        sema::link_locus_to_symbol(sema, locus, *sema::eagerly_get_id_of_symbol(sema, current_scope, name));
    } else {
        /* symbol not found */
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        sema::report(sema, diag_create(Severity::Error, locus,
                                       "use of undeclared identifier",
                                       "not found in this scope"));
        return WorkerStatus::Failed;
    }

    return WorkerStatus::Done;
}
