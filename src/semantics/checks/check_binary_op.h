#pragma once

#include "semantics/sema.h"
#include "ast/expressions.h"

namespace sema
{
    WorkerStatus check_binary_op(SemanticContext *sema, ExprBinaryOperation *binop);
} // namespace sema
