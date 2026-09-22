#include "token_buffer.h"

TokenBuffer tokenbuffer_create(const char *path,
                               const Mini_String content,
                               DiagnosticPool *diagnostics) {
    TokenBuffer buffer = {0};
    buffer.lexer       = lexer_create(path, content);
    buffer.tokens      = MINI_ARRAY_INIT(mini_default_allocator(), Token);
    buffer.diagnostics = diagnostics;
    buffer.cursor      = 0;
    return buffer;
}

void tokenbuffer_destroy(TokenBuffer *buffer) { MINI_UNREACHABLE("TODO"); }

void tokenbuffer_prepare(TokenBuffer *buffer, int window) {
    if (buffer->cursor + window >= mini_array_count(buffer->tokens)) {
        usize needed = (buffer->cursor + window) * 2;
        for (usize n = 0; n < needed; n++) {
            Token token;
            Diagnostic diagnostic;
            if (!lexer_next_token(&buffer->lexer, &token, &diagnostic)) {
                diagpool_report_diag(buffer->diagnostics, diagnostic);
                continue;
            }
            mini_array_append(buffer->tokens, token);
        }
    }
}

bool tokenbuffer_is_truly_done(const TokenBuffer *buffer) {
    return lexer_is_done(&buffer->lexer) && buffer->cursor >= mini_array_count(buffer->tokens) - 1;
}

Token tokenbuffer_peek(TokenBuffer *buffer, int ahead) {
    if (tokenbuffer_is_truly_done(buffer)) {
        // eof
        return mini_array_last(buffer->tokens);
    }

    if (buffer->cursor + ahead >= mini_array_count(buffer->tokens)) {
        tokenbuffer_prepare(buffer, ahead);
    }

    return buffer->tokens[buffer->cursor + ahead];
}

Token tokenbuffer_advance(TokenBuffer *buffer) {
    buffer->cursor += 1;
    Token current = tokenbuffer_peek(buffer, 0);
    tokenbuffer_prepare(buffer, 2);
    return current;
}
