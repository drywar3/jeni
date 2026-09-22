#pragma once

#include "../ast/stmt.h"
#include "../parser.h"

Statement *parse_variable_declaration(Parser *parser);
