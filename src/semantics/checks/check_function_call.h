#pragma once

#include "semantics/sema.h"
#include "ast/expressions.h"

namespace sema
{
    WorkerStatus check_function_call(SemanticContext *sema, ExprFunctionCall *call);
} // namespace sema
