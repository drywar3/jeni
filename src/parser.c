#include "parser.h"
#include "parser_impl.h"
#include "parsers/parse_statements.h"

Parser parser_create(const char *path,
                     const Mini_String content,
                     DiagnosticPool *diagnostics) {
    Parser parser = {0};
    parser.tokens      = tokenbuffer_create(path, content, diagnostics);
    parser.diagnostics = diagnostics;
    parser.allocator   = mini_default_allocator();
    tokenbuffer_prepare(&parser.tokens, 10);
    parser.current  = tokenbuffer_peek(&parser.tokens, 0);
    //next(&parser);
    //printf("[%s]\n", tokenkind_to_string(parser.current.kind));
    return parser;
}

void parser_set_allocator(Parser *parser, Mini_Allocator allocator) {
    parser->allocator = allocator;
}

bool parser_is_done(const Parser *parser) {
    return tokenbuffer_is_truly_done(&parser->tokens);
}

Statement stmt;

Statement *parser_parse_statement(Parser *parser) {
    /* check for a variable declaration */
    if (equals_sequence(parser, TOKEN_Identifier, TOKEN_SEP_Colon)) {
        return parse_variable_declaration(parser);
    }
    return NULL;
}
