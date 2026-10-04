#pragma once

#include "token.h"
#include "parser.h"
#include "ast/misc.h"

#define INLINE static inline

INLINE Token current(const Parser *parser) { return parser->current; }

INLINE Token previous(const Parser *parser) { return parser->previous; }

INLINE Token next(Parser *parser)
{
    parser->previous = parser->current;
    parser->current  = tokenbuffer_advance(&parser->tokens);
    return parser->previous;
}

INLINE bool equals(const Parser *parser, TokenKind kind)
{
    return current(parser).kind == kind;
}

#define equals_sequence(p, ...)                                                \
    equals_sequence_impl(p,                                                    \
                         sizeof((TokenKind[]){__VA_ARGS__}) /                  \
                             sizeof(((TokenKind[]){__VA_ARGS__})[0]),          \
                         (TokenKind[]){__VA_ARGS__})

INLINE bool equals_sequence_impl(Parser *parser, int count, TokenKind *kinds)
{
    for (usize n = 0; n < count; n++) {
        if (tokenbuffer_peek(&parser->tokens, n).kind != kinds[n])
            return false;
    }
    return true;
}

#define eat_sequence(p, ...)                                                   \
    eat_sequence_impl(p,                                                       \
                      sizeof((TokenKind[]){__VA_ARGS__}) /                     \
                          sizeof(((TokenKind[]){__VA_ARGS__})[0]),             \
                      (TokenKind[]){__VA_ARGS__})

INLINE bool eat_sequence_impl(Parser *parser, int count, TokenKind *kinds)
{
    for (usize n = 0; n < count; n++) {
        if (tokenbuffer_peek(&parser->tokens, n).kind != kinds[n])
            return false;
        next(parser);
    }
    return true;
}


#define skip_until_one_of(p, skip_past, ...)                            \
    skip_until_one_of_impl(p, skip_past,                                \
                           sizeof((TokenKind[]){__VA_ARGS__}) /         \
                           sizeof(((TokenKind[]){__VA_ARGS__})[0]),     \
                           (TokenKind[]){__VA_ARGS__})

INLINE bool skip_until_one_of_impl(Parser *p, bool skip_past, int count, TokenKind *kinds)
{
    while (!parser_is_done(p)) {
        for (usize n = 0; n < count; ++n) {
            if (equals(p, kinds[n])) {
                if (skip_past) {
                    next(p);
                    return true;
                }
            }
        }
        next(p);
    }
    return false;
}


INLINE bool expect(Parser *parser, TokenKind kind)
{
    if (!equals(parser, kind)) {
        diagpool_report(parser->diagnostics, Severity::Error, current(parser).locus,
                        "invalid token",
                        mini_string_build(
                            parser->allocator, "expected `%s` got `%s` instead",
                            tokenkind_to_string(kind),
                            tokenkind_to_string(current(parser).kind)));
        return false;
    }
    next(parser);
    return true;
}

INLINE bool try_expect(Parser *parser, TokenKind kind)
{
    if (!equals(parser, kind)) {
        return false;
    }
    next(parser);
    return true;
}

bool eat_name(Parser *parser, Name *name);

INLINE void parser_report(Parser *p, Diagnostic diag)
{
    diagpool_report_diag(p->diagnostics, diag);
}
