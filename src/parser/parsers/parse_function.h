#pragma once

#include "parser/parser.h"
#include "ast/expr.h"

ExpressionPointer parse_function(Parser *p);
