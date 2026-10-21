#pragma once

#include "ast/stmt.h"
#include "ast/expr.h"
#include "misc/id.h"
#include "compeval/value.h"
#include "hir/types/profile.h"
#include "semantics/entities/type.h"
#include "semantics/entities/symbol_id.h"

#include <mini.cc/string_view.h>
#include <optional>

namespace sema
{
    enum struct Symbol_Kind {
        Variable,
        Parameter,
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

    struct Symbol_Type {
        sema::Type_Id type_id;
    };

    struct Symbol_Function {
        Statement *statement;
    };

    struct Symbol_Namespace {
        sema::Scope_Id       scope_id;
        Opt<sema::Symbol_Id> parent_namespace;
    };

    struct Symbol_Variable {
        compeval::Value              value;
        std::optional<sema::Type_Id> type_id;
        Expression                  *expr;
        Mutability                   mutability;
        bool                         is_initialized;
    };

    struct Symbol {
        Symbol_Kind kind;
        mini::StringView name;
        Scope_Id scope_id;
        /* points to the namespace it was defined in */
        Opt<sema::Symbol_Id> def_ns;
        /* points to where the variable was defined */
        Locus locus;
        hir::Profile profile;
        Symbol_State resolve_state{Symbol_State::Unresolved};

        union {
            Symbol_Variable   variable;
            Symbol_Type       type;
            Symbol_Function   function;
            Symbol_Namespace  ns;
        };

        void set_state(Symbol_State state);
        bool is_state(Symbol_State state) const;

        bool is(Symbol_Kind kind) const;
    };
} // namespace sema
