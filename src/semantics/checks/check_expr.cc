#include "ast/misc.h"
#include "ast/expr.h"
#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/worker.h"
#include "mini.c/string_view.h"
#include "semantics/type/ids.h"
#include "semantics/type/resolver.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_binary_op.h"
#include "semantics/checks/check_struct_decl.h"
#include "semantics/checks/check_member_access.h"
#include "semantics/checks/check_function_call.h"

static Worker_Status check_typehint(Semantic_Context *sema,
                                    Expression *ident);

static void check_integer(Semantic_Context *sema, ast::expr::Integer *integer)
{
    Locus locus = integer->locus;
    sema::link_locus_to_type(sema, locus, sema::type_id::Int);
}

static void check_float(Semantic_Context *sema, ast::expr::Float *float_)
{
    Locus locus = float_->locus;
    sema::link_locus_to_type(sema, locus, sema::type_id::Float32);
}


Worker_Status sema::check_expression(Semantic_Context *sema, void *data)
{
    auto *expr = (Expression *)data;
    switch (expr->kind) {
    case EXPR_Integer: {
        check_integer(sema, (ast::expr::Integer *)expr);
        return Worker_Status::Done;
    };
    case EXPR_Float: {
        check_float(sema, (ast::expr::Float *)expr);
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
    case EXPR_Typehint: return check_typehint(sema, expr);
    case EXPR_Member_Access: return check_member_access(sema, expr);
    default:
        MINI_UNREACHABLE("TODO: %d\n", expr->kind);
    }
}

Worker_Status sema::check_identifier(Semantic_Context *sema,
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

    sema::link_locus_to_type_at_locus(sema, symbol->locus, locus);
    sema::link_locus_to_symbol(sema, locus, sym_id);

    return Worker_Status::Done;
}

Worker_Status check_typehint(Semantic_Context *sema,
                             Expression *ty_expr)
{
    auto *typehint = (Typehint *)ty_expr;
    return sema::resolve_typehint(sema, typehint, typehint->locus);
}
