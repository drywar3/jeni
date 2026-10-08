#pragma once

#include "misc/id.h"
#include "semantics/entities/type.h"

#include <mini.cc/string_view.h>
#include <optional>

namespace sema
{
    enum struct Scope_Id : usize {};
    enum struct Symbol_Id : usize {};

    enum struct Symbol_Kind {
        Variable,
        Function,
        Type,
        Namespace,
    };

    enum struct Symbol_State {
        Resolved,
        Resolving,
        Unresolved,
        Failed,
    };

    struct Symbol_Variable {
        bool is_initialized;
        std::optional<sema::Type_Id> type_id;
        Mutability mutability;
    };

    struct Symbol {
        Symbol_Kind kind;
        mini::StringView name;
        Scope_Id scope_Id;
        /* points to where the variable was defined */
        Locus locus;
        Symbol_State resolve_state{Symbol_State::Unresolved};

        union {
            Symbol_Variable variable;
        };

        void set_state(Symbol_State state);
        bool is_state(Symbol_State state) const;
    };

    using Symbol_Storage = DenseMap<Locus, Symbol>;

} // namespace sema
