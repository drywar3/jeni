#pragma once

#include "semantics/sema.h"
#include "ast/statements.h"

namespace sema
{
    Worker_Status check_namespace_definition(Semantic_Context *sema, ast::stmt::Variable *variable, bool is_resumption = false);
} // namespace sema
