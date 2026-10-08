#pragma once

#include "stmt.h"
#include "misc.h"
#include "expr.h"
#include "type.h"

#include <mini.cc/array.h>

struct StmtVariable {
    Statement base;

    Name name;
    Mutability mutability;
    bool type_is_defined;
    Typehint *typehint;
    bool is_initialized;
    Expression *initializer;
};

struct StmtBlock {
    using Body = MINI_ARRAY(StatementPointer);

    Statement base;
    Body body;
};

struct AstIfBranch {
    Expression *condition;
    Statement  *then;
};

struct StmtIf {
    using Branches = mini::Array<AstIfBranch>;

    Statement   base;
    Expression *condition;
    Statement  *then;
    Branches    branches;
    Statement  *else_;
};

struct StmtReturn {
    Statement   base;
    Expression *value;
};
