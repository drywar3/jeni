#include "parser/parser_impl.h"
#include "parser/parsers/parse_namespace.h"
#include "parser/parsers/parse_statements.h"
#include "parser/parsers/parse_expressions.h"

#include <mini.cc/scope_guard.h>

/*
       ns [in <expression>] {
          <statement>...
       }
 */

Expression *parser::parse_namespace(Parser *p)
{
    Token begin = parser::next(p);
    ast::expr::Namespace ns{};
    if (parser::equals(p, TOKEN_KW_In)) MINI_UNREACHABLE();

    if (!parser::equals(p, TOKEN_SEP_Lbrace)) {
        parser::skip_until_one_of(p, true, {TOKEN_SEP_Rbrace});
        return parser::error_expr();
    }

    parser::push_state(p, State::Namespace_Definition);
    auto scope_guard = mini::ScopeGuard([&]() {parser::pop_state(p);});

    ns.definitions = parser::parse_block(p);
    ns.parent_is_specified = false;
    ns.parent = nullptr;

    return ast::expr::alloc_expression(p->allocator,
                                       EXPR_Namespace,
                                       locus_merge(begin.locus, parser::previous(p).locus),
                                       ns);
}
