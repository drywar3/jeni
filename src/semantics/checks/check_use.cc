#include "parser/parser.h"
#include "semantics/impl.h"
#include "semantics/checks/check_use.h"

#include <mini.cc/fs.h>

bool sema::discover_use_expr(Semantic_Context *sema, Expression *expr, sema::Scope_Id scope_id)
{
    auto *use = (ast::expr::Use *)expr;
    Mini_String path = mini::fs::canonicalize(use->path, sema->allocator);

    auto *store = sema->store;

    if (auto id = sourcemgr_find_source_id_at_path(sema->sources, path);
        id.has_value()) {
        if (sema->source_id == *id) {
            return false;
        }
        auto locus   = *sema->loaded_programs.find(path);
        auto program = *store->programs.find(locus);
        store->programs.insert(expr->locus, program);
        return true;
    }

    sema->loaded_programs.insert(mini::StringView(path), expr->locus);
    SourceId source_id = sourcemgr_open_file(sema->sources,
                                             mini::StringView(path),
                                             sema->allocator);
    const Mini_String &content = sourcemgr_get_content(sema->sources, source_id);
    Parser parser = parser_create(source_id, content, sema->diagnostics, sema->allocator);

    Program program;

    if (store->programs.contains(expr->locus)) {
        program = *store->programs.find(expr->locus);
    } else {
        program = program_init(sema->allocator);
        while (!parser_is_done(&parser)) {
            Statement *statement = parser_parse_statement(&parser);
            if (!statement)
                continue;
            program_add(&program, statement);
        }
        parser_destroy(&parser);
        store->programs.insert(expr->locus, program);
    }


    {
        sema::Semantic_Id sema_id = store->semantic_contexts.allocate(
            semactx_init(
                sema->allocator,
                sema->allocator,
                sema->diagnostics,
                sema->store,
                sema->sources,
                sema->vm, sema::Semantic_Id(0), source_id));
        auto *new_sema = store->semantic_contexts.get_ptr(sema_id);
        new_sema->my_id = sema_id;
        semactx_resolve(new_sema, &program);

        auto &scope = store->scopes.at_index(usize(new_sema->global_scope));
        auto &cur_scope = store->scopes.at_index(usize(sema->global_scope));
        for (auto [name, id, _] : scope.symbols) {
            if (sema::scope_has_symbol(&cur_scope, name)) {

                auto first  = *sema::find_symbol_in(sema, sema->global_scope, name);
                auto second = sema::get_symbol_by_id(sema, id);

                sema::report_error(sema, *expr, "conflicting symbol in import",
                                   mini_string_build(sema->allocator, "symbol: `%.*s`", SVARG(name)),
                                   {Label{"first defined here", first->locus},
                                    Label{"conflict here", second->locus}});
                return false;
            }
            sema::scope_put(&cur_scope, name, id);
        }
    }
    return true;
}
