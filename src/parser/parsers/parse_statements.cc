#include "ast/statements.h"
#include "parser/parser_impl.h"
#include "parser/token.h"

#include "parse_typehint.h"
#include "parse_statements.h"
#include "parse_expressions.h"

static bool expression_requires_terminator(ExpressionPointer expr)
{
    if (!expr) return true;
    switch (expr->kind) {
    case EXPR_Function:
        return false;
    default:
        return true;
    }
}

bool parser::eat_name(Parser *parser, Name *name)
{
    if (!parser::equals(parser, TOKEN_Identifier)) {
        return false;
    }

    Token _current = parser::next(parser);
    name->value    = mini_string_substr(parser->tokens.lexer.content,
  /* the lexer lexes tokens on a 1-based
   * cursor   so there is need to substract 1
   * to get an   accurate character index
   */
                                        _current.locus.first_byte,
                                        locus_length(&_current.locus));
    name->locus    = _current.locus;
    return true;
}

Statement *parser::parse_variable_declaration(Parser *p)
{
    MINI_ASSERT(equals_sequence(p, TOKEN_Identifier, TOKEN_SEP_Colon),
                "cannot parse a variable declaration");
    Token begin = current(p);
    ast::stmt::Variable variable{};
    if (!parser::eat_name(p, &variable.name))
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
     * it is assumed that it will be initialized later, so it's mutability is
     * [MUT_Mutable]
     */
    if (equals(p, TOKEN_SEP_Semicolon)) {
        variable.mutability     = Mutability::Mutable;
        variable.is_initialized = false;
        variable.initializer    = NULL;
    } else {
        variable.mutability =
            try_expect(p, TOKEN_SEP_Colon) ? Mutability::Constant
            : try_expect(p, TOKEN_OP_Assign)
            ? Mutability::Mutable
            : ({
                    diagpool_report(p->diagnostics, Severity::Error,
                                    current(p).locus, "invalid token",
                                    "expected `:`, `=` or `;`");
                    next(p);
                    Mutability::Mutable;
                });

        variable.initializer    = parser_parse_expression(p);
        if (!variable.initializer || variable.initializer->kind == EXPR_Error) {
            variable.is_initialized = false;
            skip_until_one_of(p, true, TOKEN_SEP_Semicolon);
            return
                ast::stmt::alloc_statement(p->allocator, STMT_Variable,
                                           locus_merge(begin.locus, previous(p).locus), variable);
        }
        variable.is_initialized = true;
    }

    if (!variable.is_initialized ||
        (variable.is_initialized &&
         expression_requires_terminator(variable.initializer))) {
        expect(p, TOKEN_SEP_Semicolon);
    }

    Statement *stmt =
        ast::stmt::alloc_statement(p->allocator, STMT_Variable,
                                   locus_merge(begin.locus, previous(p).locus), variable);
    return stmt;
}

StatementPointer parser::parse_block(Parser *p)
{
    Token begin = next(p); /* consume `{` */
    ast::stmt::Block block{};
    block.body = MINI_ARRAY_INIT(p->allocator, StatementPointer);
    while (!equals(p, TOKEN_SEP_Rbrace) && !parser_is_done(p)) {
        StatementPointer stmt = parser_parse_statement(p);
        if (stmt)
            mini_array_append(block.body, stmt);
        else {
            if (!skip_until_one_of(p, false, STMT_HEAD))
                return nullptr;
        }
    }
    expect(p, TOKEN_SEP_Rbrace);
    return ast::stmt::alloc_statement(p->allocator, STMT_Block,
                                      locus_merge(begin.locus, previous(p).locus), block);
}

Statement *parser::parse_if_statement(Parser *p)
{
    const auto parse_if_header = [](Parser *p, auto &header) -> bool {
        header.condition = parser_parse_expression(p);

        if (header.condition->is_error()) {
            if (!skip_until_one_of(p, false, TOKEN_KW_Then, TOKEN_KW_Return, TOKEN_SEP_Lbrace))
                return false;
        }

        if (try_expect(p, TOKEN_KW_Then) || equals(p, TOKEN_KW_Return)) {
            header.then = parser_parse_statement(p);
            // if (!skip_until_one_of(p, false, TOKEN_SEP_Semicolon, TOKEN_KW_Else))
            if (!header.then) return false;
        } else if (equals(p, TOKEN_SEP_Lbrace)) {
            header.then = parser::parse_block(p);
            if (!header.then) return false;
        } else {
            parser::report_error(p, current(p).locus,
                                 "expected `then` or `{`; `then` must be used for single-statement if");
            header.then = parser_parse_statement(p);
            if (!header.then) return false;
        }

        return true;
    };

    Token begin = next(p);
    ast::stmt::If if_stmt{};

    if_stmt.branches = mini::Array<ast::If_Branch>(p->allocator);

    if (!parse_if_header(p, if_stmt)) return nullptr;

    while (eat_sequence(p, TOKEN_KW_Else, TOKEN_KW_If)) {
        ast::If_Branch branch{};
        if (!parse_if_header(p, branch)) return nullptr;
        if_stmt.branches.append(branch);
    }


    if (try_expect(p, TOKEN_KW_Else)) {
        if_stmt.else_ = parser_parse_statement(p);
        if (!if_stmt.else_) return nullptr;
    }

    return ast::stmt::alloc_statement(p->allocator, STMT_If,
                                      locus_merge(begin.locus, previous(p).locus), if_stmt);
}

Statement *parser::parse_return_statement(Parser *p)
{
    Token begin = next(p);
    Expression *expression = parser_parse_expression(p);
    if (expression->is_error()) {
        if (!skip_until_one_of(p, false, TOKEN_SEP_Semicolon))
            return nullptr;
    }
    expect(p, TOKEN_SEP_Semicolon);
    return ast::stmt::alloc_statement(p->allocator, STMT_Return,
                                      locus_merge(begin.locus, previous(p).locus),
                                      ast::stmt::Return{.value=expression});
}

Statement *parser::parse_for_loop(Parser *p)
{
    Token begin = next(p);

    if (parser::equals(p, TOKEN_SEP_Lbrace)) {
        ast::stmt::For_Ever for_ever{};
        for_ever.body = parser_parse_statement(p);
        if (!for_ever.body) return nullptr;
        return ast::stmt::alloc_statement(p->allocator,
                                          STMT_For_Ever,
                                          locus_merge(begin.locus, previous(p).locus),
                                          for_ever);
    }

    MINI_UNREACHABLE();
}

Statement *parser::parse_break(Parser *p)
{
    Token begin = next(p);
    parser::expect(p, TOKEN_SEP_Semicolon);
    return ast::stmt::alloc_statement(p->allocator,
                                      STMT_Break,
                                      locus_merge(begin.locus, previous(p).locus),
                                      ast::stmt::Break{});
}
