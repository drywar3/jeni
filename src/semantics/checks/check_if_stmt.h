#pragma once

#include "semantics/sema.h"

namespace sema
{
    WorkerStatus check_if_stmt(SemanticContext *sema, void *data);
} // namespace sema
