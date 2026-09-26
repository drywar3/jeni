#pragma once

#include <mini.c/array.h>
#include "token.h"
#include "lexer.h"

typedef struct TokenBuffer {
    Lexer lexer;
    MINI_ARRAY(Token) tokens;
    DiagnosticPool *diagnostics;
    int cursor;
} TokenBuffer;

TokenBuffer tokenbuffer_create(SourceId id, const Mini_String content, DiagnosticPool *diagnostics);
void tokenbuffer_destroy(TokenBuffer *buffer);

void tokenbuffer_prepare(TokenBuffer *buffer, int window);

Token tokenbuffer_peek(TokenBuffer *buffer, int ahead);
Token tokenbuffer_advance(TokenBuffer *buffer);

bool tokenbuffer_is_truly_done(const TokenBuffer *buffer);
