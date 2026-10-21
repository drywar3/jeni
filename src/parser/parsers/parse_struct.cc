#include "ast/expressions.h"
#include "parser/parser_impl.h"
#include "parser/parsers/parse_struct.h"
#include "parser/parsers/parse_expressions.h"

Expression *parser::parse_struct_decl(Parser *p)
{
    MINI_ASSERT(parser::equals(p, TOKEN_KW_Struct),
                "cannot parse struct declaration from this branch");
    Token begin = parser::next(p);

    if (!parser::expect(p, TOKEN_SEP_Lbrace)) {
        parser::skip_until_one_of(p, true, {TOKEN_SEP_Rbrace});
        return parser::error_expr();
    }

    ast::expr::Struct strukt{};
    strukt.fields = ast::expr::Struct::Fields(p->allocator);

    while (!parser_is_done(p) && !parser::equals(p, TOKEN_SEP_Rbrace)) {
        ast::Struct_Field field{};
        field.names = mini::Array<Name>(p->allocator);
        while (true) {
            Name name;
            if (parser::equals(p, TOKEN_Identifier)) {
                MINI_ASSERT(parser::eat_name(p, &name), );
            }

            field.names.append(name);
            if (!parser::try_expect(p, TOKEN_SEP_Comma)) break;
        }

        if (!parser::expect(p, TOKEN_SEP_Colon)) return parser::error_expr();

        field.type_is_defined = false;
        if (parser::equals(p, TOKEN_OP_Assign)) {
            field.has_default_value = true;
            field.default_value     = parser::parse_expression(p);
            if (!field.names.empty()) MINI_UNREACHABLE("TODO");
        } else {
            field.has_default_value = false;
            field.typehint          = parser::parse_typehint_expression(p);
            if (field.typehint->is_error()) return field.typehint;
            field.type_is_defined = true;
        }

        strukt.fields.append(field);

        if (!parser::try_expect(p, TOKEN_SEP_Semicolon)) break;
    }

    parser::expect(p, TOKEN_SEP_Rbrace);
    return ast::expr::alloc_expression(p->allocator,
                                       Expression_Kind::EXPR_Struct,
                                       locus_merge(begin.locus, previous(p).locus),
                                       strukt);
}
