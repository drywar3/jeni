#pragma once

#include "ast/stmt.h"
#include "parser/parser.h"

namespace parser
{
    Statement *parse_variable_declaration(Parser *parser);
    Statement *parse_block(Parser *parser);
    Statement *parse_if_statement(Parser *parser);
    Statement *parse_return_statement(Parser *parser);
} // namespace parser
