#include "semantics/worker.h"
#include "semantics/sema.h"

Worker::Worker(Worker_Payload data, Worker_Func func, sema::Scope_Id current_scope, sema::Semantic_Id sema_id)
    : data(data), func(func), scope_id(current_scope), sema_id(sema_id)
{
}

Worker_Status Worker::resume(Semantic_Storage *store)
{
    Semantic_Context *sema        = store->semantic_contexts.get_ptr(sema_id);
    sema::Scope_Id previous_scope = sema->current_scope;
    sema->current_scope           = scope_id;
    Worker_Status status          = func(sema, data, true);
    sema->current_scope           = previous_scope;
    return status;
}
