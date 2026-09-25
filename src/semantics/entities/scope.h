#pragma once

#include "misc/id.h"
#include "parser/locus.h"
#include "misc/densemap.h"
#include "semantics/entities/symbol.h"

#include <mini.c/string_view.h>
#include <optional>

enum struct ScopeKind {
    Global,
    Function,
    Block,
};

typedef struct ScopeFunction {
} ScopeFunction;

typedef struct Scope {
    using Map = ::Map<Mini_StringView, SymbolId>;
    using Parent = std::optional<ScopeId>;

    ScopeFunction function;
    ScopeKind     kind;
    Map           symbols;
    Scope::Parent parent;
} Scope;

using ScopeStorage = DenseMap<Locus, Scope>;

Scope scope_init(ScopeKind kind, Scope::Parent parent);
std::optional<SymbolId> scope_get_symbol(Scope *scope, Mini_StringView name);
bool scope_has_symbol(const Scope *scope, Mini_StringView name);
void scope_put(Scope *scope, Mini_StringView name, SymbolId id);
