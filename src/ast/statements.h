#pragma once

#include "stmt.h"
#include "misc.h"
#include "expr.h"
#include "type.h"

struct StmtVariable {
    Statement base;

    Name        name;
    Mutability  mutability;
    bool        type_is_defined;
    Typehint   *typehint;
    bool        is_initialized;
    Expression *initializer;
};

struct StmtBlock {
    using Body = MINI_ARRAY(StatementPointer);
    Statement base;

    Body body;
};
