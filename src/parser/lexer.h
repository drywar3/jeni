#pragma once

#include <mini.c/mini_def.h>
#include <mini.c/string.h>

#include "token.h"
#include "diagnostic.h"

typedef struct {
    SourceId source_id;
    Mini_String content;
    usize line;
    usize col, prev_col;
    usize offset, prev_offset;
} Lexer;

Lexer lexer_create(SourceId id, const Mini_String content);

bool lexer_next_token(Lexer *lexer, Token *token, Diagnostic *diag);

bool lexer_is_done(const Lexer *lexer);
