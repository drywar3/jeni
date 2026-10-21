#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/checks/check_expr.h"
#include "semantics/checks/check_member_access.h"

Worker_Status sema::check_member_access(Semantic_Context *sema, Expression *expr)
{
    auto *member_access = (ast::expr::Member_Access*)expr;
    auto  status        = sema::check_expression(sema, member_access->receiver);
    if (!status.is_done()) return status;

    sema::Type_Id type_id = sema::get_type_at_locus(sema, member_access->receiver->locus);

    int auto_pointer_deref_level = 0;

    const auto check_struct_member_access = [&](auto *layout) {
        const auto &strukt = layout->strukt;
        if (auto field_data = strukt.get_field_data(member_access->member.value);
            !field_data.has_value()) {
            sema::report_error(sema, member_access->member,
                               "invalid member name",
                               mini_string_build(sema->allocator,
                                                 "type `%s` has no member `%.*s`",
                                                 sema::display_type(sema, type_id),
                                                 SVARG(member_access->member.value.base())));
            return false;
        } else {
            sema::link_locus_to_type(sema, expr->locus, field_data->type_id);
            return true;
        }
    };

    const auto check_namespace_access = [&] (auto *layout) -> bool {
        auto symbol_opt = sema::get_symbol_at_locus(sema, member_access->receiver->locus);
        MINI_ASSERT(symbol_opt.has_value(), );

        auto symbol   = *symbol_opt;
        auto scope_id = symbol->ns.scope_id;
        auto &scope   = sema->scopes().at_index(usize(scope_id));

        if (!sema::scope_has_symbol(&scope, member_access->member.value)) {
            sema::report_error(sema, member_access->member,
                               "use of undeclared symbol",
                               mini_string_build(sema->allocator, "cannot find `%.*s` in `%.*s`",
                                                 SVARG(member_access->member.value.base()),
                                                 SVARG(symbol->name.base())));
            sema::link_locus_to_type(sema, expr->locus, sema::type_id::Error);
            return false;

        }

        auto member_symbol = *sema::find_symbol_in(sema, scope_id, member_access->member.value);
        sema::link_locus_to_type_at_locus(sema, member_symbol->locus, expr->locus);
        sema::link_locus_to_symbol(sema, expr->locus, member_symbol.get_id());
        sema::link_locus_to_symbol(sema, member_access->member.locus, member_symbol.get_id());
        return true;
    };

    const auto check_member_access_on_id = [&](this auto &&self, sema::Type_Id type_id) {
        const sema::Type *layout = sema::get_type_by_id(sema, type_id);
        switch (layout->kind) {
        case sema::Type_Kind::Struct:
            if (!check_struct_member_access(layout)) return Worker_Status::Failed;
            break;
        case sema::Type_Kind::Namespace:
            if (!check_namespace_access(layout)) return Worker_Status::Failed;
            break;
        case sema::Type_Kind::Pointer: {
            if (auto_pointer_deref_level == 0) {
                const auto &pointer = layout->pointer;
                auto_pointer_deref_level += 1;
                return self(pointer.target_type);
            }
            /* fallthrough */
        }
        default:
            sema::report_error(sema, member_access->member,
                               "invalid member name",
                               mini_string_build(sema->allocator, "type `%s` has no member `%.*s`",
                                                 sema::display_type(sema, type_id),
                                                 SVARG(member_access->member.value.base())));
            return Worker_Status::Failed;
        }

        return Worker_Status::Done;
    };

    return check_member_access_on_id(type_id);
}
