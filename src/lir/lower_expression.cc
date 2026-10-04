#include "hir/types/expr.h"
#include "lir/lir.h"

lir::ValueId lir::lower_expression(Buildr *b, const hir::Expression *expression)
{
    switch (expression->kind) {
    case hir::Expression::Kind::CString:
        return b->create_cstring(expression->as.string.value);
    case hir::Expression::Kind::Integer:
        return b->create_integer(expression->as.integer.value);
    case hir::Expression::Kind::Identifier:
        if (auto var = b->find_local(expression->as.identifier.value)) {
            return b->create_deref(b->create_local_ref(*var));
        }
        return b->create_glob_ref(expression->as.identifier.value);
    case hir::Expression::Kind::FunctionCall:
        return lower_function_call(b, expression);
    default:
        MINI_UNREACHABLE("%d", expression->kind);
    }
}
