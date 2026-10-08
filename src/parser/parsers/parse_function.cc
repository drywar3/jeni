#include "parser/parsers/parse_function.h"
#include "ast/expressions.h"
#include "parser/parser_impl.h"
#include "parser/parsers/parse_typehint.h"
#include "parser/token.h"

bool parse_function_parameters(Parser *p, ast::Function_Prototype *prototype)
{
    auto &parameters = prototype->parameters;
    parser::expect(p, TOKEN_SEP_Lparen);

    while (!parser::equals(p, TOKEN_SEP_Rparen)) {
        if (parser::try_expect(p, TOKEN_SEP_Vararg)) {
            /* after hitting the variadic mark, we do not expect any arguments
             * again */
            prototype->is_variadic = true;
            break;
        }

        ast::Function_Parameter parameter{};
        if (!parser::eat_name(p, &parameter.name)) {
            parser::report(
                p, diag_create(Severity::Error, parser::current(p).locus,
                               "invalid token", "expected parameter name"));
            if (!skip_until_one_of(p, true, TOKEN_SEP_Comma, TOKEN_SEP_Rparen))
                return false;
        } else {
            parser::expect(p, TOKEN_SEP_Colon);
            parameter.typehint = parser_parse_typehint(p);
            parameters.append(parameter);
        }

        if (!parser::try_expect(p, TOKEN_SEP_Comma))
            break;
    }

    /* todo: consider reporting a better suited diagnostic for the case where
     * the parameter list continues even after the varidic mark (if present) */
    parser::expect(p, TOKEN_SEP_Rparen);
    return true;
}

ExpressionPointer parse_function(Parser *p)
{
    Token begin = parser::next(p); /* consume `func` keyword */

    ast::expr::Function function{};
    function.prototype.parameters =
        ast::Function_Prototype::Parameters(p->allocator);
    if (!parse_function_parameters(p, &function.prototype))
        MINI_UNREACHABLE("TODO");

    if (parser::try_expect(p, TOKEN_OP_Arrow)) {
        function.prototype.return_type = parser_parse_typehint(p);
        if (!function.prototype.return_type)
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
