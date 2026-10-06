#include "semantics/sema.h"
#include "semantics/worker.h"

Worker::Worker(void *data, WorkerFunc func, sema::ScopeId current_scope)
    : data(data), func(func), scope_id(current_scope)
{
}

WorkerStatus Worker::resume(SemanticContext *sema)
{
    sema::ScopeId previous_scope = sema->current_scope;
    sema->current_scope = scope_id;
    WorkerStatus status = func(sema, data, true);
    sema->current_scope = previous_scope;
    return status;
}
