#pragma once

#include "ast/expr.h"
#include "parser/parser.h"

namespace parser
{
    Expression *parse_struct_decl(Parser *p);
} // namespace parser
