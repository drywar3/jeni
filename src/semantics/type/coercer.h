#pragma once

#include "semantics/sema.h"

namespace sema
{
    bool coerce_type_into(SemanticContext *sema, TypeId target, TypeId source,
                          Locus target_locus, Locus source_locus,
                          bool strict = true);
} // namespace sema
