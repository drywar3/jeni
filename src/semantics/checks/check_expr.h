#pragma once

#include "semantics/sema.h"
#include "ast/expressions.h"

namespace sema
{
    Worker_Status check_expression(Semantic_Context *sema, Expression *&data, bool is_resumption = false);
    Worker_Status check_identifier(Semantic_Context *sema, ast::expr::Identifier *ident);

    /* call this after the expression is successfully resolved */
    bool expression_is_value_compatible(Semantic_Context *sema, Expression *expr);
} // namespace sema
