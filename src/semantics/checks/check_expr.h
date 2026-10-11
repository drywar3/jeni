#pragma once

#include "semantics/sema.h"
#include "ast/expressions.h"

namespace sema
{
    Worker_Status check_expression(Semantic_Context *sema, void *data);
    Worker_Status check_identifier(Semantic_Context *sema, ast::expr::Identifier *ident);
} // namespace sema
