#pragma once

#include "semantics/sema.h"
#include "semantics/worker.h"

namespace sema
{
    bool discover_statement(Semantic_Context *sema, StatementPointer stmt, sema::Scope_Id scope_id);
    Worker_Status check_statement(Semantic_Context *sema, Worker_Payload data,
                                  bool is_resumption = false);
} // namespace sema
