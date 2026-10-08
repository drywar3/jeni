#pragma once

#include "ast/expressions.h"
#include "semantics/sema.h"

namespace sema
{
    Worker_Status check_function_call(Semantic_Context *sema,
                                      ast::expr::Function_Call *call);
} // namespace sema
