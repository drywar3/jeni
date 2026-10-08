#include "parse_expressions.h"
#include "ast/expressions.h"
#include "parser/parser_impl.h"
#include "parser/parsers/parse_function.h"
#include "parser/token.h"

static Expression ERROR_EXPR{
    .kind = EXPR_Error,
};

ExpressionPointer parse_relational(Parser *p);

ExpressionPointer parse_primary(Parser *p)
{
    if (parser::equals(p, TOKEN_LIT_Int)) {
        Token token  = parser::next(p);
        char *buffer = MINI_ALLOC_MANY(mini_default_allocator(), char,
                                       locus_length(&token.locus) + 1);
        Mini_StringView value_sv =
            mini_string_substr(p->tokens.lexer.content, token.locus.first_byte,
                               locus_length(&token.locus));
        memcpy(buffer, value_sv.data, value_sv.length);
        buffer[value_sv.length] = '\0';
        int64 value             = strtoll(buffer, NULL, 10);
        MINI_FREE(mini_default_allocator(), buffer);
        return ast::expr::alloc_expression(p->allocator, EXPR_Integer,
                                           token.locus,
                                           ast::expr::Integer{.value = value});
    }

    if (parser::equals(p, TOKEN_Identifier)) {
        Token token = parser::next(p);
        Mini_StringView value =
            mini_string_substr(p->tokens.lexer.content, token.locus.first_byte,
                               locus_length(&token.locus));
        return ast::expr::alloc_expression(
            p->allocator, EXPR_Identifier, token.locus,
            ast::expr::Identifier{.value = value});
    }

    if (parser::equals(p, TOKEN_LIT_CString)) {
        Token token = parser::next(p);
        Mini_StringView value =
            mini_string_substr(p->tokens.lexer.content, token.locus.first_byte,
                               locus_length(&token.locus));
        return ast::expr::alloc_expression(p->allocator, EXPR_CString,
                                           token.locus,
                                           ast::expr::String{.value = value});
    }

    parser::report(p, diag_create(Severity::Error, parser::current(p).locus,
                                  "expected primary expression", "here"));
    return &ERROR_EXPR;
}

ExpressionPointer parse_postfix(Parser *p)
{
    Expression *base = parse_primary(p);
    while (true) {
        if (parser::try_expect(p, TOKEN_SEP_Lparen)) {
            ast::expr::Function_Call fcall;
            fcall.callee = base;
            fcall.arguments =
                mini::Array<ast::Function_Call_Argument>(p->allocator);

            while (!parser::equals(p, TOKEN_SEP_Rparen)) {
                ast::Function_Call_Argument argument{};
                if (parser::try_expect(p, TOKEN_SEP_Colon)) {
                    argument.is_positional = false;
                    if (!parser::eat_name(p, &argument.name)) {
                        MINI_UNREACHABLE("could not parse argument name");
                    }
                }
                argument.argument = parser_parse_expression(p);
                mini_array_append(fcall.arguments, argument);
                if (!parser::try_expect(p, TOKEN_SEP_Comma))
                    break;
            }

            parser::expect(p, TOKEN_SEP_Rparen);
            base = ast::expr::alloc_expression(
                p->allocator, EXPR_Function_Call,
                locus_merge(base->locus, parser::previous(p).locus), fcall);
            continue;
        }

        break;
    }

    return base;
}

ExpressionPointer parse_unary(Parser *p)
{
    if (parser::equals(p, TOKEN_OP_Bang) || parser::equals(p, TOKEN_OP_Minus) ||
        parser::equals(p, TOKEN_OP_Add) || parser::equals(p, TOKEN_OP_Inc) ||
        parser::equals(p, TOKEN_OP_Dec)) {
        Token begin                  = parser::next(p);
        auto op                      = ast::Operator(begin.kind);
        ExpressionPointer expression = parse_postfix(p);
        return ast::expr::alloc_expression(
            p->allocator, EXPR_Unary,
            locus_merge(begin.locus, expression->locus),
            ast::expr::Unary_Operation{.op = op, .expression = expression});
    }
    return parse_postfix(p);
}

ExpressionPointer parse_factor(Parser *p)
{
    ExpressionPointer left = parse_unary(p);
    while (parser::equals(p, TOKEN_OP_Star) ||
           parser::equals(p, TOKEN_OP_Div)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_unary(p);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

ExpressionPointer parse_term(Parser *p)
{
    ExpressionPointer left = parse_factor(p);
    while (parser::equals(p, TOKEN_OP_Add) ||
           parser::equals(p, TOKEN_OP_Minus)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_factor(p);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

Expression *parse_assignment(Parser *p)
{
    Expression *left = parse_term(p);
    if (parser::equals(p, TOKEN_OP_Assign)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_relational(p);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

ExpressionPointer parse_comparison(Parser *p)
{
    ExpressionPointer left = parse_assignment(p);
    while (parser::equals(p, TOKEN_OP_Greater) ||
           parser::equals(p, TOKEN_OP_Less) ||
           parser::equals(p, TOKEN_OP_LessEq) ||
           parser::equals(p, TOKEN_OP_GreaterEq)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_term(p);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

ExpressionPointer parse_relational(Parser *p)
{
    ExpressionPointer left = parse_comparison(p);
    while (parser::equals(p, TOKEN_OP_Equals) ||
           parser::equals(p, TOKEN_OP_NotEquals)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_comparison(p);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

ExpressionPointer parser_parse_expression(Parser *p)
{
    if (parser::equals(p, TOKEN_KW_Func)) {
        return parse_function(p);
    }

    return parse_relational(p);
}
