#include "hir/convert.h"

hir::Expression *hir::convert_member_access(hir::Context *ctx,
                                            const ast::expr::Member_Access *member_access)
{
    auto type_id     = sema::Type_Id(*ctx->types().get_id(member_access->locus));
    hir::Expression *receiver = hir::convert_expression(ctx, member_access->receiver);

    const auto *layout = ctx->types().find(member_access->receiver->locus);
    if (layout->kind == sema::Type_Kind::Pointer) {
        receiver = ctx->create_dereference(type_id, receiver);
    }


    return ctx->new_expr(hir::Expression::Kind::Member_Access,
                         type_id,
                         hir::expr::Member_Access{receiver, member_access->member.value});
}
