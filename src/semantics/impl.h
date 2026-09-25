#pragma once

#include "semantics/sema.h"
#include "semantics/entities/scope.h"
#include "semantics/entities/symbol.h"

namespace sema
{
    ScopeId current_scope(SemanticContext *sema);

    void enter_scope(SemanticContext *sema, ScopeKind kind);
    void leave_scope(SemanticContext *sema);

    bool symbol_is_defined(SemanticContext *sema, ScopeId scope, Mini_StringView name);

    SymbolPointer find_symbol_in(SemanticContext *sema, ScopeId scope, Mini_StringView name);
    const SymbolPointer find_symbol_in(const SemanticContext *sema, ScopeId scope, Mini_StringView name);

    SymbolPointer eagerly_find_symbol_in(SemanticContext *sema, ScopeId scope, Mini_StringView name);
    const SymbolPointer eagerly_find_symbol_in(const SemanticContext *sema, ScopeId scope, Mini_StringView name);

    SymbolId register_symbol_in(SemanticContext *sema, ScopeId scope_id, Mini_StringView name, Locus locus, Symbol symbol);
}
