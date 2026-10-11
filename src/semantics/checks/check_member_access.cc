#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_member_access.h"

Worker_Status sema::check_member_access(Semantic_Context *sema, Expression *expr)
{
    auto *member_access = (ast::expr::Member_Access*)expr;
    Worker_Status status = sema::check_expression(sema, member_access->receiver);
    if (!status.is_done()) return status;

    sema::Type_Id type_id = sema::get_type_at_locus(sema, member_access->receiver->locus);
    const sema::Type *layout = sema::get_type_by_id(sema, type_id);

    switch (layout->kind) {
    case sema::Type_Kind::Struct: {
        const auto &strukt = layout->strukt;
        if (auto field_data = strukt.get_field_data(member_access->member.value);
            !field_data.has_value()) {
            sema::report_error(sema, member_access->member,
                               "invalid member name",
                               mini_string_build(sema->allocator,
                                                 "type `%s` has no member `%.*s`",
                                                 sema::display_type(sema, type_id),
                                                 SVARG(member_access->member.value.base())));
            return Worker_Status::Failed;
        } else {
            sema::link_locus_to_type(sema, expr->locus, field_data->type_id);
        }
    } break;
    default:
        sema::report_error(sema, member_access->member,
                           "invalid member name",
                           mini_string_build(sema->allocator, "type `%s` has no member `%.*s`",
                                             sema::display_type(sema, type_id),
                                             SVARG(member_access->member.value.base())));
        return Worker_Status::Failed;
    }

    return Worker_Status::Done;
}
