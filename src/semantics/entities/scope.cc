#include "semantics/entities/scope.h"

Scope scope_init(ScopeKind kind, Scope::Parent parent, Mini_Allocator allocator) {
    Scope scope{ .symbols = Scope::Map(allocator) };
    scope.kind = kind;
    scope.parent = parent;
    return scope;
}

std::optional<SymbolId> scope_get_symbol(Scope *scope, Mini_StringView name) {
    if (auto *ptr = scope->symbols.find(name); ptr != nullptr) {
        return *ptr;
    }
    return std::nullopt;
}

bool scope_has_symbol(const Scope *scope, Mini_StringView name) {
    return scope->symbols.find(name) != nullptr;
}

void scope_put(Scope *scope, Mini_StringView name, SymbolId id) {
    scope->symbols.insert(name, id);
}
