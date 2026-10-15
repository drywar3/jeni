#include "ast/type.h"
#include "ast/types.h"
#include "semantics/impl.h"
#include "semantics/sema.h"
#include "mini.c/mini_def.h"
#include "ast/expressions.h"
#include "semantics/worker.h"
#include "semantics/type/ids.h"
#include "semantics/entities/type.h"
#include "semantics/type/resolver.h"
#include "semantics/checks/check_expr.h"

sema::Type_Id sema::register_or_get_type(Semantic_Context *sema,
                                         sema::Type type)
{
    usize index = 0;
    for (const auto &_type : sema->store->types) {
        if (_type == type) {
            sema::Type_Id id = sema::Type_Id(index);
            return id;
        }
        index += 1;
    }
    auto id = sema::Type_Id(sema->store->types.add_value(type));
    return id;
}


static Worker_Status
resolve_array_typehint(Semantic_Context *sema, const Typehint *typehint,
                          std::optional<Locus> resolve_location)
{
    const auto is_valid_element_count_expr = [&] (const auto *expr) -> bool {
        switch (expr->kind) {
        case EXPR_Integer:
            return true;
        case EXPR_Identifier: {
            MINI_UNREACHABLE();
            auto symbol = *sema::get_symbol_at_locus(sema, expr->locus);
            if (symbol->kind != sema::Symbol_Kind::Variable) return false;
            if (!symbol->variable.mutability) return false;
        } break;
        default: return false;
        }
    };

    const auto *array = (const ast::type::Array *)typehint;
    auto status       = sema::check_expression(sema, array->element_type);
    if (!status.is_done()) return status;

    status       = sema::check_expression(sema, array->element_count);
    if (!status.is_done()) return status;


    if (!is_valid_element_count_expr(array->element_count)) {
        sema::report_error(sema, *array->element_count,
                           "invalid expression",
                           "expected an integer literal or constant variable");
        return Worker_Status::Failed;
    }

    usize element_count = ((ast::expr::Integer *)(array->element_count))->value;
    auto  element_type  = sema::get_type_at_locus(sema, array->element_type->locus);
    auto type_id        = sema::register_or_get_type(sema, sema::Type::Array(element_count, element_type));

    if (resolve_location) {
        sema::link_locus_to_type(sema, *resolve_location, type_id);
    }
    return Worker_Status::Done;
}

static Worker_Status
resolve_pointer_typehint(Semantic_Context *sema, const Typehint *typehint,
                         std::optional<Locus> resolve_location)
{
    ast::type::Pointer *pointer = (ast::type::Pointer *)typehint;
    Worker_Status status = sema::check_expression(sema, pointer->typehint);

    if (status != Worker_Status::Done)
        return status;

    sema::Type_Id target_type_id =
        sema::get_type_at_locus(sema, pointer->typehint->locus);

    sema::Type pointer_type =
        sema::Type::Pointer(pointer->mutability, target_type_id);
    sema::Type_Id type_id = sema::register_or_get_type(sema, pointer_type);

    if (resolve_location) {
        sema::link_locus_to_type(sema, *resolve_location, type_id);
    }

    return Worker_Status::Done;
}

Worker_Status sema::resolve_typehint(Semantic_Context *sema,
                                     const Expression *ty_expr,
                                     std::optional<Locus> resolve_location)
{
    MINI_ASSERT(ty_expr != nullptr, "invalid typehint");

    if (ty_expr->kind == EXPR_Identifier) {
        auto *ident = (ast::expr::Identifier *)ty_expr;
        Worker_Status status = check_identifier(sema, ident);
        if (status != Worker_Status::Done) return status;
        if (auto sym_opt = sema::eagerly_lookup_symbol(sema, sema->current_scope, ident->value);
            sym_opt.has_value()) {
            auto symbol = *sym_opt;
            if (!symbol->is(sema::Symbol_Kind::Type)) {
                sema::report_error(sema,
                                   *ty_expr,
                                   "expected a type",
                                   "this symbol does not refer to a type");
                return Worker_Status::Failed;
            }

            if (resolve_location.has_value()) {
                sema::link_locus_to_type(sema, *resolve_location, symbol->type.type_id);
            }
        }
        return Worker_Status::Done;
    }

    MINI_ASSERT(ty_expr->kind == EXPR_Typehint, );

    Type_Id id;

    const auto *typehint = (const Typehint *)ty_expr;

    switch (typehint->kind) {
    case TYPEHINT_Integer: {
        const auto *integer = (const ast::type::Integer *)typehint;
        switch (integer->kind) {
        case ast::type::Integer::Int:
            id = sema::type_id::Int;
            break;
        case ast::type::Integer::Int64:
            id = sema::type_id::Int64;
            break;
        case ast::type::Integer::Uint8:
            id = sema::type_id::Uint8;
            break;
        default:
            MINI_UNREACHABLE("%d", integer->kind);
        }
    } break;

    case TYPEHINT_Float: {
        const auto *float_ = (const ast::type::Float *)typehint;
        switch (float_->width) {
        case ast::type::Float::Float32:
            id = sema::type_id::Float32;
            break;
        case ast::type::Float::Float64:
            id = sema::type_id::Float64;
            break;
        default:
            MINI_UNREACHABLE();
        }
    } break;

    case TYPEHINT_String:
        id = sema::type_id::String;
        break;
    case TYPEHINT_Pointer:
        return resolve_pointer_typehint(sema, (const Typehint *)typehint, resolve_location);
    case TYPEHINT_Array: return resolve_array_typehint(sema, typehint, resolve_location);
    case TYPEHINT_Bool:
        id = sema::type_id::Bool;
        break;
    case TYPEHINT_Char:
        id = sema::type_id::Char;
        break;
    case TYPEHINT_Void:
        id = sema::type_id::Void;
        break;
    default:
        MINI_UNREACHABLE("TODO");
    }

    if (resolve_location.has_value())
        sema::link_locus_to_type(sema, *resolve_location, id);
    return Worker_Status::Done;
}
