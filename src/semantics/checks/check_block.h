#pragma once

#include "ast/statements.h"
#include "semantics/sema.h"

/* this function will not register any workers */
Worker_Status check_block(Semantic_Context *sema, ast::stmt::Block *block,
                          bool is_resumption);
