#pragma once

#include "semantics/sema.h"
#include "semantics/worker.h"

namespace sema
{
    bool discover_statement(Semantic_Context *sema, StatementPointer stmt);
    Worker_Status check_statement(Semantic_Context *sema, void *data,
                                  bool is_resumption = false);
} // namespace sema
