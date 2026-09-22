#include "token.h"

const char *tokenkind_to_string(TokenKind kind) {
    return (const char*[]) {
        [TOKEN_KW_Func] = "func",
        [TOKEN_KW_Return] = "return",

        [TOKEN_OP_Add] = "+",
        [TOKEN_OP_Minus] = "-",
        [TOKEN_OP_Star] = "*",
        [TOKEN_OP_Greater] = ">",
        [TOKEN_OP_Assign] = "=",

        [TOKEN_SEP_Colon] = ":",
        [TOKEN_SEP_Comma] = ",",
        [TOKEN_SEP_Semicolon] = ";",
        [TOKEN_SEP_Lbrace] = "{",
        [TOKEN_SEP_Rbrace] = "}",
        [TOKEN_SEP_Lparen] = "(",
        [TOKEN_SEP_Rparen] = ")",

        [TOKEN_LIT_Int] = "integer literal",

        [TOKEN_Identifier] = "identifier",
        [TOKEN_Endoffile]  = "EOF"
    } [(int)kind];
}
