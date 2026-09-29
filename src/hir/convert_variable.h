#pragma once

#include "ast/stmt.h"
#include "hir/convert.h"

namespace hir
{
    hir::Statement *convert_variable_stmt(hir::Context *ctx, const ::Statement *stmt);
} // namespace hir
