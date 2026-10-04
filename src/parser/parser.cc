#include "parser.h"
#include "ast/stmt.h"
#include "ast/expr.h"
#include "parser_impl.h"
#include "parsers/parse_statements.h"
#include "parser/parsers/parse_expressions.h"

Parser parser_create(SourceId id, const Mini_String content,
                     DiagnosticPool *diagnostics)
{
    Parser parser = {.tokens = tokenbuffer_create(id, content, diagnostics)};
    parser.diagnostics = diagnostics;
    parser.allocator   = mini_default_allocator();
    tokenbuffer_prepare(&parser.tokens, 10);
    parser.current = tokenbuffer_peek(&parser.tokens, 0);
    // next(&parser);
    // printf("[%s]\n", tokenkind_to_string(parser.current.kind));
    return parser;
}

void parser_set_allocator(Parser *parser, Mini_Allocator allocator)
{
    parser->allocator = allocator;
}

bool parser_is_done(const Parser *parser)
{
    return (current(parser).kind == TOKEN_Endoffile ||
            previous(parser).kind == TOKEN_Endoffile) ||
           tokenbuffer_is_truly_done(&parser->tokens);
}

#define STMT_HEAD                                                 \
    TOKEN_KW_If, TOKEN_KW_Do,                                     \
        TOKEN_KW_For, TOKEN_KW_Cast,                              \
        TOKEN_KW_Continue, TOKEN_KW_Import,                       \
        TOKEN_KW_Enum, TOKEN_KW_Break,                            \
        TOKEN_KW_Func, TOKEN_KW_Return,                           \
        TOKEN_SEP_Lbracket, TOKEN_SEP_Lparen, TOKEN_SEP_Lbrace,   \
        TOKEN_OP_Inc, TOKEN_OP_Dec,                               \
        TOKEN_LIT_Int,                                            \
        TOKEN_LIT_Char,                                           \
        TOKEN_LIT_String,                                         \
        TOKEN_LIT_True,                                           \
        TOKEN_LIT_False

Statement *parser_parse_statement(Parser *parser)
{
    /* check for a variable declaration */
    if (equals_sequence(parser, TOKEN_Identifier, TOKEN_SEP_Colon)) {
        if (auto variable = parse_variable_declaration(parser)) return variable;
        skip_until_one_of(parser, false, STMT_HEAD);
        return nullptr;
    }

    if (equals(parser, TOKEN_SEP_Lbrace)) {
        if (auto block = parse_block(parser)) return block;
        skip_until_one_of(parser, false, STMT_HEAD);
        return nullptr;
    }

    Expression *expr = parser_parse_expression(parser);
    if (!expr || expr->kind == EXPR_Error) {
        skip_until_one_of(parser, false, STMT_HEAD);
        return nullptr;
    }

    expr->base.kind = STMT_Expr;
    expect(parser, TOKEN_SEP_Semicolon);
    return (Statement*)expr;
}

void parser_destroy(Parser *parser) { tokenbuffer_destroy(&parser->tokens); }
