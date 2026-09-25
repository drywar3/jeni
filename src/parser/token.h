#pragma once

#include "locus.h"

typedef enum {
    TOKEN_KW_If,
    TOKEN_KW_Else,
    TOKEN_KW_For,
    TOKEN_KW_While,
    TOKEN_KW_Do,
    TOKEN_KW_Break,
    TOKEN_KW_Continue,
    TOKEN_KW_Return,
    TOKEN_KW_Import,
    TOKEN_KW_Struct,
    TOKEN_KW_Enum,
    TOKEN_KW_Union,
    TOKEN_KW_Const,
    TOKEN_KW_Cast,
    TOKEN_KW_Func,

    TOKEN_OP_Add,
    TOKEN_OP_Minus,
    TOKEN_OP_Div,
    TOKEN_OP_Star,
    TOKEN_OP_Inc,
    TOKEN_OP_Dec,
    TOKEN_OP_Assign,
    TOKEN_OP_Greater,
    TOKEN_OP_Less,
    TOKEN_OP_Equals,
    TOKEN_OP_NotEquals,
    TOKEN_OP_Bang,

    TOKEN_SEP_Semicolon,
    TOKEN_SEP_Colon,
    TOKEN_SEP_Comma,
    TOKEN_SEP_Lbrace,
    TOKEN_SEP_Rbrace,
    TOKEN_SEP_Lbracket,
    TOKEN_SEP_Rbracket,
    TOKEN_SEP_Lparen,
    TOKEN_SEP_Rparen,

    TOKEN_LIT_Int,
    TOKEN_LIT_String,
    TOKEN_LIT_RString,
    TOKEN_LIT_Char,
    TOKEN_LIT_True,
    TOKEN_LIT_False,
    TOKEN_LIT_Null,

    TOKEN_Identifier,
    TOKEN_Endoffile,
} TokenKind;

typedef struct {
    TokenKind kind;
    Locus     locus;
} Token;

Token token_create(TokenKind kind, Locus locus);

const char *tokenkind_to_string(TokenKind kind);
