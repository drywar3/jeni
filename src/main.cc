#include <stdio.h>
#include <mini.c/fs.h>
#include <mini.cc/dtor.h>
#include <mini.c/string.h>
#include <mini.c/bulk_allocator.h>
#include <mini.c/debug_allocator.h>


#include "source.h"
#include "ast/ast.h"
#include "ast/print.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "semantics/sema.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s <input>\n", argv[0]);
        return 1;
    }


    Mini_BulkAllocator bka = mini_bka_create(mini_default_allocator());
    Mini_Allocator allocator = mini_bka_allocator(&bka);
    auto _bka = mini::AttachDtor(bka, mini_bka_destroy);

    const char *input_path = argv[1];

    SourceManager sources = sourcemgr_init();
    auto _sources         = mini::AttachDtor(sources, sourcemgr_destroy);
    SourceId root_file    = sourcemgr_open_file(&sources, input_path, allocator);

    DiagnosticPool diagnostics = diagpool_create();
    auto diagnostics_          = mini::AttachDtor(diagnostics, diagpool_destroy);

    const Mini_String &content = sourcemgr_get_content(&sources, root_file);
    Parser parser              = parser_create(root_file, content, &diagnostics);
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

    SemanticContext sema = semactx_init(allocator, &diagnostics);
    semactx_resolve(&sema, &program);

    return 0;
}
