#pragma once

#include "ast/type.h"
#include "semantics/entities/type.h"
#include "semantics/sema.h"

#include <optional>

namespace sema
{
    Type_Id register_or_get_type(Semantic_Context *sema, sema::Type type);
    Worker_Status resolve_typehint(Semantic_Context *sema,
                                   const TypehintPointer typehint,
                                   /* target locus to link resolved type to */
                                   std::optional<Locus> resolve_location);
} // namespace sema
