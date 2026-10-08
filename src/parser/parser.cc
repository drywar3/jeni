#include "parser.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "parser/parsers/parse_expressions.h"
#include "parser_impl.h"
#include "parsers/parse_statements.h"

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
    return (parser::current(parser).kind == TOKEN_Endoffile ||
            parser::previous(parser).kind == TOKEN_Endoffile) ||
           tokenbuffer_is_truly_done(&parser->tokens);
}


Statement *parser_parse_statement(Parser *parser)
{
    /* check for a variable declaration */
    if (equals_sequence(parser, TOKEN_Identifier, TOKEN_SEP_Colon)) {
        if (auto variable = parser::parse_variable_declaration(parser))
            return variable;
        skip_until_one_of(parser, false, STMT_HEAD);
        return nullptr;
    }

    if (parser::equals(parser, TOKEN_SEP_Lbrace)) {
        if (auto block = parser::parse_block(parser))
            return block;
        skip_until_one_of(parser, false, STMT_HEAD);
        return nullptr;
    }

    if (parser::equals(parser, TOKEN_KW_If)) {
        if (auto if_ = parser::parse_if_statement(parser))
            return if_;
        skip_until_one_of(parser, false, STMT_HEAD);
        return nullptr;
    }

    if (parser::equals(parser, TOKEN_KW_Return
              )) {
        if (auto ret = parser::parse_return_statement(parser))
            return ret;
        skip_until_one_of(parser, false, STMT_HEAD);
        return nullptr;
    }


    Expression *expr = parser_parse_expression(parser);
    if (!expr || expr->kind == EXPR_Error) {
        skip_until_one_of(parser, false, STMT_HEAD);
        return nullptr;
    }

    expr->base.kind = STMT_Expr;
    parser::expect(parser, TOKEN_SEP_Semicolon);
    return (Statement *)expr;
}

void parser_destroy(Parser *parser) { tokenbuffer_destroy(&parser->tokens); }
