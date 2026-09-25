#pragma once

#include "ast/stmt.h"
#include "parser/parser.h"

Statement *parse_variable_declaration(Parser *parser);
