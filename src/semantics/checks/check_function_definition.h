#pragma once

#include "ast/statements.h"
#include "semantics/sema.h"

namespace sema
{
    Worker_Status check_function_definition(Semantic_Context *sema,
                                            ast::stmt::Variable *variable,
                                            bool is_resumption);
} // namespace sema
