#include "semantics/checks/check_if_stmt.h"
#include "ast/statements.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_stmt.h"

Worker_Status sema::check_if_stmt(Semantic_Context *sema, void *data)
{
    const auto check_if_header = [](Semantic_Context *sema,
                                    const auto &header) -> Worker_Status {
        Worker_Status status = sema::check_expression(sema, header.condition);
        if (status != Worker_Status::Done)
            return status;

        status = check_statement(sema, header.then);
        if (status != Worker_Status::Done)
            return status;
        return status;
    };

    ast::stmt::If *if_stmt = (ast::stmt::If *)data;

    Expression *condition = if_stmt->condition;
    Statement *then       = if_stmt->then;

    MINI_ASSERT(condition, );
    MINI_ASSERT(then, );

    if (auto status = check_if_header(sema, *if_stmt); !status.is_done())
        return status;

    for (const auto &branch : if_stmt->branches.iter()) {
        if (auto status = check_if_header(sema, branch); !status.is_done())
            return status;
    }

    if (if_stmt->else_) {
        if (auto status = sema::check_statement(sema, if_stmt->else_);
            !status.is_done())
            return status;
    }

    return Worker_Status::Done;
}
