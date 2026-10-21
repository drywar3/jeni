#pragma once

#include "parser/parser.h"
#include "ast/expressions.h"

namespace parser
{
    Expression *parse_namespace(Parser *p);
} // namespace parser
