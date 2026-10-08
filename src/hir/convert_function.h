#pragma once

#include "hir/convert.h"
#include "hir/types/stmt.h"
#include "ast/statements.h"

namespace hir
{
    Statement *convert_function_stmt(hir::Context *ctx, const ast::stmt::Variable *var);
} // namespace hir
