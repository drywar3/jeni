#include <mini.c/bulk_allocator.h>
#include <mini.c/debug_allocator.h>
#include <mini.c/fs.h>
#include <mini.c/string.h>
#include <mini.cc/scope_guard.h>
#include <stdio.h>

#include "ast/ast.h"
#include "ast/print.h"
#include "codegen/backends/c.h"
#include "codegen/codegen.h"
#include "hir/convert.h"
#include "lir/lir.h"
#include "parser/parser.h"
#include "semantics/sema.h"
#include "source.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("usage: %s <input> [output]\n", argv[0]);
        return 1;
    }

    const char *output_name = "a.out";
    if (argc > 2) {
        output_name = argv[2];
    }

    Mini_BulkAllocator bka   = mini_bka_create(mini_default_allocator());
    Mini_Allocator allocator = mini_bka_allocator(&bka);

    const char *input_path = argv[1];

    SourceManager sources = sourcemgr_init();
    SourceId root_file = sourcemgr_open_file(&sources, input_path, allocator);

    DiagnosticPool diagnostics = diagpool_create();

    auto scope_guard = mini::ScopeGuard([&]() {
        sourcemgr_destroy(&sources);
        diagpool_destroy(&diagnostics);
        mini_bka_destroy(&bka);
    });

    const Mini_String &content = sourcemgr_get_content(&sources, root_file);
    Parser parser = parser_create(root_file, content, &diagnostics);
    parser_set_allocator(&parser, allocator);

    Program program = program_init(allocator);
    while (!parser_is_done(&parser)) {
        Statement *statement = parser_parse_statement(&parser);
        if (!statement)
            continue;
        program_add(&program, statement);
    }
    parser_destroy(&parser);

    if (!diagpool_is_empty(&diagnostics)) {
        for (usize n = 0; n < mini_array_count(diagnostics.diagnostics); n++) {
            const Diagnostic *diagnostic = &diagnostics.diagnostics[n];
            diag_report(diagnostic, &sources);
        }
        return 1;
    }

    SemanticStorage storage = semastore_init(allocator);
    semastore_init_builtin_types(&storage);

    SemanticContext sema = semactx_init(allocator, &diagnostics, &storage);
    semactx_resolve(&sema, &program);

    if (!diagpool_is_empty(&diagnostics)) {
        for (usize n = 0; n < mini_array_count(diagnostics.diagnostics); n++) {
            const Diagnostic *diagnostic = &diagnostics.diagnostics[n];
            diag_report(diagnostic, &sources);
        }
        return 1;
    }

    hir::Context hir         = hir::ctx_init(allocator, &storage);
    hir::Program hir_program = hir::program_from_raw(&hir, program);

    lir::Context context = lir::ctx_init(allocator, &storage);
    lir::Module mod      = lir::module_init(&context);
    lir::inflate_module(&mod, hir_program);

    codegen::Context cg_context = codegen::ctx_init(
        &mod, allocator,
        {codegen::c_backend_entry_point, codegen::c_backend_finalize});
    codegen::ctx_generate(&cg_context, output_name);

    return 0;
}
