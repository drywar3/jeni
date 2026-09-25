#include "lexer.h"

#include <mini.c/string_view.h>
#include "token.h"

typedef uint32 Codepoint;

typedef struct {
    const char *text;
    TokenKind   kind;
} TokenSpec;

static const TokenSpec KEYWORDS[] = {
    { "func", TOKEN_KW_Func },
    { "return", TOKEN_KW_Return },
};
static const usize KEYWORD_COUNT = sizeof(KEYWORDS)/sizeof(KEYWORDS[0]);

static const TokenSpec PUNCTUATIONS[] = {
    { "++", TOKEN_OP_Inc },
    { "--", TOKEN_OP_Dec },
    { "==", TOKEN_OP_Equals },
    { "!=", TOKEN_OP_NotEquals },

    { "+", TOKEN_OP_Add },
    { "-", TOKEN_OP_Minus },
    { "*", TOKEN_OP_Star },
    { "/", TOKEN_OP_Div },
    { "=", TOKEN_OP_Assign },
    { ">", TOKEN_OP_Greater },
    { "<", TOKEN_OP_Less },
    { "!", TOKEN_OP_Bang },

    { "{", TOKEN_SEP_Lbrace },
    { "[", TOKEN_SEP_Lbracket },
    { "(", TOKEN_SEP_Lparen },
    { ")", TOKEN_SEP_Rparen },
    { "}", TOKEN_SEP_Rbrace },
    { "]", TOKEN_SEP_Rbracket },
    { ":", TOKEN_SEP_Colon },
    { ";", TOKEN_SEP_Semicolon },
    { ",", TOKEN_SEP_Comma },
};
static const usize PUNCT_COUNT = sizeof(PUNCTUATIONS)/sizeof(PUNCTUATIONS[0]);

Lexer lexer_create(const char *path, const Mini_String content) {
    Lexer lexer = {0};
    lexer.path  = path;
    lexer.content = content;
    lexer.line    = 1;
    lexer.col     = lexer.prev_col = 1;
    return lexer;
}

static bool set_token_and_return(Token *token, TokenKind kind, Locus locus, bool value) {
    token->kind  = kind;
    token->locus = locus;
    return value;
}

static Locus lexer_get_locus(const Lexer *lexer) {
    return locus_create(lexer->line, lexer->prev_col, lexer->col, lexer->prev_offset, lexer->offset, lexer->path);
}

static size_t decode_utf8_char(const char *str, Codepoint *codepoint) {
    const uint8_t *bytes = (const uint8_t *)str;
    if (!bytes || !*bytes) return 0;

    uint8_t b0 = bytes[0];

    // 1-byte sequence (ASCII: 0xxxxxxx)
    if ((b0 & 0x80) == 0x00) {
        *codepoint = b0;
        return 1;
    }

    // 2-byte sequence (110xxxxx 10xxxxxx)
    if ((b0 & 0xE0) == 0xC0) {
        if ((bytes[1] & 0xC0) != 0x80) return 0; // Invalid continuation byte
        *codepoint = ((b0 & 0x1F) << 6) | (bytes[1] & 0x3F);
        return (*codepoint >= 0x80) ? 2 : 0;     // Reject overlong encodings
    }

    // 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
    if ((b0 & 0xF0) == 0xE0) {
        if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80) return 0;
        *codepoint = ((b0 & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);

        // Reject overlongs and UTF-16 surrogates (0xD800 - 0xDFFF)
        if (*codepoint < 0x800 || (*codepoint >= 0xD800 && *codepoint <= 0xDFFF)) return 0;
        return 3;
    }

    // 4-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
    if ((b0 & 0xF8) == 0xF0) {
        if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80) return 0;
        *codepoint = ((b0 & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) | ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);

        // Reject overlongs and code points above the Unicode limit (0x10FFFF)
        if (*codepoint < 0x10000 || *codepoint > 0x10FFFF) return 0;
        return 4;
    }

    return 0; // Invalid lead byte
}

static Codepoint current(const Lexer *lexer) {
    if (lexer_is_done(lexer))
        return 0;
    Codepoint cp;
    usize length = decode_utf8_char(&lexer->content[lexer->offset], &cp);
    if (length == 0)
        return 0;
    return cp;
}

static Codepoint next(Lexer *lexer) {
    if (lexer_is_done(lexer))
        return 0;
    Codepoint cp;
    usize length = decode_utf8_char(&lexer->content[lexer->offset], &cp);
    if (cp == '\n') {
        lexer->line += 1;
        lexer->col   = 1;
    } else {
        lexer->col += 1;
    }
    lexer->offset += length;
    return cp;
}

static bool codepoint_isalpha(Codepoint cp) {
    return
        (cp >= 'a' && cp <= 'z') ||
        (cp >= 'A' && cp <= 'Z') ||
        (cp == '_') ||
        (cp > 127);
}

static bool codepoint_isdigit(Codepoint cp, int base) {
    if (base == 10) {
        return (cp >= '0' && cp <= '9');
    } else if (base == 8) {
        return (cp >= '0' && cp <= '7');
    } else if (base == 2) {
        return (cp == '0' || cp == '1');
    } else if (base == 16) {
        return
            (cp >= '0' && cp <= '7') ||
            (cp >= 'a' && cp <= 'f') ||
            (cp >= 'A' && cp <= 'F');
    }
    MINI_UNREACHABLE();
}

static bool codepoint_isspace(Codepoint cp) {
    return
        (cp == ' ') ||
        (cp == '\t') ||
        (cp == '\r') ||
        (cp == '\n');
}

static bool is_prefix(const Lexer *lexer, const char *prefix) {
    if (lexer_is_done(lexer)) {
        return false;
    }

    Mini_StringView head = mini_sv_init(&lexer->content[lexer->offset],
                                        mini_string_count(lexer->content) - lexer->offset);
    if (mini_sv_begins_with(head, prefix)) {
        return true;
    }

    return false;
}

static bool eat_prefix(Lexer *lexer, const char *prefix) {
    if (is_prefix(lexer, prefix)) {
        usize n = strlen(prefix);
        for (usize c = 0; c < n; c++) {
            next(lexer);
        }
        return true;
    }
    return false;
}

bool lexer_next_token(Lexer *lexer, Token *token, Diagnostic *diagnostic) {
    if (lexer_is_done(lexer)) {
        return set_token_and_return(token, TOKEN_Endoffile, lexer_get_locus(lexer), true);
    }

    while (true) {
        if (codepoint_isspace(current(lexer))) {
            next(lexer);
            continue;
        }

        /* skip single line comments. */
        if (is_prefix(lexer, "//")) {
            while (current(lexer) != '\n' && current(lexer) != '\r') {
                next(lexer);
            }
            continue;
        }

        break;
    }

    if (lexer_is_done(lexer)) {
        return set_token_and_return(token, TOKEN_Endoffile, lexer_get_locus(lexer), true);
    }

    lexer->prev_offset = lexer->offset;
    lexer->prev_col    = lexer->col;

    /* lex an identifier or keyword */
    if (codepoint_isalpha(current(lexer))) {
        while (!lexer_is_done(lexer) && codepoint_isalpha(current(lexer))) {
            next(lexer);
        }
        usize length = lexer->offset - lexer->prev_offset;
        Mini_StringView word = mini_string_substr(lexer->content,
                                                  lexer->prev_offset,
                                                  length);
        for (usize n = 0; n < KEYWORD_COUNT; n++) {
            TokenSpec spec = KEYWORDS[n];
            if (mini_sv_equals_cstr(word, spec.text)) {
                return set_token_and_return(token,
                                            spec.kind,
                                            lexer_get_locus(lexer),
                                            true);
            }
        }

        return set_token_and_return(token,
                                    TOKEN_Identifier,
                                    lexer_get_locus(lexer),
                                    true);
    }

    /* lex integers:
     *
     * if it starts with a base 10 digit that isn't 0, then it is fully base 10.
     * else if it does start with 0, then we are looking a base prefix
     * i.e 0x, 0o, 0d, 0b.
     */
    if (codepoint_isdigit(current(lexer), 10)) {
        int continue_base = 10;
        if (current(lexer) == '0') {
            MINI_UNREACHABLE("TODO");
        }

        /* lex decimal part */
        while (!lexer_is_done(lexer) && codepoint_isdigit(current(lexer), continue_base))
            next(lexer);

        /* check for (and lex) floating point part */
        if (current(lexer) == '.') {
            next(lexer);
            while (!lexer_is_done(lexer) && codepoint_isdigit(current(lexer), continue_base))
                next(lexer);
        }

        /* todo: lex integer suffixes i.e u8, i8, i16, uz */

        return set_token_and_return(token, TOKEN_LIT_Int, lexer_get_locus(lexer), true);
    }


    /* lex punctuations (operators and separators) */
    {
        for (usize n = 0; n < PUNCT_COUNT; n++) {
            TokenSpec spec = PUNCTUATIONS[n];
            if (eat_prefix(lexer, spec.text)) {
                return set_token_and_return(token, spec.kind, lexer_get_locus(lexer), true);
            }
        }
    }

    MINI_UNREACHABLE("%d - '%c'", current(lexer), current(lexer));
}

bool lexer_is_done(const Lexer *lexer) {
    if (lexer == NULL)
        return true;
    // todo: something seems off here :|
    return lexer->offset >= mini_string_count(lexer->content) - 1;
}
