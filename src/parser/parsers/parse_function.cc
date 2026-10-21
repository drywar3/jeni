#include "parser/token.h"
#include "ast/expressions.h"
#include "parser/parser_impl.h"
#include "parser/parsers/parse_function.h"
#include "parser/parsers/parse_expressions.h"

bool parse_function_parameter(Parser *p, ast::Function_Parameter *parameter)
{
    if (!parser::eat_name(p, &parameter->name)) {
        parser::report(
                       p, diag_create(Severity::Error, parser::current(p).locus,
                                      "invalid token", "expected parameter name"));
        if (!parser::skip_until_one_of(p, true, {TOKEN_SEP_Comma, TOKEN_SEP_Rparen}))
            return false;
    } else {
        parser::expect(p, TOKEN_SEP_Colon);
        parameter->typehint = parser::parse_typehint_expression(p);
    }
    return true;
}

bool parse_function_parameters(Parser *p, ast::Function_Prototype *prototype)
{
    auto &parameters = prototype->parameters;
    if (!parser::expect(p, TOKEN_SEP_Lparen)) {
        if (!parser::skip_until_one_of(p, false, {TOKEN_Identifier, TOKEN_SEP_Rparen}))
            return false;
    }

    while (!parser::equals(p, TOKEN_SEP_Rparen)) {
        if (parser::try_expect(p, TOKEN_SEP_Vararg)) {
            /* after hitting the variadic mark, we do not expect any arguments
             * again */
            prototype->is_variadic = true;
            break;
        }

        ast::Function_Parameter parameter{};
        if (!parse_function_parameter(p, &parameter)) return false;

        parameters.append(parameter);
        if (!parser::try_expect(p, TOKEN_SEP_Comma))
            break;
    }

    /* todo: consider reporting a better suited diagnostic for the case where
     * the parameter list continues even after the varidic mark (if present) */
    parser::expect(p, TOKEN_SEP_Rparen);
    return true;
}

bool parse_generic_parameters(Parser *p, ast::Generic_Parameters *params)
{
    if (!parser::expect(p, TOKEN_OP_Less)) return false;
    while (!parser_is_done(p) &&
           !parser::equals(p, TOKEN_OP_Greater)) {
        ast::Function_Parameter parameter{};
        if (!parse_function_parameter(p, &parameter)) return false;
        params->append(parameter);
        if (!parser::try_expect(p, TOKEN_SEP_Comma))  break;
    }

    parser::expect(p, TOKEN_OP_Greater);
    return true;
}

bool parse_function_attributes(Parser *p, ast::Attributes *attrs)
{
    while (parser::current(p).is_directive()) {
        Token token = parser::next(p);
        switch (token.kind) {
        case TOKEN_DR_Canon_Name:
            {
                if (!parser::try_expect(p, TOKEN_LIT_String)) {
                    parser::report_error(p, token.locus, "invalid token", "expected string after `#canon_name`");
                    return false;
                }
                token = parser::previous(p);
                mini::StringView value =
                    mini_string_substr(p->tokens.lexer.content, token.locus.first_byte + 1,
                                       locus_length(&token.locus) - 2);
                attrs->append(ast::Attribute{.kind = ast::Attribute::Kind::Canon_Name, .canon_name = value});
            } break;
        default: parser::report_error(p, token.locus, "invalid function attribute"); return false;
        }
    }
    return true;
}

Expression *parse_function(Parser *p)
{
    Token begin = parser::next(p); /* consume `func` keyword */

    ast::expr::Function function{};
    function.prototype.parameters =
        ast::Function_Prototype::Parameters(p->allocator);
    function.attributes =
        ast::Attributes(p->allocator);
    function.generic_params = ast::Generic_Parameters(p->allocator);

    if (parser::equals(p, TOKEN_OP_Less)) {
        if (!parse_generic_parameters(p, &function.generic_params)
            && !parser::skip_until_one_of(p, false, {TOKEN_SEP_Lparen}))
            return parser::error_expr();
    }

    if (!parse_function_parameters(p, &function.prototype)) {
        if (!parser::skip_until_one_of(p, false, {TOKEN_SEP_Lbrace}))
            return parser::error_expr();
    }

    if (parser::try_expect(p, TOKEN_OP_Arrow)) {
        function.prototype.return_type = parser::parse_typehint_expression(p);
        if (!function.prototype.return_type)
            return nullptr;
    }

    if (!parse_function_attributes(p, &function.attributes)) {
        if (!parser::skip_until_one_of(p, false, {TOKEN_SEP_Lbrace}))
            return nullptr;
    }

    if (parser::equals(p, TOKEN_SEP_Lbrace)) {
        function.body_is_defined = true;
        function.body            = parser_parse_statement(p);
        if (function.body == nullptr)
            return nullptr;
    } else {
        parser::expect(p, TOKEN_SEP_Nobody);
        function.body_is_defined = false;
        function.body            = nullptr;
    }

    return ast::expr::alloc_expression(
        p->allocator, EXPR_Function,
        locus_merge(begin.locus, parser::previous(p).locus), function);
}
