#include "../ast/statements.h"
#include "../parser_impl.h"
#include "../token.h"

#include "parse_typehint.h"
#include "parse_statements.h"
#include "parse_expressions.h"

bool eat_name(Parser *parser, Name *name) {
    if (!equals(parser, TOKEN_Identifier)) {
        MINI_UNREACHABLE("[%s] instead",
                         tokenkind_to_string(current(parser).kind));
        return false;
    }

    Token _current = next(parser);
    name->value   = mini_string_substr(parser->tokens.lexer.content,
                                       /* the lexer lexes tokens on a 1-based
                                        * cursor   so there is need to substract 1
                                        * to get an   accurate character index
                                        */
                                     _current.locus.begin - 1,
                                     locus_length(&_current.locus));
    name->locus   = _current.locus;
    return true;
}

Statement *parse_variable_declaration(Parser *p) {
    MINI_ASSERT(equals_sequence(p, TOKEN_Identifier, TOKEN_SEP_Colon),
                "cannot parse a variable declaration");
    Token begin = current(p);
    StmtVariable variable = {0};
    if (!eat_name(p, &variable.name))
        MINI_UNREACHABLE();
    /* skip `:` after variable name */
    next(p);

    /* check and parse a typehint */
    if (!equals(p, TOKEN_SEP_Colon) && !equals(p, TOKEN_OP_Assign)) {
        variable.typehint = parser_parse_typehint(p);
        if (variable.typehint)
            variable.type_is_defined = true;
    } else {
        variable.type_is_defined = false;
    }

    /* if a variable is declared without initialization,
     * it is assumed that it will be initialized later, so it's mutability is [MUT_Mutable]
     */
    if (equals(p, TOKEN_SEP_Semicolon)) {
        variable.mutability     = MUT_Mutable;
        variable.is_initialized = false;
        variable.initializer    = NULL;
    } else {
        variable.mutability =
            try_expect(p, TOKEN_SEP_Colon) ? MUT_Constant :
            try_expect(p, TOKEN_OP_Assign) ? MUT_Mutable : ({
                    diagpool_report(p->diagnostics,
                                    DIAG_Error,
                                    current(p).locus,
                                    "invalid token",
                                    "expected `:`, `=` or `;`");
                    return NULL;
                    MUT_Mutable;
            });

        variable.initializer = parser_parse_expression(p);
    }

    expect(p, TOKEN_SEP_Semicolon);
    Statement *stmt = ALLOC_STMT(p->allocator,
                                STMT_Variable,
                                locus_merge(begin.locus, previous(p).locus),
                                variable);
    return stmt;
}
