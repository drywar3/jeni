#pragma once

#include "misc/densemap.h"
#include "misc/id.h"
#include "parser/locus.h"
#include "semantics/entities/symbol.h"

#include <mini.c/string_view.h>
#include <optional>

namespace sema
{
    constexpr auto INVALID_SCOPE = (Scope_Id)0;

    enum struct Scope_Kind {
        Global,
        Function,
        Block,
        Loop,
        Invalid,
    };

    struct Scope_Function {
        sema::Type_Id return_type;
    };

    struct Scope {
        using Map    = ::Map<Mini_StringView, Symbol_Id>;
        using Parent = std::optional<Scope_Id>;

        Scope_Function function;
        Scope_Kind kind;
        Map symbols;
        Scope::Parent parent;
    };

    using Scope_Storage = DenseMap<Locus, Scope>;

    Scope scope_init(Scope_Kind kind, Scope::Parent parent,
                     Mini_Allocator allocator = mini_default_allocator());
    std::optional<Symbol_Id> scope_get_symbol(Scope *scope,
                                              mini::StringView name);
    const std::optional<Symbol_Id> scope_get_symbol(const Scope *scope,
                                                    mini::StringView name);
    bool scope_has_symbol(const Scope *scope, Mini_StringView name);
    void scope_put(Scope *scope, Mini_StringView name, Symbol_Id id);

} // namespace sema
