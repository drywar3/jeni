#include <mini.c/string.h>
#include <mini.c/fs.h>
#include <mini.c/bulk_allocator.h>
#include <stdio.h>

#include "parser/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "ast/print.h"
#include "semantics/sema.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s <input>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    Mini_String content    = mini_string_default_init();

    if (!mini_fs_read_into(input_path, &content)) {
        printf("error: could not read file: %s\n", input_path);
        return 1;
    }

    Mini_BulkAllocator bka = mini_bka_create(mini_default_allocator());
    Mini_Allocator allocator = mini_bka_allocator(&bka);

    DiagnosticPool diagnostics = diagpool_create();
    Parser parser              = parser_create(input_path, content, &diagnostics);
    parser_set_allocator(&parser, allocator);

    Program program = program_init(allocator);
    while (!parser_is_done(&parser)) {
        Statement *statement = parser_parse_statement(&parser);
        if (!statement)
            continue;
        program_add(&program, statement);
    }

    SemanticContext sema = semactx_init(allocator, &diagnostics);
    semactx_resolve(&sema, &program);

    mini_bka_destroy(&bka);
    return 0;
}
