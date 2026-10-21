#include "semantics/entities/type.h"
#include "semantics/entities/symbol.h"

Mini_String sema::Type::display(Mini_Allocator a, Semantic_Storage &store) const
{
    auto &types = store.types;
    auto &symbols = store.symbols;
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
    case Type_Kind::Float32:
        return mini_string_build(a, "float");
    case Type_Kind::Float64:
        return mini_string_build(a, "float64");
    case Type_Kind::Uint:
        return mini_string_build(a, "uint");
    case Type_Kind::Struct: {
        auto *symbol = symbols.at_index_ptr(usize(*symbol_id));
        return mini_string_build(a, "%s", symbol->profile.repr_name);
    } break;
    case Type_Kind::Array: {
        Mini_String output = mini_string_build(a, "[");
        mini_string_append_fmt(
            &output,
            "%d]",
            array.element_count);
        mini_string_append_string(
            &output,
            types.at_index(usize(array.element_type)).display(a, store));
        return output;
    }
    case Type_Kind::Pointer: {
        Mini_String output = mini_string_build(a, "*");
        if (pointer.mutability)
            mini_string_append_string(&output, "const ");
        mini_string_append_string(
            &output,
            types.at_index(usize(pointer.target_type)).display(a, store));
        return output;
    }
    case Type_Kind::Function: {
        Mini_String output = mini_string_build(a, "func(");
        for (usize n = 0; n < function.parameters.count(); ++n) {
            if (n != 0)
                mini_string_append_string(&output, ", ");
            mini_string_append_string(
                &output, types.at_index(usize(function.parameters[n]))
                             .display(a, store));
        }
        mini_string_append_fmt(
            &output, ") -> %s",
            types.at_index(usize(function.return_type)).display(a, store));
        return output;
    } break;
    case Type_Kind::Uint8:
        return mini_string_build(a, "uint8");
    case Type_Kind::Namespace:
        return mini_string_build(a, "namespace");
    case Type_Kind::Error:
        return mini_string_build(a, "!error!");
    default:
        MINI_UNREACHABLE("%d", kind.kind);
    }
}

Opt<sema::Struct_Type::Field_Data> sema::Struct_Type::get_field_data(mini::StringView field_name) const
{
    for (const auto &field_data : fields.iter()) {
        if (field_data.name.value == field_name) return field_data;
    }
    return None;
}

bool sema::Type_Kind::is_some_type() const
{
    return kind == V::Struct;
}
