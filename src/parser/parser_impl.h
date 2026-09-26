#pragma once

#include "token.h"
#include "parser.h"
#include "ast/misc.h"

#define INLINE static inline

INLINE Token current(const Parser *parser) { return parser->current; }

INLINE Token previous(const Parser *parser) { return parser->previous; }

INLINE Token next(Parser *parser) {
    parser->previous = parser->current;
    parser->current  = tokenbuffer_advance(&parser->tokens);
    return parser->previous;
}

INLINE bool equals(const Parser *parser, TokenKind kind) {
    return current(parser).kind == kind;
}

#define equals_sequence(p, ...)                              \
    equals_sequence_impl(p,                                             \
                         sizeof((TokenKind[]){__VA_ARGS__}) /           \
                         sizeof(((TokenKind[]){__VA_ARGS__})[0]),       \
                         (TokenKind[]){__VA_ARGS__})

INLINE bool equals_sequence_impl(Parser *parser, int count, TokenKind *kinds) {
    for (usize n = 0; n < count; n++) {
        if (tokenbuffer_peek(&parser->tokens, n).kind != kinds[n])
            return false;
    }
    return true;
}

#define eat_sequence(p, ...)                                            \
    eat_sequence_impl(p,                                                \
                      sizeof((TokenKind[]){__VA_ARGS__}) /              \
                      sizeof(((TokenKind[]){__VA_ARGS__})[0]),          \
                      (TokenKind[]){__VA_ARGS__})

INLINE bool eat_sequence_impl(Parser *parser, int count, TokenKind *kinds) {
    for (usize n = 0; n < count; n++) {
        if (tokenbuffer_peek(&parser->tokens, n).kind != kinds[n])
            return false;
        next(parser);
    }
    return true;
}

INLINE bool expect(Parser *parser, TokenKind kind) {
    if (!equals(parser, kind)) {
        diagpool_report(parser->diagnostics, DIAG_Error, current(parser).locus,
                        "invalid token", mini_string_build(parser->allocator,
                                                           "expected `%s` got `%s` instead",
                                                           tokenkind_to_string(kind),
                                                           tokenkind_to_string(current(parser).kind)));
        return false;
    }
    next(parser);
    return true;
}

INLINE bool try_expect(Parser *parser, TokenKind kind) {
    if (!equals(parser, kind)) {
        return false;
    }
    next(parser);
    return true;
}

bool eat_name(Parser *parser, Name *name);
