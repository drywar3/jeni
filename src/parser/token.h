#pragma once

#include "locus.h"

typedef enum {
    TOKEN_OP_Add = 0,
    TOKEN_OP_Minus = 1,
    TOKEN_OP_Star = 2,
    TOKEN_OP_Div = 3,
    TOKEN_OP_Inc,
    TOKEN_OP_Dec,
    TOKEN_OP_Assign,
    TOKEN_OP_Greater,
    TOKEN_OP_Less,
    TOKEN_OP_GreaterEq,
    TOKEN_OP_LessEq,
    TOKEN_OP_Equals,
    TOKEN_OP_NotEquals,
    TOKEN_OP_Bang,
    TOKEN_OP_Arrow,
    TOKEN_OP_Caret,

    TOKEN_KW_If,
    TOKEN_KW_Then,
    TOKEN_KW_Else,
    TOKEN_KW_For,
    TOKEN_KW_While,
    TOKEN_KW_Do,
    TOKEN_KW_Break,
    TOKEN_KW_In,
    TOKEN_KW_Continue,
    TOKEN_KW_Return,
    TOKEN_KW_Import,
    TOKEN_KW_Struct,
    TOKEN_KW_Enum,
    TOKEN_KW_Union,
    TOKEN_KW_Const,
    TOKEN_KW_Cast,
    TOKEN_KW_Func,
    TOKEN_KW_Namespace,

    TOKEN_DIRECTIVES,
    TOKEN_DR_Eval,
    TOKEN_DR_Canon_Name,
    TOKEN_DR_Use,
    TOKEN_DIRECTIVES_END,

    TOKEN_TYPE_KEYWORDS,
    TOKEN_TK_Void,
    TOKEN_TK_Int,
    TOKEN_TK_Int8,
    TOKEN_TK_Int16,
    TOKEN_TK_Int32,
    TOKEN_TK_Int64,
    TOKEN_TK_Float32,
    TOKEN_TK_Float64,
    TOKEN_TK_Uint,
    TOKEN_TK_String,
    TOKEN_TK_Char,
    TOKEN_TK_Uchar,
    TOKEN_TK_Bool,
    TOKEN_TK_Usize,
    TOKEN_TK_Isize,
    TOKEN_TYPE_KEYWORDS_END,

    TOKEN_SEP_Semicolon,
    TOKEN_SEP_Colon,
    TOKEN_SEP_Comma,
    TOKEN_SEP_Lbrace,
    TOKEN_SEP_Rbrace,
    TOKEN_SEP_Lbracket,
    TOKEN_SEP_Rbracket,
    TOKEN_SEP_Lparen,
    TOKEN_SEP_Rparen,
    TOKEN_SEP_Dot,
    // used to indicate that a function has no body
    TOKEN_SEP_Nobody, // ---
    TOKEN_SEP_Vararg, // ...

    TOKEN_LIT_Int,
    TOKEN_LIT_Float,
    TOKEN_LIT_String,
    TOKEN_LIT_RString,
    TOKEN_LIT_CString,
    TOKEN_LIT_Char,
    TOKEN_LIT_True,
    TOKEN_LIT_False,
    TOKEN_LIT_Null,

    TOKEN_Identifier,
    TOKEN_Endoffile,
} TokenKind;

struct Token {
    TokenKind kind;
    Locus locus;
    int   base;

    bool is_directive() const { return kind > TOKEN_DIRECTIVES && kind < TOKEN_DIRECTIVES_END; }
};

Token token_create(TokenKind kind, Locus locus, int base = 10);

const char *tokenkind_to_string(TokenKind kind);
