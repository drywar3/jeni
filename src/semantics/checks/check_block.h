#pragma once

#include "ast/statements.h"
#include "semantics/sema.h"

/* this function will not register any workers */
WorkerStatus check_block(SemanticContext *sema, StmtBlock *block,
                         bool is_resumption);
