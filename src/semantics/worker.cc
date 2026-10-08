#include "semantics/worker.h"
#include "semantics/sema.h"

Worker::Worker(void *data, Worker_Func func, sema::Scope_Id current_scope)
    : data(data), func(func), scope_Id(current_scope)
{
}

Worker_Status Worker::resume(Semantic_Context *sema)
{
    sema::Scope_Id previous_scope = sema->current_scope;
    sema->current_scope           = scope_Id;
    Worker_Status status          = func(sema, data, true);
    sema->current_scope           = previous_scope;
    return status;
}
