#pragma once

#include "ast/type.h"
#include "semantics/sema.h"
#include "semantics/entities/type.h"

#include <optional>

namespace sema
{
    TypeId register_or_get_type(SemanticContext *sema, sema::Type type);
    WorkerStatus resolve_typehint(SemanticContext *sema,
                                  const TypehintPointer typehint,
                                  /* target locus to link resolved type to */
                                  std::optional<Locus> resolve_location);
} // namespace sema
