#include "parse_typehint.h"
#include "parser/parser_impl.h"
#include "ast/types.h"

#include <utility>

constexpr std::initializer_list<std::pair<const char *, int>> TYPES = {
    {"int", TypeInteger::AST_TYPE_INT_Int},
    {"uint", TypeInteger::AST_TYPE_INT_Uint},
    {"isize", TypeInteger::AST_TYPE_INT_Isize},
    {"usize", TypeInteger::AST_TYPE_INT_Usize},
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
    Locus begin            = current(p).locus;
    Mutability is_constant = (Mutability)try_expect(p, TOKEN_KW_Const);

    /* check for pointer */
    if (try_expect(p, TOKEN_OP_Star)) {
        TypePointer pointer{};
        pointer.typehint = parser_parse_typehint(p);
        return ALLOC_TYPE(p->allocator, TYPEHINT_Pointer,
                          locus_merge(begin, current(p).locus), is_constant,
                          pointer);
    }

    /* check for integer */
    {
        if (equals(p, TOKEN_Identifier)) {
            Name name;
            MINI_ASSERT(eat_name(p, &name), "");

            /* check for integer types */
            for (const auto [t, k] : TYPES) {
                if (mini_sv_equals_cstr(name.value, t)) {
                    return ALLOC_TYPE(
                        p->allocator, TYPEHINT_Integer,
                        locus_merge(begin, previous(p).locus), is_constant,
                        (TypeInteger){.kind = (TypeInteger::Kind)k});
                }
            }

            if (mini_sv_equals_cstr(name.value, "string")) {
                return ALLOC_TYPE(p->allocator, TYPEHINT_String,
                                  locus_merge(begin, previous(p).locus),
                                  is_constant, TypeString{});
            }
        }
    }
    MINI_UNREACHABLE("TODO");
}
