#pragma once

#include "parser/parser.h"
#include "ast/expr.h"


namespace parser
{
    Expression *parse_expression(Parser *parser, bool is_within_typehint_context = false);
    Expression *parse_typehint_expression(Parser *parser);
    Expression *error_expr();
} // namespace parser
