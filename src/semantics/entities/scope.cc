#include "semantics/entities/scope.h"

sema::Scope sema::scope_init(Scope_Kind kind, Scope::Parent parent,
                             Mini_Allocator allocator)
{
    Scope scope{.symbols = Scope::Map(allocator)};
    scope.kind   = kind;
    scope.parent = parent;
    return scope;
}

std::optional<sema::Symbol_Id> sema::scope_get_symbol(Scope *scope,
                                                      mini::StringView name)
{
    if (auto *ptr = scope->symbols.find(name.base()); ptr != nullptr) {
        return *ptr;
    }
    return std::nullopt;
}

const std::optional<sema::Symbol_Id>
sema::scope_get_symbol(const Scope *scope, mini::StringView name)
{
    if (const auto *ptr = scope->symbols.find(name.base()); ptr != nullptr) {
        return *ptr;
    }
    return std::nullopt;
}

bool sema::scope_has_symbol(const Scope *scope, Mini_StringView name)
{
    return scope->symbols.find(name) != nullptr;
}

void sema::scope_put(Scope *scope, Mini_StringView name, Symbol_Id id)
{
    scope->symbols.insert(name, id);
}
