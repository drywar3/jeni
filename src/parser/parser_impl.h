#pragma once

#include "token.h"
#include "parser.h"
#include "ast/misc.h"

#define INLINE static inline

namespace parser
{
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

#define equals_sequence(p, ...)                                     \
    parser::equals_sequence_impl(p,                                 \
                                 sizeof((TokenKind[]){__VA_ARGS__}) /   \
                                 sizeof(((TokenKind[]){__VA_ARGS__})[0]), \
                                 (TokenKind[]){__VA_ARGS__})

    INLINE bool equals_sequence_impl(Parser *parser, int count, TokenKind *kinds)
    {
        for (usize n = 0; n < (usize)count; n++) {
            if (tokenbuffer_peek(&parser->tokens, n).kind != kinds[n])
                return false;
        }
        return true;
    }

#define eat_sequence(p, ...)                                    \
    parser::eat_sequence_impl(p,                                \
                              sizeof((TokenKind[]){__VA_ARGS__}) /  \
                              sizeof(((TokenKind[]){__VA_ARGS__})[0]),  \
                              (TokenKind[]){__VA_ARGS__})

    INLINE bool eat_sequence_impl(Parser *parser, int count, TokenKind *kinds)
    {
        if (!equals_sequence_impl(parser, count, kinds)) return false;
        for (usize n = 0; n < (usize)count; n++) next(parser);
        return true;
    }

    using Token_Pack = std::initializer_list<TokenKind>;

    INLINE bool skip_until_one_of(Parser *p, bool skip_past, Token_Pack kinds)
    {
        while (!parser_is_done(p)) {
            for (const auto kind : kinds) {
                if (equals(p, kind)) {
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

    INLINE void report(Parser *p, Diagnostic diag)
    {
        diagpool_report_diag(p->diagnostics, diag);
    }

    template<typename Something_With_Locus>
    INLINE void report_error(Parser *p,
                             const Something_With_Locus &at,
                             const char *message,
                             const char *tag = "here")
    {
        report(p, diag_create(Severity::Error, at.locus, message, tag));
    }

    template<>
    void report_error<Locus>(Parser *p,
                             const Locus &at,
                             const char *message,
                             const char *tag)
    {
        report(p, diag_create(Severity::Error, at, message, tag));
    }

    INLINE void push_state(Parser *p, parser::State state)
    {
        p->state_stack.append(state);
    }

    INLINE Opt<parser::State> pop_state(Parser *p)
    {
        if (p->state_stack.empty()) return None;
        return p->state_stack.pop();
    }

    INLINE bool is_within_state(Parser *p, parser::State state)
    {
        if (p->state_stack.empty()) return false;
        usize index = p->state_stack.count() - 1;
        while (index >= 0) {
            const State &current_state = p->state_stack[index--];
            if (current_state.kind == state.kind) return true;
        }
        return false;
    }
} // namespace parser
