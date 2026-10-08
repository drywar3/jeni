#include "semantics/entities/type.h"

Mini_String sema::Type::display(Mini_Allocator a, Type_Storage &types) const
{
    switch (kind) {
    case Type_Kind::Void:
        return mini_string_build(a, "void");
    case Type_Kind::String:
        return mini_string_build(a, "string");
    case Type_Kind::Char:
        return mini_string_build(a, "char");
    case Type_Kind::Int:
        return mini_string_build(a, "int");
    case Type_Kind::Int64:
        return mini_string_build(a, "int64");
    case Type_Kind::Uint:
        return mini_string_build(a, "uint");
    case Type_Kind::Pointer: {
        Mini_String output = mini_string_build(a, "*");
        if (pointer.mutability)
            mini_string_append_string(&output, "const ");
        mini_string_append_string(
            &output,
            types.at_index(usize(pointer.target_type)).display(a, types));
        return output;
    }
    case Type_Kind::Function: {
        Mini_String output = mini_string_build(a, "func(");
        for (usize n = 0; n < function.parameters.count(); ++n) {
            if (n != 0)
                mini_string_append_string(&output, ", ");
            mini_string_append_string(
                &output, types.at_index(usize(function.parameters[n]))
                             .display(a, types));
        }
        mini_string_append_fmt(
            &output, ") -> %s",
            types.at_index(usize(function.return_type)).display(a, types));
        return output;
    } break;
    case Type_Kind::Error:
        return mini_string_build(a, "!error!");
    default:
        MINI_UNREACHABLE("%d", kind.kind);
    }
}
