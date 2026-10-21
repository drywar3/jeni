#include "ast/statements.h"
#include "semantics/impl.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_for_ever.h"

Worker_Status sema::check_for_ever_stmt(Semantic_Context *sema, void *data)
{
    auto *for_ever = (ast::stmt::For_Ever *)data;
    sema::enter_scope(sema, sema::Scope_Kind::Loop);
    Worker_Status status = sema::check_statement(sema, for_ever->body);
    status.at_scope(sema->current_scope);
    sema::leave_scope(sema);
    return status;
}

Worker_Status sema::check_break_stmt(Semantic_Context *sema, void *data)
{
    auto *break_ = (ast::stmt::Break *)data;

    const auto check_scope = [&](auto *sema, const auto &scope) {
        (void)sema;
        (void)scope;
    };

    if (!sema::find_first_scope_of(sema, sema::Scope_Kind::Loop, check_scope)) {
        sema::report_error(sema, *break_, "`break` cannot appear outside of a loop");
        return Worker_Status::Failed;
    }

    return Worker_Status::Done;
}

Worker_Status sema::check_while_loop(Semantic_Context *sema, void *data)
{
    auto *loop = (ast::stmt::While_Loop *)data;
    Worker_Status status = sema::check_expression(sema, loop->condition);
    if (!status.is_done()) return status;

    sema::enter_scope(sema, sema::Scope_Kind::Loop);
    status = sema::check_statement(sema, loop->body);
    status.at_scope(sema->current_scope);
    sema::leave_scope(sema);
    return status;
}
