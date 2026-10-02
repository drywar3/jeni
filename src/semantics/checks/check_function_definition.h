#pragma once

#include "semantics/sema.h"

namespace sema
{
    WorkerStatus check_function_definition(SemanticContext *sema,
                                           StmtVariable *variable, bool is_resumption);
} // namespace sema
