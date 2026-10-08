#include "semantics/impl.h"

bool sema::symbol_is_defined(Semantic_Context *sema, Scope_Id scope_Id,
                             mini::StringView name)
{
    Scope &scope = sema->store->scopes.at_index((usize)scope_Id);
    return scope_has_symbol(&scope, name.base());
}

sema::Scope_Id sema::current_scope(Semantic_Context *sema)
{
    return sema->current_scope;
}

sema::Symbol_Id sema::register_symbol_in(Semantic_Context *sema,
                                         Scope_Id scope_Id,
                                         mini::StringView name, Locus locus,
                                         Symbol symbol)
{
    Scope &scope = sema->store->scopes.at_index((usize)scope_Id);
    MINI_ASSERT(!sema::scope_has_symbol(&scope, name.base()),
                "scope already contains symbol: %.*s", SVARG(name.base()));
    Symbol_Id symbol_id{sema->store->symbols.insert(locus, symbol)};
    scope_put(&scope, name.base(), symbol_id);
    return symbol_id;
}

void sema::report(Semantic_Context *sema, Diagnostic diagnostic)
{
    diagpool_report_diag(sema->diagnostics, diagnostic);
}

Opt<sema::Symbol_Proxy> sema::find_symbol_in(Semantic_Context *sema,
                                             Scope_Id scope_Id,
                                             mini::StringView name)
{
    Scope &scope = sema->store->scopes.at_index((usize)scope_Id);
    if (!scope_has_symbol(&scope, name.base())) {
        return std::nullopt;
    }

    Symbol_Id symbol_id = *scope_get_symbol(&scope, name.base());
    return sema::Symbol_Proxy(sema->store, symbol_id);
}

const sema::Symbol *find_symbol_in(const Semantic_Context *sema,
                                   sema::Scope_Id scope, Mini_StringView name);

void sema::link_locus_to_type(Semantic_Context *sema, Locus locus, Type_Id id)
{
    sema->store->types.link(locus, (usize)id);
}

sema::Type_Id sema::get_type_at_locus(Semantic_Context *sema, Locus locus)
{
    return (sema::Type_Id)*sema->store->types.get_id(locus);
}

sema::Scope_Id sema::enter_scope(Semantic_Context *sema, Scope_Kind kind)
{
    Scope scope = scope_init(kind, sema->current_scope, sema->allocator);
    sema->current_scope = (sema::Scope_Id)sema->store->scopes.add_value(scope);
    return sema->current_scope;
}

sema::Scope_Id sema::enter_scope(Semantic_Context *sema,
                                 sema::Scope_Function function_scope)
{
    Scope scope    = scope_init(sema::Scope_Kind::Function, sema->current_scope,
                                sema->allocator);
    scope.function = function_scope;
    sema->current_scope = (sema::Scope_Id)sema->store->scopes.add_value(scope);
    return sema->current_scope;
}

sema::Scope_Id sema::leave_scope(Semantic_Context *sema)
{
    auto scope_Id      = sema->current_scope;
    auto prev_scope_Id = sema->current_scope;
    auto &scope        = sema->store->scopes.at_index((usize)scope_Id);
    if (scope.parent.has_value()) {
        scope_Id = *scope.parent;
    } else {
        scope_Id = sema->global_scope;
    }
    sema->current_scope = scope_Id;
    return prev_scope_Id;
}

sema::Scope_Id sema::find_scope_by_locus(Semantic_Context *sema, Locus locus)
{
    if (sema->store->scopes.contains(locus)) {
        return (sema::Scope_Id)*sema->store->scopes.get_id(locus);
    }
    return sema::INVALID_SCOPE;
}

void sema::link_locus_to_scope(Semantic_Context *sema, Locus locus, Scope_Id id)
{
    sema->store->scopes.link(locus, (usize)id);
}

Opt<sema::Symbol_Proxy> sema::eagerly_find_symbol_in(Semantic_Context *sema,
                                                     Scope_Id start,
                                                     mini::StringView name)
{
    std::optional<Scope_Id> current = start;
    while (current.has_value()) {
        sema::Scope &scope = sema->scopes().at_index(usize(*current));
        if (scope_has_symbol(&scope, name.base())) {
            const auto symbol_id = scope_get_symbol(&scope, name.base());
            return sema::Symbol_Proxy(sema->store, *symbol_id);
        }
        current = scope.parent;
    }
    return std::nullopt;
}

Opt<sema::Symbol_Proxy>
sema::eagerly_find_symbol_in(const Semantic_Context *sema, Scope_Id start,
                             mini::StringView name)
{
    std::optional<Scope_Id> current = start;
    while (current.has_value()) {
        const sema::Scope &scope = sema->scopes().at_index((usize)*current);
        if (scope_has_symbol(&scope, name.base())) {
            const auto symbol_id = scope_get_symbol(&scope, name.base());
            return sema::Symbol_Proxy(sema->store, *symbol_id);
        }
        current = scope.parent;
    }
    return std::nullopt;
}

std::optional<sema::Symbol_Id> sema::get_id_of_symbol(Semantic_Context *sema,
                                                      Scope_Id scope_Id,
                                                      mini::StringView name)
{
    const sema::Scope &scope = sema->scopes().at_index(usize(scope_Id));
    if (scope_has_symbol(&scope, name.base())) {
        const auto symbol_id = scope_get_symbol(&scope, name.base());
        return symbol_id;
    }
    return std::nullopt;
}

std::optional<sema::Symbol_Id>
sema::eagerly_get_id_of_symbol(Semantic_Context *sema, Scope_Id start,
                               mini::StringView name)
{
    std::optional<Scope_Id> current = start;
    while (current.has_value()) {
        const sema::Scope &scope = sema->scopes().at_index((usize)*current);
        if (scope_has_symbol(&scope, name.base())) {
            const auto symbol_id = scope_get_symbol(&scope, name.base());
            return symbol_id;
        }
        current = scope.parent;
    }
    return std::nullopt;
}

Opt<sema::Symbol_Proxy> sema::get_symbol_at_locus(Semantic_Context *sema,
                                                  Locus locus)
{
    auto symbol_id = sema->store->symbols.get_id(locus);
    if (!symbol_id)
        return std::nullopt;
    return sema::Symbol_Proxy(sema->store, sema::Symbol_Id(*symbol_id));
}

void sema::link_locus_to_symbol(Semantic_Context *sema, Locus locus,
                                Symbol_Id id)
{
    sema->store->symbols.link(locus, (usize)id);
}

Opt<sema::Symbol_Proxy> sema::lookup_symbol(Semantic_Context *sema,
                                            Scope_Id scope_Id,
                                            mini::StringView name)
{
    sema::Scope &scope = sema->scopes().at_index(usize(scope_Id));
    if (!sema::scope_has_symbol(&scope, name.base()))
        return std::nullopt;

    sema::Symbol_Id id = *sema::scope_get_symbol(&scope, name);
    return sema::Symbol_Proxy(sema->store, id);
}

Opt<sema::Symbol_Proxy> sema::eagerly_lookup_symbol(Semantic_Context *sema,
                                                    Scope_Id scope_Id,
                                                    mini::StringView name)
{
    Opt<sema::Scope_Id> current = scope_Id;
    while (current.has_value()) {
        if (auto sym_opt = sema::lookup_symbol(sema, *current, name);
            sym_opt.has_value())
            return sym_opt;
        Scope &scope = sema->scopes().at_index(usize(*current));
        current      = scope.parent;
    }
    return std::nullopt;
}

sema::Symbol_Proxy sema::get_symbol_by_id(const Semantic_Context *sema,
                                          sema::Symbol_Id symbol_id)
{
    return sema::Symbol_Proxy(sema->store, symbol_id);
}

sema::Symbol_Proxy sema::get_symbol_by_id(Semantic_Context *sema,
                                          sema::Symbol_Id symbol_id)
{
    return sema::Symbol_Proxy(sema->store, symbol_id);
}

Mini_String sema::display_type(Semantic_Context *sema, sema::Type_Id type_id)
{
    const auto &type = sema->types().at_index(usize(type_id));
    return type.display(sema->allocator, sema->types());
}
