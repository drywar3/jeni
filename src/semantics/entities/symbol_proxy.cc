#include "semantics/entities/symbol_proxy.h"

sema::SymbolProxy::SymbolProxy(SemanticStorage *store, sema::SymbolId id)
    : id(id), store(store)
{
}

const sema::Symbol *sema::SymbolProxy::operator->() const
{
    return store->symbols.at_index_ptr(usize(id));
}

sema::Symbol *sema::SymbolProxy::operator->()
{
    return store->symbols.at_index_ptr(usize(id));
}
