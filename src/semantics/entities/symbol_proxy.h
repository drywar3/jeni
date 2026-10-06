#pragma once

#include "semantics/sema.h"
#include "semantics/entities/symbol.h"

namespace sema
{
    struct SymbolProxy {
        sema::SymbolId id;
        SemanticStorage *store;

        explicit SymbolProxy(SemanticStorage *store, sema::SymbolId id);

        sema::SymbolId get_id() const { return id; }

        const sema::Symbol *operator->() const;
        sema::Symbol *operator->();
    };
} // namespace sema
