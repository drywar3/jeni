#include "semantics/impl.h"
#include "semantics/checks/check_stmt.h"
#include "semantics/checks/check_namespace_definition.h"

#include <mini.cc/scope_guard.h>

/* namespaces are not allowed in function bodies. so we can assume that we are always at the global scope
 * when evaluating a namespace.
 *
 * invariance is ensured by the parser.
 */
Worker_Status sema::check_namespace_definition(
    Semantic_Context *sema,
    ast::stmt::Variable *variable,
    bool is_resumption)
{
    MINI_ASSERT(variable->initializer->kind == EXPR_Namespace, "expected namespace initializer");
    auto *ns = (ast::expr::Namespace*)variable->initializer;

    auto symbol_opt = sema::get_symbol_at_locus(sema, variable->name.locus);
    MINI_ASSERT(symbol_opt.has_value(), "could not find symbol");

    auto symbol     = *symbol_opt;
    symbol->kind    = sema::Symbol_Kind::Namespace;

    usize start = 0;

    if (is_resumption && sema->block_has_save_point(variable->name.locus)) {
        start = sema->get_block_save_point(variable->name.locus);
    }

    sema::Scope_Id scope_id    = sema->current_scope;
    if (!is_resumption) {
        scope_id = sema::enter_scope(sema, sema::Scope_Kind::Namespace);
    }

    auto current_namespace = sema->get_current_namespace();

    symbol->ns.scope_id         = scope_id;
    symbol->ns.parent_namespace = current_namespace;

    auto save = sema->set_current_namespace(symbol.get_id());

    auto scope_guard = mini::ScopeGuard([&] () {
        if (!is_resumption)
            sema::leave_scope(sema);
        sema->set_current_namespace(save);
    });


    auto *definitions = (ast::stmt::Block*)ns->definitions;

    sema::link_locus_to_type(sema, variable->name.locus, sema::type_id::BT_Namespace);
    symbol->set_state(sema::Symbol_State::Resolved);

    if (!is_resumption) {
        for (auto &definition : definitions->body.iter()) {
            if (!sema::discover_statement(sema, definition, scope_id)) return Worker_Status::Failed;
        }
    }

    Worker_Status status = Worker_Status::Done;
    for (usize n = start; n < definitions->body.count(); ++n) {
        auto &definition        = definitions->body[n];
        bool stmt_is_resumption = (n == start) ? is_resumption : false;
        status             = sema::check_statement(sema, definition, stmt_is_resumption);
        if (status == Worker_Status::Failed) {
            return status;
        } else if (status == Worker_Status::Pending) {
            for (const auto symbol_id : mini::iterate(status.waiting_on)) {
                sema->register_worker(
                    symbol_id,
                    Worker(definition, sema::check_statement,
                           status.working_scope.value_or(scope_id), sema->my_id));
            }
        }
        sema->set_block_save_point(variable->name.locus, n);
    }

    return status;
}
