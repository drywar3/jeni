#pragma once

#include "diagnostic.h"
#include "semantics/sema.h"
#include "semantics/entities/type.h"
#include "semantics/entities/scope.h"
#include "semantics/entities/symbol.h"
#include "semantics/entities/symbol_proxy.h"

namespace sema
{
    ScopeId current_scope(SemanticContext *sema);

    ScopeId enter_scope(SemanticContext *sema, ScopeKind kind);
    ScopeId leave_scope(SemanticContext *sema);

    bool symbol_is_defined(SemanticContext *sema, ScopeId scope,
                           mini::StringView name);

    SymbolProxy get_symbol_by_id(const SemanticContext *sema, sema::SymbolId symbol_id);
    SymbolProxy get_symbol_by_id(SemanticContext *sema, sema::SymbolId symbol_id);

    Opt<SymbolProxy> find_symbol_in(SemanticContext *sema, ScopeId scope,
                           mini::StringView name);
    Opt<SymbolProxy> find_symbol_in(const SemanticContext *sema, ScopeId scope,
                                 mini::StringView name);

    Opt<SymbolProxy> eagerly_find_symbol_in(SemanticContext *sema, ScopeId scope,
                                   mini::StringView name);
    Opt<SymbolProxy> eagerly_find_symbol_in(const SemanticContext *sema,
                                         ScopeId scope, mini::StringView name);

    SymbolId register_symbol_in(SemanticContext *sema, ScopeId scope_id,
                                mini::StringView name, Locus locus,
                                Symbol symbol);

    Opt<sema::SymbolProxy> lookup_symbol(SemanticContext *sema, ScopeId scope_id, mini::StringView name);
    Opt<sema::SymbolProxy> eagerly_lookup_symbol(SemanticContext *sema, ScopeId scope_id, mini::StringView name);

    std::optional<SymbolId> get_id_of_symbol(SemanticContext *sema, ScopeId scope_id, mini::StringView name);
    std::optional<SymbolId> eagerly_get_id_of_symbol(SemanticContext *sema, ScopeId scope_id, mini::StringView name);

    void link_locus_to_type(SemanticContext *sema, Locus locus, TypeId id);
    TypeId get_type_at_locus(SemanticContext *sema, Locus locus);

    void link_locus_to_symbol(SemanticContext *sema, Locus locus, SymbolId id);
    SymbolId get_symbol_at_locus(SemanticContext *sema, Locus locus);


    void link_locus_to_scope(SemanticContext *sema, Locus locus, ScopeId id);
    ScopeId find_scope_by_locus(SemanticContext *sema, Locus locus);

    void report(SemanticContext *sema, Diagnostic diagnostic);
} // namespace sema
