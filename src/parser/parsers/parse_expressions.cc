#include "parser/token.h"
#include "ast/expressions.h"
#include "parse_expressions.h"
#include "parser/parser_impl.h"
#include "parser/parsers/parse_struct.h"
#include "parser/parsers/parse_function.h"
#include "parser/parsers/parse_typehint.h"

#include <mini.cc/scope_guard.h>

static Expression ERROR_EXPR{
    .kind = EXPR_Error,
};

Expression *parser::error_expr()
{
    return &ERROR_EXPR;
}

Expression *parse_relational(Parser *p, bool is_within_typehint_context = false);

Expression *parse_primary(Parser *p, bool is_within_typehint_context = false)
{
    if (!is_within_typehint_context && parser::equals(p, TOKEN_LIT_Int)) {
        /* todo: add conversion mechanism to mini.string_view(c) and mini.string_view(cc)
         *       so we don't have to allocate a temporary buffer here */
        Token token  = parser::next(p);
        char *buffer = MINI_ALLOC_MANY(mini_default_allocator(), char,
                                       locus_length(&token.locus) + 1);

        auto scope_guard = mini::ScopeGuard([&] () { MINI_FREE(mini_default_allocator(), buffer); });

        int64 value;
        if (token.base == 8) {
            /* [strtol] and other function alike expect a base 8 integer literal
             * with prefix `0` but the lexer lexes them with prefix `0o`
             * so we have to build a valid c octal integer first before converting */
            Mini_StringView value_sv =
                mini_string_substr(p->tokens.lexer.content, token.locus.first_byte,
                               locus_length(&token.locus));
            memcpy(buffer, "0", 1);
            memcpy(buffer, value_sv.data + 2, value_sv.length - 2);
            buffer[value_sv.length - 1] = '\0';
            value                   = strtol(buffer, NULL, token.base);
        } else {
            Mini_StringView value_sv =
                mini_string_substr(p->tokens.lexer.content, token.locus.first_byte,
                                   locus_length(&token.locus));
            memcpy(buffer, value_sv.data, value_sv.length);
            buffer[value_sv.length] = '\0';
            value                   = strtol(buffer, NULL, token.base);
        }
        return ast::expr::alloc_expression(p->allocator, EXPR_Integer,
                                           token.locus,
                                           ast::expr::Integer{.value = value});
    }

    if (!is_within_typehint_context && parser::equals(p, TOKEN_LIT_Float)) {
        /* todo: add conversion mechanism to mini.string_view(c) and mini.string_view(cc)
         *       so we don't have to allocate a temporary buffer here */
        Token token  = parser::next(p);
        char *buffer = MINI_ALLOC_MANY(mini_default_allocator(), char,
                                       locus_length(&token.locus) + 1);

        auto scope_guard = mini::ScopeGuard([&] () { MINI_FREE(mini_default_allocator(), buffer); });
        Mini_StringView value_sv =
            mini_string_substr(p->tokens.lexer.content, token.locus.first_byte,
                               locus_length(&token.locus));
        memcpy(buffer, value_sv.data, value_sv.length);
        buffer[value_sv.length] = '\0';
        auto value                   = strtof(buffer, NULL);
        return ast::expr::alloc_expression(p->allocator, EXPR_Float,
                                           token.locus,
                                           ast::expr::Float{.value = value});
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

    if (!is_within_typehint_context && parser::equals(p, TOKEN_LIT_CString)) {
        Token token = parser::next(p);
        Mini_StringView value =
            mini_string_substr(p->tokens.lexer.content, token.locus.first_byte,
                               locus_length(&token.locus));
        return ast::expr::alloc_expression(p->allocator, EXPR_CString,
                                           token.locus,
                                           ast::expr::String{.value = value});
    }

    if (is_within_typehint_context) {
        Expression *typehint = parser_parse_typehint(p);
        typehint->kind = EXPR_Typehint;
        return typehint;
    }

    parser::report(p, diag_create(Severity::Error, parser::current(p).locus,
                                  "expected primary expression", "here"));
    return &ERROR_EXPR;
}

Expression *parse_member_access(Parser *p, bool is_within_typehint_context = false)
{
    Expression *base = parse_primary(p, is_within_typehint_context);
    if (is_within_typehint_context) return base;

    while (parser::equals(p, TOKEN_SEP_Dot)) {
        Token dot = parser::next(p);
        Name member;

        if (parser::equals(p, TOKEN_Identifier)) {
            parser::eat_name(p, &member);
        } else {
            parser::report_error(p, dot, "expected identifier after `.`");
            return parser::error_expr();
        }

        base = ast::expr::alloc_expression(p->allocator,
                                           EXPR_Member_Access,
                                           locus_merge(base->locus, member.locus),
                                           ast::expr::Member_Access{.receiver = base, .member = member});
    }

    return base;
}

Expression *parse_postfix(Parser *p, bool is_within_typehint_context = false)
{
    Expression *base = parse_member_access(p, is_within_typehint_context);
    if (is_within_typehint_context) return base;
    while (true) {
        if (parser::try_expect(p, TOKEN_SEP_Lparen) && !is_within_typehint_context) {
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

                argument.argument = parser::parse_expression(p, is_within_typehint_context);
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

Expression *parse_unary(Parser *p, bool is_within_typehint_context = false)
{
    if (parser::equals(p, TOKEN_OP_Star) && is_within_typehint_context) {
        Expression *typehint = parser_parse_typehint(p);
        if (typehint->is_error()) return typehint;
        typehint->kind = EXPR_Typehint;
        return typehint;
    }

    if (!is_within_typehint_context) {
        if (parser::equals(p, TOKEN_OP_Bang) || parser::equals(p, TOKEN_OP_Minus) ||
            parser::equals(p, TOKEN_OP_Add) || parser::equals(p, TOKEN_OP_Inc) ||
            parser::equals(p, TOKEN_OP_Dec)) {
            Token begin                  = parser::next(p);
            auto op                      = ast::Operator(begin.kind);
            Expression *expression = parse_postfix(p, is_within_typehint_context);
            return ast::expr::alloc_expression(
                                               p->allocator, EXPR_Unary,
                                               locus_merge(begin.locus, expression->locus),
                                               ast::expr::Unary_Operation{.op = op, .expression = expression});
        }
    }

    return parse_postfix(p, is_within_typehint_context);
}

Expression *parse_factor(Parser *p, bool is_within_typehint_context = false)
{
    Expression *left = parse_unary(p, is_within_typehint_context);
    if (is_within_typehint_context) return left;

    while (parser::equals(p, TOKEN_OP_Star) ||
           parser::equals(p, TOKEN_OP_Div)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_unary(p, is_within_typehint_context);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

Expression *parse_term(Parser *p, bool is_within_typehint_context = false)
{
    Expression *left = parse_factor(p, is_within_typehint_context);
    if (is_within_typehint_context) return left;
    while (parser::equals(p, TOKEN_OP_Add) ||
           parser::equals(p, TOKEN_OP_Minus)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_factor(p, is_within_typehint_context);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

Expression *parse_assignment(Parser *p, bool is_within_typehint_context = false)
{
    Expression *left = parse_term(p, is_within_typehint_context);
    if (is_within_typehint_context) return left;
    if (parser::equals(p, TOKEN_OP_Assign)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_relational(p, is_within_typehint_context);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

Expression *parse_comparison(Parser *p, bool is_within_typehint_context = false)
{
    Expression *left = parse_assignment(p, is_within_typehint_context);
    if (is_within_typehint_context) return left;
    while (parser::equals(p, TOKEN_OP_Greater) ||
           parser::equals(p, TOKEN_OP_Less) ||
           parser::equals(p, TOKEN_OP_LessEq) ||
           parser::equals(p, TOKEN_OP_GreaterEq)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_term(p, is_within_typehint_context);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

Expression *parse_relational(Parser *p, bool is_within_typehint_context)
{
    Expression *left = parse_comparison(p, is_within_typehint_context);

    if (is_within_typehint_context) return left;

    while (parser::equals(p, TOKEN_OP_Equals) ||
           parser::equals(p, TOKEN_OP_NotEquals)) {
        ast::expr::Binary_Operation binop{};
        binop.left  = left;
        binop.op    = ast::Operator(parser::next(p).kind);
        binop.right = parse_comparison(p, is_within_typehint_context);
        left        = ast::expr::alloc_expression(
            p->allocator, EXPR_Binop,
            locus_merge(left->locus, binop.right->locus), binop);
    }
    return left;
}

Expression *parser::parse_expression(Parser *p, bool is_within_typehint_context)
{
    if (!is_within_typehint_context && parser::equals(p, TOKEN_KW_Struct))
        return parser::parse_struct_decl(p);

    if (!is_within_typehint_context && parser::equals(p, TOKEN_KW_Func)) {
        return parse_function(p);
    }

    return parse_relational(p, is_within_typehint_context);
}

Expression *parser::parse_typehint_expression(Parser *p)
{
    parser::push_state(p, State::Typehint_Expression);
    auto *typehint = parser::parse_expression(p, true);
    parser::pop_state(p);
    return typehint;
}
