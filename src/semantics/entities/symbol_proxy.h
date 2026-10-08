#pragma once

#include "semantics/entities/symbol.h"
#include "semantics/sema.h"

namespace sema
{
    struct Symbol_Proxy {
        sema::Symbol_Id id;
        Semantic_Storage *store;

        explicit Symbol_Proxy(Semantic_Storage *store, sema::Symbol_Id id);

        sema::Symbol_Id get_id() const { return id; }

        const sema::Symbol *operator->() const;
        sema::Symbol *operator->();
    };
} // namespace sema
