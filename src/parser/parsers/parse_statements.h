#pragma once

#include "ast/stmt.h"
#include "parser/parser.h"

namespace parser
{
    Statement *parse_variable_declaration(Parser *parser);
    Statement *parse_block(Parser *parser);
    Statement *parse_if_statement(Parser *parser);
    Statement *parse_return_statement(Parser *parser);
    Statement *parse_for_loop(Parser *parser);
    Statement *parse_break(Parser *parser);
} // namespace parser
