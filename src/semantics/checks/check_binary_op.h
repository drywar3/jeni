#pragma once

#include "ast/expressions.h"
#include "semantics/sema.h"

namespace sema
{
    Worker_Status check_binary_op(Semantic_Context *sema,
                                  ast::expr::Binary_Operation *binop);
} // namespace sema
