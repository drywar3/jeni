#pragma once

#include "diagnostic.h"
#include "semantics/entities/scope.h"
#include "semantics/entities/symbol.h"
#include "semantics/entities/symbol_proxy.h"
#include "semantics/entities/type.h"
#include "semantics/sema.h"
#include "semantics/type/ids.h"

namespace sema
{
    sema::Scope_Id current_scope(Semantic_Context *sema);

    sema::Scope_Id enter_scope(Semantic_Context *sema, sema::Scope_Kind kind);
    sema::Scope_Id enter_scope(Semantic_Context *sema,
                               sema::Scope_Function function_scope);

    template <typename Fn>
    Opt<sema::Scope_Id> find_first_scope_of(Semantic_Context *sema,
                                            sema::Scope_Kind kind, Fn fn)
    {
        Opt<sema::Scope_Id> current = sema->current_scope;
        while (current.has_value()) {
            sema::Scope &scope = sema->scopes().at_index(usize(*current));
            if (scope.kind == kind) {
                fn(sema, scope);
                return current;
            }
            current = scope.parent;
        }
        return std::nullopt;
    }

    sema::Scope_Id leave_scope(Semantic_Context *sema);

    bool symbol_is_defined(Semantic_Context *sema, Scope_Id scope,
                           mini::StringView name);

    Symbol_Proxy get_symbol_by_id(const Semantic_Context *sema,
                                  sema::Symbol_Id symbol_id);
    Symbol_Proxy get_symbol_by_id(Semantic_Context *sema,
                                  sema::Symbol_Id symbol_id);

    Opt<Symbol_Proxy> find_symbol_in(Semantic_Context *sema, Scope_Id scope,
                                     mini::StringView name);
    Opt<Symbol_Proxy> find_symbol_in(const Semantic_Context *sema,
                                     Scope_Id scope, mini::StringView name);

    Opt<Symbol_Proxy> eagerly_find_symbol_in(Semantic_Context *sema,
                                             Scope_Id scope,
                                             mini::StringView name);
    Opt<Symbol_Proxy> eagerly_find_symbol_in(const Semantic_Context *sema,
                                             Scope_Id scope,
                                             mini::StringView name);

    Symbol_Id register_symbol_in(Semantic_Context *sema, Scope_Id scope_Id,
                                 mini::StringView name, Locus locus,
                                 Symbol symbol);

    Opt<sema::Symbol_Proxy> lookup_symbol(Semantic_Context *sema,
                                          Scope_Id scope_Id,
                                          mini::StringView name);
    Opt<sema::Symbol_Proxy> eagerly_lookup_symbol(Semantic_Context *sema,
                                                  Scope_Id scope_Id,
                                                  mini::StringView name);

    std::optional<Symbol_Id> get_id_of_symbol(Semantic_Context *sema,
                                              Scope_Id scope_Id,
                                              mini::StringView name);
    std::optional<Symbol_Id> eagerly_get_id_of_symbol(Semantic_Context *sema,
                                                      Scope_Id scope_Id,
                                                      mini::StringView name);

    void link_locus_to_type(Semantic_Context *sema, Locus locus, Type_Id id);
    Type_Id get_type_at_locus(Semantic_Context *sema, Locus locus);

    void link_locus_to_symbol(Semantic_Context *sema, Locus locus,
                              Symbol_Id id);
    Opt<sema::Symbol_Proxy> get_symbol_at_locus(Semantic_Context *sema,
                                                Locus locus);

    void link_locus_to_scope(Semantic_Context *sema, Locus locus, Scope_Id id);
    Scope_Id find_scope_by_locus(Semantic_Context *sema, Locus locus);

    void report(Semantic_Context *sema, Diagnostic diagnostic);

    template<typename Something_With_Locus>
    static inline
    void report_error(Semantic_Context *sema, const Something_With_Locus &at,
                      const char *message, const char *tag = "here")
    {
        sema::report(sema, diag_create(Severity::Error, at.locus,
                                       message, tag));
    }

    template<>
    void report_error<Locus>(Semantic_Context *sema, const Locus &at,
                             const char *message, const char *tag)
    {
        sema::report(sema, diag_create(Severity::Error, at,
                                       message, tag));
    }

    Mini_String display_type(Semantic_Context *sema, sema::Type_Id type_id);
} // namespace sema
