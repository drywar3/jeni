#pragma once

#include "diagnostic.h"
#include "lexer.h"
#include "token.h"
#include "ast/stmt.h"
#include "token_buffer.h"

#include <mini.c/allocator.h>

#define STMT_HEAD                                                              \
    TOKEN_KW_If, TOKEN_KW_Do, TOKEN_KW_For, TOKEN_KW_Cast, TOKEN_KW_Continue,  \
        TOKEN_KW_Import, TOKEN_KW_Enum, TOKEN_KW_Break, TOKEN_KW_Func,         \
        TOKEN_KW_Return, TOKEN_SEP_Lbracket, TOKEN_SEP_Lparen,                 \
        TOKEN_SEP_Lbrace, TOKEN_OP_Inc, TOKEN_OP_Dec, TOKEN_LIT_Int,           \
        TOKEN_LIT_Char, TOKEN_LIT_String, TOKEN_LIT_True, TOKEN_LIT_False

struct Parser {
    TokenBuffer tokens;
    DiagnosticPool *diagnostics;
    Token current, previous;

    Mini_Allocator allocator;
};

Parser parser_create(SourceId id, const Mini_String content,
                     DiagnosticPool *diagnostics);
void parser_destroy(Parser *parser);

void parser_set_allocator(Parser *parser, Mini_Allocator allocator);
bool parser_is_done(const Parser *parser);

Statement *parser_parse_statement(Parser *parser);
