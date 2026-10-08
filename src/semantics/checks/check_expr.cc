#include "ast/expr.h"
#include "ast/misc.h"
#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/worker.h"
#include "mini.c/string_view.h"
#include "semantics/type/ids.h"
#include "semantics/type/resolver.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_binary_op.h"
#include "semantics/checks/check_function_call.h"

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
            sema::Type::Pointer(Mutability::Constant, sema::type_id::Char));
        sema::link_locus_to_type(sema, locus, type_id);
        return WorkerStatus::Done;
    }
    case EXPR_Binop: return check_binary_op(sema, (ExprBinaryOperation*)expr);
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

    // Get the symbol ID directly alongside the symbol pointer
    auto sym_opt = sema::eagerly_lookup_symbol(sema, current_scope, name);
    if (!sym_opt.has_value()) {
        /* symbol not found */
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        sema::report(sema, diag_create(Severity::Error, locus,
                                       "use of undeclared identifier",
                                       "not found in this scope"));
        return WorkerStatus::Failed;
    }

    sema::SymbolId sym_id = sym_opt->id;
    const sema::SymbolProxy symbol   = *sym_opt;

    if (symbol->is_state(sema::SymbolState::Unresolved)) {
        WorkerStatus status = WorkerStatus::Pending;
        status.wait_for(sym_id); // wait specifically on THIS symbol's ID
        return status;
    }

    if (symbol->is_state(sema::SymbolState::Resolving)) {
        /* a symbol can only cycle on ITSELF, not if a parent function scope is Resolving!
         * Make sure symbol->id == sym_id and we are actually in a dependency cycle on this specific symbol. */
        auto diag = diag_create(Severity::Error, locus,
                                "cyclic dependency detected", "here");
        diag = diag_add_label(diag, Label{"check_here", symbol->locus});
        sema::report(sema, diag);
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        return WorkerStatus::Failed;
    }

    sema::link_locus_to_type(sema, locus, *symbol->variable.type_id);
    sema::link_locus_to_symbol(sema, locus, sym_id);

    return WorkerStatus::Done;
}
