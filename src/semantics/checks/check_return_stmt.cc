#include "ast/statements.h"
#include "semantics/impl.h"
#include "semantics/type/coercer.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_return_stmt.h"

WorkerStatus sema::check_return_stmt(SemanticContext *sema, void *data)
{
    StmtReturn *ret = (StmtReturn*)data;
    WorkerStatus status = sema::check_expression(sema, ret->value);
    if (status != WorkerStatus::Done)
        return status;

    sema::TypeId ret_val_type_id = sema::get_type_at_locus(sema, ret->value->locus);
    sema::find_first_scope_of(sema, sema::ScopeKind::Function, [&] (auto *sema, const auto &scope) {
        if (!sema::try_coerce_type_into(sema,
                                        scope.function.return_type, ret_val_type_id,
                           /* strict */ false)) {
            Diagnostic diag = diag_create(
                                          Severity::Error, ret->value->locus,
                                          "invalid return value type",
                                          mini_string_build(sema->allocator,
                                                            "expected expression with type of `%s`",
                                                            sema::display_type(sema, scope.function.return_type)));
            sema::report(sema, diag);
            status = WorkerStatus::Failed;
        }
    });

    return status;
}
