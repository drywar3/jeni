#include "semantics/impl.h"
#include "semantics/type/ids.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_binary_op.h"

using CheckOpCompatibilityFn = bool(*)(SemanticContext *sema, Locus locus, const Expression *left, const Expression *right);

struct CheckOpCompatibility {
    AstOperator op;
    CheckOpCompatibilityFn func;
};

static bool check_basic_compatibility(SemanticContext *sema, Locus locus, const Expression *left, const Expression *right);
static bool check_equality_compatibility(SemanticContext *sema, Locus locus, const Expression *left, const Expression *right);
static bool check_assign_compatibility(SemanticContext *sema, Locus locus, const Expression *left, const Expression *right);

std::initializer_list<CheckOpCompatibility> op_compatible = {
    {AstOperator::Add,check_basic_compatibility},
    {AstOperator::Sub,check_basic_compatibility},
    {AstOperator::Mul,check_basic_compatibility},
    {AstOperator::Div,check_basic_compatibility},
    {AstOperator::Assign,check_assign_compatibility},
    {AstOperator::Equals,check_equality_compatibility},
    {AstOperator::NotEquals,check_equality_compatibility},
    {AstOperator::LessThan,check_basic_compatibility},
    {AstOperator::GreaterThan,check_basic_compatibility},
    {AstOperator::LessThanEquals,check_basic_compatibility},
    {AstOperator::GreaterThanEquals,check_basic_compatibility},
};

WorkerStatus sema::check_binary_op(SemanticContext *sema, ExprBinaryOperation *binop)
{
    Expression *left  = binop->left;
    Expression *right = binop->right;
    AstOperator op    = binop->op;

    WorkerStatus status = sema::check_expression(sema, left);
    if (!status.is_done()) return status;

    status = sema::check_expression(sema, right);
    if (!status.is_done()) return status;

    for (const auto [op_, fn] : op_compatible) {
        if (op == op_) {
            if (!fn(sema, binop->base.locus, left, right))
                return WorkerStatus::Failed;

            return WorkerStatus::Done;
        }
    }

    MINI_UNREACHABLE();
}

bool check_basic_compatibility(SemanticContext *sema, Locus locus, const Expression *left, const Expression *right)
{
    sema::TypeId left_type_id  = sema::get_type_at_locus(sema, left->locus);
    sema::TypeId right_type_id = sema::get_type_at_locus(sema, left->locus);

    /* for now only int is ok */
    if (left_type_id != sema::type_id::Int ||
        right_type_id != sema::type_id::Int) {
        sema::report(sema, diag_create(Severity::Error, locus,
                                       "invalid operands to binary operator",
                                       mini_string_build(sema->allocator,
                                                         "cannot operator on `%s` and `%s`",
                                                         sema::display_type(sema, left_type_id),
                                                         sema::display_type(sema, right_type_id))));
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        return false;
    }

    sema::link_locus_to_type(sema, locus, left_type_id);
    return true;
}


static bool expression_is_lvalue(const Expression *expr)
{
    switch (expr->kind) {
    case EXPR_Integer:
    case EXPR_String:
    case EXPR_Binop:
    case EXPR_FunctionCall:
        return false;
    case EXPR_Identifier:
        return true;
    default: MINI_UNREACHABLE();
    }
}

bool check_assign_compatibility(SemanticContext *sema, Locus locus, const Expression *left, const Expression *right)
{
    sema::TypeId left_type_id  = sema::get_type_at_locus(sema, left->locus);
    sema::TypeId right_type_id = sema::get_type_at_locus(sema, left->locus);

    if (left_type_id != right_type_id) {
        sema::report(sema, diag_create(Severity::Error, right->locus,
                                       "invalid operands to binary operator",
                                       mini_string_build(sema->allocator,
                                                         "expected `%s`", sema::display_type(sema, left_type_id))));
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        return false;
    }

    if (!expression_is_lvalue(left)) {
        sema::report(sema, diag_create(Severity::Error, left->locus,
                                       "cannot assign to expression",
                                       "expected an lvalue (expression with a stable address)"));
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        return false;
    }


    if (auto symbol_opt = sema::get_symbol_at_locus(sema, left->locus);
        symbol_opt.has_value()) {
        sema::SymbolProxy symbol = *symbol_opt;
        if (symbol->kind == sema::SymbolKind::Variable &&
            symbol->variable.mutability) {
            auto diag = diag_create(Severity::Error, left->locus,
                                    "cannot assign to expression",
                                    "expression is constant");
            sema::report(sema, diag_add_label(diag, Label{"defined here", symbol->locus}));
            sema::link_locus_to_type(sema, locus, sema::type_id::Error);
            return false;
        }
    }

    sema::link_locus_to_type(sema, locus, left_type_id);
    return true;
}

bool check_equality_compatibility(SemanticContext *sema, Locus locus,
                                  const Expression *left,
                                  const Expression *right)
{
    sema::TypeId left_type_id  = sema::get_type_at_locus(sema, left->locus);
    sema::TypeId right_type_id = sema::get_type_at_locus(sema, right->locus);

    if (left_type_id != right_type_id) {
        sema::report(sema, diag_create(Severity::Error, right->locus,
                                       "invalid operands to binary operator",
                                       mini_string_build(sema->allocator,
                                                         "expected `%s`", sema::display_type(sema, left_type_id))));
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        return false;
    }

    bool is_ok_to_compare;

    switch (left_type_id) {
    case sema::type_id::Int:
        is_ok_to_compare = true;
        break;
    default:
        is_ok_to_compare = false;
        break;
    }

    if (!is_ok_to_compare) {
        sema::report(sema, diag_create(Severity::Error, right->locus,
                                       "invalid operands to binary operator",
                                       mini_string_build(sema->allocator,
                                                         "cannot operate on `%s` and `%s`",
                                                         sema::display_type(sema, left_type_id),
                                                         sema::display_type(sema, right_type_id))));
        sema::link_locus_to_type(sema, locus, sema::type_id::Error);
        return false;
    }

    sema::link_locus_to_type(sema, locus, sema::type_id::Bool);
    return true;
}
