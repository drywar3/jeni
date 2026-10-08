#include "semantics/checks/check_expr.h"
#include "ast/expr.h"
#include "ast/expressions.h"
#include "ast/misc.h"
#include "mini.c/string_view.h"
#include "semantics/checks/check_binary_op.h"
#include "semantics/checks/check_function_call.h"
#include "semantics/impl.h"
#include "semantics/type/ids.h"
#include "semantics/type/resolver.h"
#include "semantics/worker.h"

static Worker_Status check_identifier(Semantic_Context *sema,
                                      ast::expr::Identifier *ident);

static void check_integer(Semantic_Context *sema, ast::expr::Integer *integer)
{
    Locus locus = integer->locus;
    sema::link_locus_to_type(sema, locus, sema::type_id::Int);
}

Worker_Status sema::check_expression(Semantic_Context *sema, void *data)
{
    auto *expr = (Expression *)data;
    switch (expr->kind) {
    case EXPR_Integer: {
        check_integer(sema, (ast::expr::Integer *)expr);
        return Worker_Status::Done;
    };
    case EXPR_Identifier:
        return check_identifier(sema, (ast::expr::Identifier *)expr);
    case EXPR_Function_Call:
        return check_function_call(sema, (ast::expr::Function_Call *)expr);
    case EXPR_CString: {
        Locus locus  = expr->locus;
        auto type_id = sema::register_or_get_type(
            sema,
            sema::Type::Pointer(Mutability::Constant, sema::type_id::Char));
        sema::link_locus_to_type(sema, locus, type_id);
        return Worker_Status::Done;
    }
    case EXPR_Binop:
        return check_binary_op(sema, (ast::expr::Binary_Operation *)expr);
    default:
        MINI_UNREACHABLE("TODO: %d\n", expr->kind);
    }
}

static Worker_Status check_identifier(Semantic_Context *sema,
                                      ast::expr::Identifier *ident)
{
    Locus locus           = ident->locus;
    mini::StringView name = ident->value;

    sema::Scope_Id current_scope = sema->current_scope;
    sema::link_locus_to_scope(sema, locus, current_scope);

    // Get the symbol ID directly alongside the symbol pointer
    auto sym_opt = sema::eagerly_lookup_symbol(sema, current_scope, name);
    if (!sym_opt.has_value()) {
        /* symbol not found */
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        sema::report(sema, diag_create(Severity::Error, locus,
                                       "use of undeclared identifier",
                                       "not found in this scope"));
        return Worker_Status::Failed;
    }

    sema::Symbol_Id sym_id          = sym_opt->id;
    const sema::Symbol_Proxy symbol = *sym_opt;

    if (symbol->is_state(sema::Symbol_State::Unresolved)) {
        Worker_Status status = Worker_Status::Pending;
        status.wait_for(sym_id); // wait specifically on THIS symbol's ID
        return status;
    }

    if (symbol->is_state(sema::Symbol_State::Resolving)) {
        /* a symbol can only cycle on ITSELF, not if a parent function scope is
         * Resolving! Make sure symbol->id == sym_id and we are actually in a
         * dependency cycle on this specific symbol. */
        auto diag = diag_create(Severity::Error, locus,
                                "cyclic dependency detected", "here");
        diag      = diag_add_label(diag, Label{"check_here", symbol->locus});
        sema::report(sema, diag);
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        return Worker_Status::Failed;
    }

    sema::link_locus_to_type(sema, locus, *symbol->variable.type_id);
    sema::link_locus_to_symbol(sema, locus, sym_id);

    return Worker_Status::Done;
}
