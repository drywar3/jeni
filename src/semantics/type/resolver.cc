#include "semantics/type/resolver.h"
#include "ast/type.h"
#include "ast/types.h"
#include "mini.c/mini_def.h"
#include "semantics/entities/type.h"
#include "semantics/impl.h"
#include "semantics/sema.h"
#include "semantics/type/ids.h"
#include "semantics/worker.h"

sema::TypeId sema::register_or_get_type(SemanticContext *sema, sema::Type type)
{
    usize index = 0;
    for (const auto &_type : sema->store->types) {
        if (_type == type) {
            sema::TypeId id = sema::TypeId(index);
            return id;
        }
        index += 1;
    }
    auto id = sema::TypeId(sema->store->types.add_value(type));
    return id;
}

static WorkerStatus
resolve_pointer_typehint(SemanticContext *sema, const Typehint *typehint,
                         std::optional<Locus> resolve_location)
{
    TypePointer *pointer = (TypePointer *)typehint;
    WorkerStatus status  = sema::resolve_typehint(sema, pointer->typehint,
                                                  pointer->typehint->locus);
    if (status != WorkerStatus::Done)
        return status;

    sema::TypeId target_type_id =
        sema::get_type_at_locus(sema, pointer->typehint->locus);

    sema::Type pointer_type =
        sema::Type::Pointer(pointer->mutability, target_type_id);
    sema::TypeId type_id = sema::register_or_get_type(sema, pointer_type);

    if (resolve_location) {
        sema::link_locus_to_type(sema, *resolve_location, type_id);
    }

    return WorkerStatus::Done;
}

WorkerStatus sema::resolve_typehint(SemanticContext *sema,
                                    const TypehintPointer typehint,
                                    std::optional<Locus> resolve_location)
{
    MINI_ASSERT(typehint != nullptr, "invalid typehint");

    TypeId id;

    switch (typehint->kind) {
    case TYPEHINT_Integer: {
        const auto *integer = (const TypeInteger*)typehint;
        switch (integer->kind) {
        case TypeInteger::Int:
            id = sema::type_id::Int;
            break;
        case TypeInteger::Int64:
            id = sema::type_id::Int64;
            break;
        default: MINI_UNREACHABLE();
        }
    } break;
    case TYPEHINT_String:
        id = sema::type_id::String;
        break;
    case TYPEHINT_Pointer:
        return resolve_pointer_typehint(sema, typehint, resolve_location);
    case TYPEHINT_Char:
        id = sema::type_id::Char;
        break;
    default:
        MINI_UNREACHABLE("TODO");
    }

    if (resolve_location.has_value())
        sema::link_locus_to_type(sema, *resolve_location, id);
    return WorkerStatus::Done;
}
