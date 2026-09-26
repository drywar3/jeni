#pragma once

#include "diagnostic.h"
#include "lexer.h"
#include "token.h"
#include "ast/stmt.h"
#include "token_buffer.h"

#include <mini.c/allocator.h>

typedef struct {
    TokenBuffer     tokens;
    DiagnosticPool *diagnostics;
    Token current, previous;

    Mini_Allocator allocator;
} Parser;

Parser parser_create(SourceId id, const Mini_String content, DiagnosticPool *diagnostics);
void parser_destroy(Parser *parser);

void parser_set_allocator(Parser *parser, Mini_Allocator allocator);
bool parser_is_done(const Parser *parser);

Statement *parser_parse_statement(Parser *parser);
