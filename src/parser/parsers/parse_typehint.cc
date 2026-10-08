#include "parse_typehint.h"
#include "ast/types.h"
#include "mini.c/string_view.h"
#include "parser/parser_impl.h"

#include <utility>

constexpr std::initializer_list<std::pair<const char *, int>> TYPES = {
    {"int", TypeInteger::Int},
    {"uint", TypeInteger::Uint},
    {"isize", TypeInteger::Isize},
    {"usize", TypeInteger::Usize},
    {"int64", TypeInteger::Int64},
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
        TypePointer pointer{};
        pointer.mutability = (Mutability)parser::try_expect(p, TOKEN_KW_Const);
        pointer.typehint   = parser_parse_typehint(p);
        return ALLOC_TYPE(p->allocator, TYPEHINT_Pointer,
                          locus_merge(begin, parser::current(p).locus), pointer);
    }

    if (parser::equals(p, TOKEN_Identifier)) {
        Name name;
        MINI_ASSERT(parser::eat_name(p, &name), "");

        /* check for integer */
        {
            for (const auto [t, k] : TYPES) {
                if (mini_sv_equals_cstr(name.value, t)) {
                    return ALLOC_TYPE(
                        p->allocator, TYPEHINT_Integer,
                        locus_merge(begin, parser::previous(p).locus),
                        TypeInteger{.kind = (TypeInteger::Kind)k});
                }
            }
        }

        {
            if (mini_sv_equals_cstr(name.value, "char")) {
                return ALLOC_TYPE(p->allocator, TYPEHINT_Char,
                                  locus_merge(begin, parser::previous(p).locus),
                                  TypeChar{});
            }
        }

        {
            if (mini_sv_equals_cstr(name.value, "string")) {
                return ALLOC_TYPE(p->allocator, TYPEHINT_String,
                                  locus_merge(begin, parser::previous(p).locus),
                                  TypeString{});
            }
        }
    }

    MINI_UNREACHABLE("TODO");
}
