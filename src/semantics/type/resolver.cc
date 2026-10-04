#include "semantics/impl.h"
#include "semantics/type/ids.h"
#include "semantics/type/resolver.h"
#include "semantics/entities/type.h"

sema::TypeId sema::register_or_get_type(SemanticContext *sema, sema::Type type)
{
    usize index = 0;
    for (const auto &_type : sema->store->types) {
        if (_type == type) {
            sema::TypeId id = (sema::TypeId)(index);
            return id;
        }
        index += 1;
    }
    return (sema::TypeId)sema->store->types.add_value(type);
}

WorkerStatus sema::resolve_typehint(SemanticContext *sema,
                                    const TypehintPointer typehint,
                                    std::optional<Locus> resolve_location)
{
    MINI_ASSERT(typehint != nullptr, "invalid typehint");

    TypeId id;

    switch (typehint->kind) {
    case TYPEHINT_Integer: {
        id = sema::type_id::Int;
    } break;
    case TYPEHINT_String:
        id = sema::type_id::String;
        break;
    default:
        MINI_UNREACHABLE("TODO");
    }

    if (resolve_location.has_value())
        sema::link_locus_to_type(sema, *resolve_location, id);
    return WorkerStatus::Done;
}
