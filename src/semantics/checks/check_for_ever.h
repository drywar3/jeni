#pragma once

#include "semantics/sema.h"

namespace sema
{
    Worker_Status check_for_ever_stmt(Semantic_Context *sema, void *data);
    Worker_Status check_break_stmt(Semantic_Context *sema, void *data);
} // namespace sema
