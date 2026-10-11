#pragma once

#include "ast/expr.h"
#include "semantics/sema.h"

namespace sema
{
    Worker_Status check_member_access(Semantic_Context *sema, Expression *data);
} // namespace sema
