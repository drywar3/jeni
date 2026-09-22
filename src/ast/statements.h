#pragma once

#include "stmt.h"
#include "misc.h"
#include "expr.h"
#include "type.h"

typedef struct {
    Statement base;

    Name        name;
    Mutability  mutability;
    bool        type_is_defined;
    Typehint   *typehint;
    bool        is_initialized;
    Expression *initializer;
} StmtVariable;
