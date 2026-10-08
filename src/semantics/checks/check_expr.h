#pragma once

#include "semantics/sema.h"

namespace sema
{
    Worker_Status check_expression(Semantic_Context *sema, void *data);
} // namespace sema
