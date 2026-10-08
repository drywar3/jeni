#pragma once

#include "semantics/sema.h"

namespace sema
{
    bool coerce_type_into(Semantic_Context *sema, Type_Id target,
                          Type_Id source, Locus target_locus,
                          Locus source_locus, bool strict = true);
    bool try_coerce_type_into(Semantic_Context *sema, Type_Id target,
                              Type_Id source, bool strict = true);
} // namespace sema
