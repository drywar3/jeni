#pragma once

#include "lexer.h"
#include "token.h"
#include <mini.c/array.h>

typedef struct TokenBuffer {
    Lexer lexer;
    MINI_ARRAY(Token) tokens;
    Diagnostic_Pool *diagnostics;
    int cursor;
} TokenBuffer;

TokenBuffer tokenbuffer_create(SourceId id, const Mini_String content,
                               Diagnostic_Pool *diagnostics);
void tokenbuffer_destroy(TokenBuffer *buffer);

void tokenbuffer_prepare(TokenBuffer *buffer, int window);

Token tokenbuffer_peek(TokenBuffer *buffer, int ahead);
Token tokenbuffer_advance(TokenBuffer *buffer);

bool tokenbuffer_is_truly_done(const TokenBuffer *buffer);
