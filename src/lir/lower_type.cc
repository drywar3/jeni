#include "lir/lir.h"

lir::TypePtr lir::lower_type(Buildr *builder, sema::TypeId type_id)
{
    lir::Type type;
    const sema::Type &type_layout = builder->get_type_layout(type_id);

    switch (type_layout.kind) {
    case sema::TypeKind::Int:
        type.kind = lir::Type::Kind::Int32;
        break;
    case sema::TypeKind::Void:
        type.kind = lir::Type::Kind::Void;
        break;
    case sema::TypeKind::String:
        type.kind = lir::Type::Kind::String;
        break;
    default: MINI_UNREACHABLE();
    }

    return Box(builder->allocator(), type);
}
