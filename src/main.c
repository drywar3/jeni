#include <mini.c/string.h>
#include <mini.c/fs.h>
#include <mini.c/bulk_allocator.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "ast/print.h"

int main() {
    const char *input_path = "test.jeni";
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

    while (!parser_is_done(&parser)) {
        Statement *statement = parser_parse_statement(&parser);
        if (!statement)
            continue;
        ast_print(statement, 0);
    }

    mini_bka_destroy(&bka);
    return 0;
}
