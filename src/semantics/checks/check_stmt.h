#pragma once

#include "semantics/sema.h"
#include "semantics/worker.h"

bool discover_statement(SemanticContext *sema, StatementPointer stmt);
WorkerStatus check_statement(SemanticContext *sema, void *data);
