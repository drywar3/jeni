#include "semantics/impl.h"

bool sema::symbol_is_defined(SemanticContext *sema, ScopeId scope_id, Mini_StringView name) {
    Scope &scope = sema->scopes.at_index(scope_id.id);
    return scope_has_symbol(&scope, name);
}

ScopeId sema::current_scope(SemanticContext *sema) { return sema->current_scope; }

SymbolId sema::register_symbol_in(SemanticContext *sema,
                                  ScopeId scope_id,
                                  Mini_StringView name,
                                  Locus locus,
                                  Symbol symbol) {
    Scope &scope = sema->scopes.at_index(scope_id.id);
    SymbolId symbol_id{sema->symbols.insert(locus, symbol)};
    scope_put(&scope, name, symbol_id);
    return symbol_id;
}
