#include "ast/expressions.h"
#include "parser/parser_impl.h"
#include "parser/parsers/parse_function.h"
#include "parser/parsers/parse_typehint.h"

bool parse_function_parameters(Parser *p, AstFunctionPrototype::Parameters *parameters) {
    expect(p, TOKEN_SEP_Lparen);
    while (!equals(p, TOKEN_SEP_Rparen)) {
        AstFunctionParameter parameter;
        if (!eat_name(p, &parameter.name)) MINI_UNREACHABLE("TODO");
        expect(p, TOKEN_SEP_Colon);
        parameter.typehint = parser_parse_typehint(p);
        if (parameters)
            mini_array_append(*parameters, parameter);
        if (!try_expect(p, TOKEN_SEP_Comma))
            break;
    }
    expect(p, TOKEN_SEP_Rparen);
    return true;
}

ExpressionPointer parse_function(Parser *p) {
    Token begin = next(p); /* consume `func` keyword */

    ExprFunction function{};
    if (!parse_function_parameters(p, nullptr)) MINI_UNREACHABLE("TODO");

    if (equals(p, TOKEN_SEP_Lbrace)) {
        function.body_is_defined = true;
        function.body            = parser_parse_statement(p);
        if (function.body == nullptr)
            return nullptr;
    } else {
        function.body_is_defined = false;
        function.body            = nullptr;
    }

    return ALLOC_EXPR(p->allocator, EXPR_Function, locus_merge(begin.locus, previous(p).locus), function);
}
