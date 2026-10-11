#include "parse_typehint.h"
#include "ast/types.h"
#include "mini.c/string_view.h"
#include "parser/parser_impl.h"
#include "parser/parsers/parse_expressions.h"

#include <utility>

constexpr std::initializer_list<std::pair<int, int>> TYPES = {
    {TOKEN_TK_Int, ast::type::Integer::Int},
    {TOKEN_TK_Uint, ast::type::Integer::Uint},
    {TOKEN_TK_Isize, ast::type::Integer::Isize},
    {TOKEN_TK_Usize, ast::type::Integer::Usize},
    {TOKEN_TK_Int64, ast::type::Integer::Int64},
};

/* typehint = ["const"] type_spec ;
 *
 * type_spec =
 *      pointer
 *      | integer
 *      | "bool"
 *      | array
 *      ;
 *
 * pointer = "*" typehint ;
 *
 * integer = "int" | "uint" | "usize" | "isize" ;
 *
 * array = "[" expression "]" typehint ;
 */
Typehint *parser_parse_typehint(Parser *p)
{
    Locus begin = parser::current(p).locus;

    /* check for pointer */
    if (parser::try_expect(p, TOKEN_OP_Star)) {
        ast::type::Pointer pointer{};
        pointer.mutability = (Mutability)parser::try_expect(p, TOKEN_KW_Const);
        pointer.typehint   = parser::parse_typehint_expression(p);
        return ast::type::alloc_typehint(
            p->allocator, TYPEHINT_Pointer,
            locus_merge(begin, parser::current(p).locus), pointer);
    }

    Token token = parser::next(p);

    if (token.kind > TOKEN_TYPE_KEYWORDS &&
        token.kind < TOKEN_TYPE_KEYWORDS_END) {
        switch (token.kind) {
        case TOKEN_TK_Int:
        case TOKEN_TK_Uint:
        case TOKEN_TK_Int8:
        case TOKEN_TK_Int16:
        case TOKEN_TK_Int32:
        case TOKEN_TK_Int64:
            for (const auto [t, k] : TYPES) {
                if (token.kind == t) {
                    return ast::type::alloc_typehint(
                        p->allocator, TYPEHINT_Integer,
                        locus_merge(begin, parser::previous(p).locus),
                        ast::type::Integer{.kind =
                                               (ast::type::Integer::Kind)k});
                }
            }
        case TOKEN_TK_Char:
            return ast::type::alloc_typehint(
                p->allocator, TYPEHINT_Char,
                locus_merge(begin, parser::previous(p).locus),
                ast::type::Char{});
        case TOKEN_TK_Float32:
            return ast::type::alloc_typehint(
                p->allocator, TYPEHINT_Float,
                locus_merge(begin, parser::previous(p).locus),
                ast::type::Float{.width = ast::type::Float::Float32});
        case TOKEN_TK_Float64:
            return ast::type::alloc_typehint(
                p->allocator, TYPEHINT_Float,
                locus_merge(begin, parser::previous(p).locus),
                ast::type::Float{.width = ast::type::Float::Float64});
        case TOKEN_TK_String:
            return ast::type::alloc_typehint(
                p->allocator, TYPEHINT_String,
                locus_merge(begin, parser::previous(p).locus),
                ast::type::String{});
        default:
            break;
        }
    }
    MINI_UNREACHABLE("TODO");
}
