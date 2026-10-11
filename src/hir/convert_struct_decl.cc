#include "hir/convert.h"

hir::Statement *hir::convert_struct_decl(hir::Context *ctx, const ast::stmt::Variable *variable)
{
    auto *symbol = ctx->symbols().find(variable->name.locus);

    MINI_ASSERT(symbol->kind == sema::Symbol_Kind::Type, "expected a type");

    auto type_id = symbol->type.type_id;
    const auto &layout = ctx->types().at_index(usize(type_id));

    MINI_ASSERT(layout.kind == sema::Type_Kind::Struct, "expected a type");

    mini::Array<hir::stmt::Struct::Field> fields{ctx->allocator};
    for (const auto &field : layout.strukt.fields.iter()) {
        fields.append(hir::stmt::Struct::Field{field.name.value, field.type_id});
    }

    return ctx->new_stmt(hir::Statement::Kind::Struct,
                         hir::stmt::Struct{variable->name.value, fields});
}
