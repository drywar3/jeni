#pragma once

#include "semantics/sema.h"

namespace sema
{
    WorkerStatus check_return_stmt(SemanticContext *sema, void *data);
} // namespace sema
