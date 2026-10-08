#pragma once

#include "semantics/sema.h"

namespace sema
{
    Worker_Status check_if_stmt(Semantic_Context *sema, void *data);
} // namespace sema
