#include "semantics/impl.h"

bool sema::symbol_is_defined(SemanticContext *sema, ScopeId scope_id,
                             Mini_StringView name)
{
    Scope &scope = sema->store->scopes.at_index((usize)scope_id);
    return scope_has_symbol(&scope, name);
}

sema::ScopeId sema::current_scope(SemanticContext *sema)
{
    return sema->current_scope;
}

sema::SymbolId sema::register_symbol_in(SemanticContext *sema, ScopeId scope_id,
                                        Mini_StringView name, Locus locus,
                                        Symbol symbol)
{
    Scope &scope = sema->store->scopes.at_index((usize)scope_id);
    SymbolId symbol_id{sema->store->symbols.insert(locus, symbol)};
    scope_put(&scope, name, symbol_id);
    return symbol_id;
}

void sema::report(SemanticContext *sema, Diagnostic diagnostic)
{
    diagpool_report_diag(sema->diagnostics, diagnostic);
}

sema::SymbolPointer sema::find_symbol_in(SemanticContext *sema,
                                         ScopeId scope_id, Mini_StringView name)
{
    Scope &scope = sema->store->scopes.at_index((usize)scope_id);
    if (!scope_has_symbol(&scope, name)) {
        return nullptr;
    }

    SymbolId symbol_id = *scope_get_symbol(&scope, name);
    return &sema->store->symbols.at_index((usize)symbol_id);
}

const sema::SymbolPointer find_symbol_in(const SemanticContext *sema,
                                         sema::ScopeId scope,
                                         Mini_StringView name);

void sema::link_locus_to_type(SemanticContext *sema, Locus locus, TypeId id)
{
    sema->store->types.link(locus, (usize)id);
}

sema::TypeId sema::get_type_at_locus(SemanticContext *sema, Locus locus) {
    return (sema::TypeId)*sema->store->types.get_id(locus);
}

sema::ScopeId sema::enter_scope(SemanticContext *sema, ScopeKind kind) {
    Scope scope = scope_init(kind, sema->current_scope, sema->allocator);
    sema->current_scope = (sema::ScopeId)sema->store->scopes.add_value(scope);
    return sema->current_scope;
}

sema::ScopeId sema::leave_scope(SemanticContext *sema) {
    auto scope_id = sema->current_scope;
    auto prev_scope_id = sema->current_scope;
    auto &scope  = sema->store->scopes.at_index((usize)scope_id);
    if (scope.parent.has_value()) {
        scope_id = *scope.parent;
    } else {
        scope_id = sema->global_scope;
    }
    sema->current_scope = scope_id;
    return prev_scope_id;
}

sema::ScopeId sema::find_scope_by_locus(SemanticContext *sema, Locus locus) {
    if (sema->store->scopes.contains(locus)) {
        return (sema::ScopeId)*sema->store->scopes.get_id(locus);
    }
    return sema::INVALID_SCOPE;
}

void sema::link_locus_to_scope(SemanticContext *sema, Locus locus, ScopeId id)
{
    sema->store->scopes.link(locus, (usize)id);
}
