#include "ast/statements.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/checks/check_if_stmt.h"

WorkerStatus sema::check_if_stmt(SemanticContext *sema, void *data)
{
    const auto check_if_header = [] (SemanticContext *sema, const auto &header) -> WorkerStatus {
        WorkerStatus status = sema::check_expression(sema, header.condition);
        if (status != WorkerStatus::Done)
            return status;

        status = check_statement(sema, header.then);
        if (status != WorkerStatus::Done)
            return status;
        return status;
    };

    StmtIf *if_stmt = (StmtIf*)data;

    Expression *condition = if_stmt->condition;
    Statement *then       = if_stmt->then;

    MINI_ASSERT(condition,);
    MINI_ASSERT(then,);

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

    return WorkerStatus::Done;
}
