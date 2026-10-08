#include "hir/types/expr.h"
#include "lir/lir.h"

static lir::ValueId lower_binary_op(lir::Buildr *b,
                                    const hir::Expression *expression)
{
    lir::ValueId left  = lir::lower_expression(b, expression->as.binop.left);
    lir::ValueId right = lir::lower_expression(b, expression->as.binop.right);
    lir::CmpOp op;
    switch (expression->as.binop.op) {
        /* todo: hack */
    case ast::Operator::Sub:
        op = lir::CmpOp::Sub;
        break;
    case ast::Operator::Mul:
        op = lir::CmpOp::Mul;
        break;
    case ast::Operator::Add:
        op = lir::CmpOp::Add;
        break;
    case ast::Operator::Div:
        op = lir::CmpOp::Div;
        break;

    case ast::Operator::Equals:
        op = lir::CmpOp::Equals;
        break;
    case ast::Operator::NotEquals:
        op = lir::CmpOp::NotEquals;
        break;
    case ast::Operator::LessThan:
        op = lir::CmpOp::LessThan;
        break;
    case ast::Operator::GreaterThan:
        op = lir::CmpOp::GreaterThan;
        break;

    case ast::Operator::GreaterThanEquals:
        op = lir::CmpOp::GreaterThanEquals;
        break;
    case ast::Operator::LessThanEquals:
        op = lir::CmpOp::LessThanEquals;
        break;
    default:
        MINI_UNREACHABLE();
    }
    return b->create_cmp(op, left, right);
}

static lir::ValueId lower_assign(lir::Buildr *b,
                                 const hir::Expression *expression)
{
    lir::ValueId receiver =
        lir::lower_expression(b, expression->as.assign.receiver);
    lir::ValueId value = lir::lower_expression(b, expression->as.assign.value);
    b->create_store(receiver, lir::lower_type(b, expression->type_id), value);
    return value;
}

lir::ValueId lir::lower_expression(Buildr *b, const hir::Expression *expression)
{
    switch (expression->kind) {
    case hir::Expression::Kind::CString:
        return b->create_cstring(expression->as.string.value);
    case hir::Expression::Kind::Integer:
        return b->create_integer(expression->as.integer.value);
    case hir::Expression::Kind::Boolean:
        return b->create_boolean(expression->as.boolean.value);
    case hir::Expression::Kind::Identifier:
        if (auto var = b->find_local(expression->as.identifier.value)) {
            return b->create_deref(b->create_local_ref(*var));
        }
        return b->create_glob_ref(expression->as.identifier.value);
    case hir::Expression::Kind::Function_Call:
        return lower_function_call(b, expression);
    case hir::Expression::Kind::Assign:
        return lower_assign(b, expression);
    case hir::Expression::Kind::Binary_Operation:
        return lower_binary_op(b, expression);
    default:
        MINI_UNREACHABLE("%d", expression->kind);
    }
}
