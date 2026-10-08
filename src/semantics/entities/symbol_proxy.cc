#include "semantics/entities/symbol_proxy.h"

sema::Symbol_Proxy::Symbol_Proxy(Semantic_Storage *store, sema::Symbol_Id id)
    : id(id), store(store)
{
}

const sema::Symbol *sema::Symbol_Proxy::operator->() const
{
    return store->symbols.at_index_ptr(usize(id));
}

sema::Symbol *sema::Symbol_Proxy::operator->()
{
    return store->symbols.at_index_ptr(usize(id));
}
