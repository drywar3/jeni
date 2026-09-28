#pragma once

#include "misc/id.h"
#include "semantics/entities/type.h"

#include <mini.c/string_view.h>
#include <optional>

namespace sema
{
    enum struct ScopeId : usize {};
    enum struct SymbolId : usize {};

    enum struct SymbolKind {
        Variable,
        Function,
        Type,
        Namespace,
    };

    enum struct SymbolState {
        Resolved,
        Resolving,
        Unresolved,
    };

    struct SymbolVariable {
        bool is_initialized;
        std::optional<sema::TypeInfo> type_info;
    };

    struct Symbol {
        SymbolKind kind;
        Mini_StringView name;
        ScopeId scope_id;
        /* points to where the variable was defined */
        Locus locus;
        SymbolState resolve_state;

        union {
            SymbolVariable variable;
        } as;
    };

    typedef Symbol *SymbolPointer;
    using SymbolStorage = DenseMap<Locus, Symbol>;

} // namespace sema
