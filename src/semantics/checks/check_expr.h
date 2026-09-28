#pragma once

#include "semantics/sema.h"

namespace sema
{
    WorkerStatus check_expression(SemanticContext *sema, void *data);
} // namespace sema
