#include "ast/expr.h"
#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/checks/check_expr.h"
#include "semantics/type/ids.h"

static WorkerStatus check_identifier(SemanticContext *sema,
                                     ExprIdentifier *ident);
static void check_integer(SemanticContext *sema, ExprInteger *integer)
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
    case EXPR_Identifier:
        return check_identifier(sema, (ExprIdentifier *)expr);
    default:
        MINI_UNREACHABLE("TODO");
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
            status.wait_for(*sema::eagerly_get_id_of_symbol(sema, current_scope, name));
            return status;
        } else if (symbol->resolve_state == sema::SymbolState::Resolving) {
            auto diag = diag_create(DIAG_Error, locus,
                                    "cyclic dependency detected",
                                    "here");
            sema::report(sema, diag);
            return WorkerStatus::Failed;
        }
        sema::link_locus_to_type(sema, locus, *symbol->as.variable.type_id);
    } else {
        /* symbol not found */
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        sema::report(sema, diag_create(DIAG_Error, locus,
                                       "use of undeclared identifier",
                                       "not found in this scope"));
        return WorkerStatus::Failed;
    }

    return WorkerStatus::Done;
}
