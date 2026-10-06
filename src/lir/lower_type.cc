#include "lir/lir.h"
#include "semantics/entities/type.h"

lir::TypePtr lir::lower_type(Buildr *builder, sema::TypeId type_id)
{
    lir::Type type{};
    const sema::Type &type_layout = builder->get_type_layout(type_id);

    switch (type_layout.kind) {
    case sema::TypeKind::Int:
        type.kind = lir::Type::Kind::Int32;
        break;
    case sema::TypeKind::Int64:
        type.kind = lir::Type::Kind::Int64;
        break;
    case sema::TypeKind::Void:
        type.kind = lir::Type::Kind::Void;
        break;
    case sema::TypeKind::String:
        type.kind = lir::Type::Kind::String;
        break;
    case sema::TypeKind::Pointer: {
        auto inner = lir::lower_type(builder, type_layout.pointer.target_type);
        return lir::Type::PointerTo(builder->allocator(),
                                    type_layout.pointer.mutability, inner);
    } break;
    case sema::TypeKind::Char:
        type.kind = lir::Type::Kind::Int8;
        break;
    case sema::TypeKind::Function: {
        auto ret_type = lir::lower_type(builder, type_layout.function.return_type);
        auto params   = mini::Array<lir::TypePtr>(builder->allocator());
        for (const auto &param_type : type_layout.function.parameters.iter()) {
            params.append(lir::lower_type(builder, param_type));
        }
        return lir::Type::FunctionPointer(builder->allocator(), ret_type, params);
    } break;
    default:
        MINI_UNREACHABLE();
    }

    return Box(builder->allocator(), type);
}
