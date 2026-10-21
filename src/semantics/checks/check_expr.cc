#include "ast/misc.h"
#include "ast/expr.h"
#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/worker.h"
#include "mini.c/string_view.h"
#include "semantics/type/ids.h"
#include "semantics/type/coercer.h"
#include "semantics/type/resolver.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_binary_op.h"
#include "semantics/checks/check_struct_decl.h"
#include "semantics/checks/check_member_access.h"
#include "semantics/checks/check_function_call.h"

#include <mini.cc/scope_guard.h>

static Worker_Status check_dereference(Semantic_Context *sema, Expression *expr);
static Worker_Status check_addr_of(Semantic_Context *sema, Expression *expr);
static Worker_Status check_typehint(Semantic_Context *sema, Expression *expr);
static Worker_Status check_subscript(Semantic_Context *sema, Expression *expr);
static Worker_Status check_unary(Semantic_Context *sema, Expression *expr);
static Worker_Status check_eval(Semantic_Context *sema, Expression* &expr);
static Worker_Status check_cast(Semantic_Context *sema, Expression* &expr, bool is_resumption);

bool sema::expression_is_value_compatible(Semantic_Context *sema, Expression *expr)
{
    auto type_id = sema::get_type_at_locus(sema, expr->locus);
    auto *layout = sema::get_type_by_id(sema, type_id);

    switch (layout->kind) {
    case sema::Type_Kind::Namespace:
        return false;
    default: return true;
    }
}

static void check_integer(Semantic_Context *sema, ast::expr::Integer *integer)
{
    Locus locus = integer->locus;
    auto value  = integer->value;

    auto type_context = sema->get_current_type_context();

    if (type_context && sema::try_coerce_type_into(sema, *type_context, sema::type_id::Int, false)) {
        sema::link_locus_to_type(sema, locus, *type_context);
    } else sema::link_locus_to_type(sema, locus, sema::type_id::Int);
}

static void check_float(Semantic_Context *sema, ast::expr::Float *float_)
{
    Locus locus = float_->locus;
    sema::link_locus_to_type(sema, locus, sema::type_id::Float32);
}


Worker_Status sema::check_expression(Semantic_Context *sema, Expression *&expr, bool is_resumption)
{
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
    case EXPR_Dereference:   return check_dereference(sema, expr);
    case EXPR_Address_Of:    return check_addr_of(sema, expr);
    case EXPR_Subscript:     return check_subscript(sema, expr);
    case EXPR_Unary:         return check_unary(sema, expr);
    case EXPR_Eval:          return check_eval(sema, expr);
    case EXPR_Cast:          return check_cast(sema, expr, is_resumption);
    case EXPR_Use:           return Worker_Status::Done;
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
        status.at_scope(sema->current_scope);
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

Worker_Status check_dereference(Semantic_Context *sema,
                                Expression *expr)
{
    auto deref  = (ast::expr::Dereference *)expr;
    auto status = sema::check_expression(sema, deref->value);
    if (!status.is_done()) return status;

    auto type_id = sema::get_type_at_locus(sema, deref->value->locus);
    auto &layout = *sema::get_type_by_id(sema, type_id);

    if (layout.kind != sema::Type_Kind::Pointer) {
        sema::report_error(sema, expr->locus,
                           "can only dereference a pointer",
                           mini_string_build(sema->allocator,
                                             "expected a pointer, got `%s`",
                                             sema::display_type(sema, type_id)));
        return Worker_Status::Failed;
    }

    if (layout.pointer.target_type == sema::type_id::Void) {
        sema::report_error(sema, expr->locus,
                           "cannot dereference a pointer to `void`");
        return Worker_Status::Failed;
    }

    sema::link_locus_to_type(sema, expr->locus, layout.pointer.target_type);
    return Worker_Status::Done;
}

Worker_Status check_addr_of(Semantic_Context *sema,
                            Expression *expr)
{
    auto *addr_of = (ast::expr::AddressOf *)expr;
    auto status   = sema::check_expression(sema, addr_of->value);
    if (!status.is_done()) return status;

    if (!sema::expression_is_lvalue(sema, addr_of->value)) {
        sema::report_error(sema, expr->locus, "expected an lvalue");
        return Worker_Status::Failed;
    }

    auto type_id_of_value = sema::get_type_at_locus(sema, addr_of->value->locus);
    auto type_id_of_expr  = sema::register_or_get_type(sema, sema::Type::Pointer(addr_of->mutability, type_id_of_value));
    sema::link_locus_to_type(sema, expr->locus, type_id_of_expr);
    return Worker_Status::Done;
}

Worker_Status check_subscript(Semantic_Context *sema, Expression *expr)
{
    auto *subscript = (ast::expr::Subscript *)expr;
    auto status     = sema::check_expression(sema, subscript->expr);
    if (!status.is_done()) return status;

    status = sema::check_expression(sema, subscript->index);
    if (!status.is_done()) return status;

    auto type_id = sema::get_type_at_locus(sema, subscript->index->locus);
    if (!sema::try_coerce_type_into(sema, sema::type_id::Int, type_id)) {
        sema::report_error(sema, *expr, "cannot perform subscript", "index is not an integer");
        return Worker_Status::Failed;
    }

    type_id      = sema::get_type_at_locus(sema, subscript->expr->locus);
    auto *layout = sema::get_type_by_id(sema, type_id);

    sema::Type_Id type_id_of_element;

    switch (layout->kind) {
    case sema::Type_Kind::Pointer:
        type_id_of_element = layout->pointer.target_type;
        if (type_id_of_element == sema::type_id::Void) {
            sema::report_error(sema, *expr, "cannot perform subscript", "cannot index `void` pointer");
            return Worker_Status::Failed;
        }
        break;
    case sema::Type_Kind::Array:
        type_id_of_element = layout->array.element_type;
        break;
    default:
        sema::report_error(sema, *subscript->expr, "cannot perform subscript",
                           mini_string_build(sema->allocator,
                                             "type `%s` cannot be indexed", sema::display_type(sema, type_id)));
        return Worker_Status::Failed;
    }
    sema::link_locus_to_type(sema, subscript->locus, type_id_of_element);
    return Worker_Status::Done;
}

Worker_Status check_unary(Semantic_Context *sema, Expression *expr)
{
    auto *unary = (ast::expr::Unary_Operation *)expr;
    if (unary->op != ast::Operator::Not) MINI_UNREACHABLE();

    auto status = sema::check_expression(sema, unary->expression);
    if (!status.is_done()) return status;

    auto type_id = sema::get_type_at_locus(sema, unary->expression->locus);
    if (!sema::try_coerce_type_into(sema, sema::type_id::Bool, type_id)) {
        sema::report_error(sema, *expr, "invalid operand to unary operation", "expected a boolean");
        sema::link_locus_to_type(sema, expr->locus, sema::type_id::Error);
        return Worker_Status::Failed;
    }

    sema::link_locus_to_type(sema, expr->locus, sema::type_id::Bool);
    return Worker_Status::Done;
}

Expression *value_to_ast_expr(Semantic_Context *sema,
                              Expression *old,
                              compeval::Value value)
{
    switch (value.kind) {
    case compeval::Value::Int:
        return ast::expr::alloc_expression(sema->allocator, EXPR_Integer,
                                           old->locus,
                                           ast::expr::Integer{.value=value.integer});
    case compeval::Value::Float:
        return ast::expr::alloc_expression(sema->allocator, EXPR_Float,
                                           old->locus,
                                           ast::expr::Float{.value=value.floating});

    default: MINI_UNREACHABLE();
    }
}

Worker_Status check_eval(Semantic_Context *sema, Expression* &expr)
{
    auto *eval = (ast::expr::Eval *)expr;
    auto status = sema::check_expression(sema, eval->expr);
    if (!status.is_done()) return status;
    auto result = sema::comptime_evaluate(sema, eval->expr);
    if (!result) return Worker_Status::Failed;
    expr = value_to_ast_expr(sema, expr, *result);
    sema::link_locus_to_type_at_locus(sema, eval->expr->locus, expr->locus);
    return status;
}

Worker_Status check_cast(Semantic_Context *sema, Expression* &expr, bool is_resumption)
{
    auto *cast = (ast::expr::Cast *)expr;

    auto type_context = sema->get_current_type_context();

    if (!cast->type_is_defined && !type_context) {
        sema::report_error(sema, *expr, "cannot infer type to cast to");
        return Worker_Status::Failed;
    }

    sema::Type_Id expected_type;
    sema::Type_Id recieved_type;

    if (cast->type_is_defined) {
        auto status = sema::resolve_typehint(sema, cast->typehint, cast->typehint->locus);
        if (!status.is_done()) return status;
        expected_type = sema::get_type_at_locus(sema, cast->typehint->locus);
    } else if (type_context) {
        expected_type = *type_context;
    } else MINI_UNREACHABLE();


    sema->set_current_type_context(None);
    auto scope_guard = mini::ScopeGuard([&]() {sema->set_current_type_context(type_context);});

    auto status = sema::check_expression(sema, cast->expr);
    if (!status.is_done()) return status;

    recieved_type = sema::get_type_at_locus(sema, cast->expr->locus);

    if (expected_type != recieved_type) {
        if (!sema::try_coerce_type_into(sema, expected_type, recieved_type, false)) {
            sema::report_error(sema, *expr,
                               "cast is invalid",
                               mini_string_build(sema->allocator,
                                                 "cannot cast from `%s` to `%s`",
                                                 sema::display_type(sema, recieved_type),
                                                 sema::display_type(sema, expected_type)));
            return Worker_Status::Failed;
        }
    } else {
        sema::report_warning(sema, *expr,
                           "redundant cast");
    }

    sema::link_locus_to_type(sema, expr->locus, expected_type);
    return Worker_Status::Done;
}
