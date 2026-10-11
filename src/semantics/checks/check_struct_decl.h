#pragma once

#include "ast/expr.h"
#include "ast/statements.h"
#include "semantics/sema.h"

namespace sema
{
    Worker_Status check_struct_decl(Semantic_Context *sema, ast::stmt::Variable *variable, bool is_resumption);
} // namespace sema
