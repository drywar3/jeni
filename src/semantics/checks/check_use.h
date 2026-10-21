#pragma once

#include "semantics/sema.h"
#include "ast/expr.h"

namespace sema
{
    bool discover_use_expr(Semantic_Context *sema, Expression *expr, sema::Scope_Id scope_id);
} // namespace sema
