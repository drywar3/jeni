#include "ast/expressions.h"
#include "parser/parser_impl.h"
#include "parser/parsers/parse_function.h"
#include "parser/parsers/parse_typehint.h"
#include "parser/token.h"

bool parse_function_parameters(Parser *p,
                               AstFunctionPrototype *prototype)
{
    auto &parameters = prototype->parameters;
    expect(p, TOKEN_SEP_Lparen);

    while (!equals(p, TOKEN_SEP_Rparen)) {
        if (try_expect(p, TOKEN_SEP_Vararg)) {
            /* after hitting the variadic mark, we do not expect any arguments again */
            prototype->is_variadic = true;
            break;
        }

        AstFunctionParameter parameter{};
        if (!eat_name(p, &parameter.name)) {
            parser_report(p, diag_create(Severity::Error, current(p).locus,
                                         "invalid token",
                                         "expected parameter name"));
            if (!skip_until_one_of(p, true, TOKEN_SEP_Comma, TOKEN_SEP_Rparen))
                return false;
        } else {
            expect(p, TOKEN_SEP_Colon);
            parameter.typehint = parser_parse_typehint(p);
            parameters.append(parameter);
        }

        if (!try_expect(p, TOKEN_SEP_Comma))
            break;
    }

    /* todo: consider reporting a better suited diagnostic for the case where the
    *        parameter list continues even after the varidic mark (if present) */
    expect(p, TOKEN_SEP_Rparen);
    return true;
}

ExpressionPointer parse_function(Parser *p)
{
    Token begin = next(p); /* consume `func` keyword */

    ExprFunction function{};
    function.prototype.parameters =
        AstFunctionPrototype::Parameters(p->allocator);
    if (!parse_function_parameters(p, &function.prototype))
        MINI_UNREACHABLE("TODO");

    if (try_expect(p, TOKEN_OP_Arrow)) {
        function.prototype.return_type = parser_parse_typehint(p);
        if (!function.prototype.return_type)
            return nullptr;
    }

    if (equals(p, TOKEN_SEP_Lbrace)) {
        function.body_is_defined = true;
        function.body            = parser_parse_statement(p);
        if (function.body == nullptr)
            return nullptr;
    } else {
        expect(p, TOKEN_SEP_Nobody);
        function.body_is_defined = false;
        function.body            = nullptr;
    }

    return ALLOC_EXPR(p->allocator, EXPR_Function,
                      locus_merge(begin.locus, previous(p).locus), function);
}
