#include "semantics/entities/type.h"

Mini_String sema::Type::display(Mini_Allocator a, TypeStorage &types) const
{
    switch (kind) {
    case TypeKind::Void:
        return mini_string_build(a, "void");
    case TypeKind::String:
        return mini_string_build(a, "string");
    case TypeKind::Char:
        return mini_string_build(a, "char");
    case TypeKind::Int:
        return mini_string_build(a, "int");
    case TypeKind::Int64:
        return mini_string_build(a, "int64");
    case TypeKind::Uint:
        return mini_string_build(a, "uint");
    case TypeKind::Pointer: {
        Mini_String output = mini_string_build(a, "*");
        if (pointer.mutability)
            mini_string_append_string(&output, "const ");
        mini_string_append_string(&output, types.at_index(usize(pointer.target_type))
                                  .display(a, types));
        return output;
    }
    case TypeKind::Function: {
        Mini_String output = mini_string_build(a, "func(");
        for (usize n = 0; n < function.parameters.count();
             ++n) {
            if (n != 0)
                mini_string_append_string(&output, ", ");
            mini_string_append_string(
                                      &output, types.at_index(usize(function.parameters[n]))
                                      .display(a, types));
        }
        mini_string_append_fmt(
                               &output, ") -> %s",
                               types.at_index(usize(function.return_type))
                               .display(a, types));
        return output;
    } break;
    case TypeKind::Error:
        return mini_string_build(a, "!error!");
    default:
        MINI_UNREACHABLE("%d", kind.kind);
    }
}
