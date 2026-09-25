#pragma once

#include "parser/parser.h"
#include "ast/expr.h"

ExpressionPointer parser_parse_expression(Parser *parser);
