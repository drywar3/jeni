#include "ast/statements.h"
#include "ast/stmt.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/impl.h"
#include "semantics/sema.h"

#include <mini.cc/scope_guard.h>

Worker_Status check_block(Semantic_Context *sema, ast::stmt::Block *block,
                          bool is_resumption)
{
    if (!is_resumption)
        sema::enter_scope(sema, sema::Scope_Kind::Block);
    auto scope_end = mini::ScopeGuard([&] {
        if (!is_resumption)
            sema::leave_scope(sema);
    });

    usize start = 0;
    if (sema->block_has_save_point(block->locus)) {
        start = sema->get_block_save_point(block->locus);
    }

    for (usize n = start; n < mini_array_count(block->body); ++n) {
        StatementPointer stmt = block->body[n];
        /* pass `is_resumption` only to the first statement reached when
         * resuming */
        bool stmt_is_resumption = (n == start) ? is_resumption : false;
        Worker_Status s = sema::check_statement(sema, stmt, stmt_is_resumption);
        s.at_scope(sema->current_scope);
        if (s != Worker_Status::Done) {
            /* do not advance save point past a failing statement */
            return s;
        }
    }

    return Worker_Status::Done;
}
