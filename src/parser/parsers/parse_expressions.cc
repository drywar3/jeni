#include "parse_expressions.h"
#include "ast/expressions.h"
#include "parser/parser_impl.h"

ExpressionPointer parse_primary(Parser *p) {
    if (equals(p, TOKEN_LIT_Int)) {
        Token token = next(p);
        char *buffer = MINI_ALLOC_MANY(mini_default_allocator(),
                                       char,
                                       locus_length(&token.locus));
        Mini_StringView value_sv = mini_string_substr(p->tokens.lexer.content,
                                                      token.locus.first_byte,
                                                      locus_length(&token.locus));
        memcpy(buffer, value_sv.data, value_sv.length);
        int64 value = strtoll(buffer, NULL, 10);
        MINI_FREE(mini_default_allocator(), buffer);
        return ALLOC_EXPR(p->allocator, EXPR_Integer, token.locus, ((ExprInteger){ .value = value }));
    }

    if (equals(p, TOKEN_Identifier)) {
        Token token = next(p);
        Mini_StringView value = mini_string_substr(p->tokens.lexer.content,
                                                   token.locus.first_byte,
                                                   locus_length(&token.locus));
        return ALLOC_EXPR(p->allocator, EXPR_Identifier, token.locus, ((ExprIdentifier){ .value = value }));
    }

    MINI_UNREACHABLE("[%s]", tokenkind_to_string(p->current.kind));
}

ExpressionPointer parse_postfix(Parser *p) {
    Expression *base = parse_primary(p);
    while (true) {
        if (try_expect(p, TOKEN_SEP_Lparen)) {
            ExprFunctionCall fcall;
            fcall.callee    = base;
            fcall.arguments = MINI_ARRAY_INIT(p->allocator, AstFunctionCallArgument);

            while (!equals(p, TOKEN_SEP_Rparen)) {
                AstFunctionCallArgument argument;
                if (try_expect(p, TOKEN_SEP_Colon)) {
                    argument.is_positional = false;
                    if (!eat_name(p, &argument.name)) {
                        MINI_UNREACHABLE("could not parse argument name");
                    }
                }
                argument.argument = parser_parse_expression(p);
                mini_array_append(fcall.arguments, argument);
                if (!try_expect(p, TOKEN_SEP_Comma))
                    break;
            }

            expect(p, TOKEN_SEP_Rparen);
            base = ALLOC_EXPR(p->allocator,
                              EXPR_FunctionCall,
                              locus_merge(base->locus, previous(p).locus),
                              fcall);
            continue;
        }

        break;
    }

    return base;
}

ExpressionPointer parse_unary(Parser *p) {
    if (equals(p, TOKEN_OP_Bang)
        || equals(p, TOKEN_OP_Minus)
        || equals(p, TOKEN_OP_Add)
        || equals(p, TOKEN_OP_Inc)
        || equals(p, TOKEN_OP_Dec)
       ) {
        Token begin    = next(p);
        AstOperator op = (AstOperator)begin.kind;
        ExpressionPointer expression = parse_postfix(p);
        return ALLOC_EXPR(p->allocator,
                          EXPR_Unary,
                          locus_merge(begin.locus, expression->locus),
                          ((ExprUnaryOperation){ .op = op, .expression = expression }));
    }
    return parse_postfix(p);
}

ExpressionPointer parse_factor(Parser *p) {
    ExpressionPointer left = parse_unary(p);
    while (equals(p, TOKEN_OP_Star) || equals(p, TOKEN_OP_Div)) {
        ExprBinaryOperation binop{};
        binop.left  = left;
        binop.op    = (AstOperator)next(p).kind;
        binop.right = parse_unary(p);
        left        = ALLOC_EXPR(p->allocator,
                                 EXPR_Binop,
                                 locus_merge(left->locus, binop.right->locus),
                                 binop);
    }
    return left;
}

ExpressionPointer parse_term(Parser *p) {
    ExpressionPointer left = parse_factor(p);
    while (equals(p, TOKEN_OP_Add) || equals(p, TOKEN_OP_Minus)) {
        ExprBinaryOperation binop{};
        binop.left  = left;
        binop.op    = (AstOperator)next(p).kind;
        binop.right = parse_factor(p);
        left        = ALLOC_EXPR(p->allocator,
                                 EXPR_Binop,
                                 locus_merge(left->locus, binop.right->locus),
                                 binop);
    }
    return left;
}

ExpressionPointer parse_comparison(Parser *p) {
    ExpressionPointer left = parse_term(p);
    while (equals(p, TOKEN_OP_Greater) || equals(p, TOKEN_OP_Less)) {
        ExprBinaryOperation binop{};
        binop.left  = left;
        binop.op    = (AstOperator)next(p).kind;
        binop.right = parse_term(p);
        left        = ALLOC_EXPR(p->allocator,
                                 EXPR_Binop,
                                 locus_merge(left->locus, binop.right->locus),
                                 binop);
    }
    return left;
}

ExpressionPointer parse_relational(Parser *p) {
    ExpressionPointer left = parse_comparison(p);
    while (equals(p, TOKEN_OP_Equals) || equals(p, TOKEN_OP_NotEquals)) {
        ExprBinaryOperation binop{};
        binop.left  = left;
        binop.op    = (AstOperator)next(p).kind;
        binop.right = parse_comparison(p);
        left        = ALLOC_EXPR(p->allocator,
                                 EXPR_Binop,
                                 locus_merge(left->locus, binop.right->locus),
                                 binop);
    }
    return left;
}

ExpressionPointer parser_parse_expression(Parser *p) {
    if (equals(p, TOKEN_KW_Func)) {
        MINI_UNREACHABLE();
    }

    return parse_relational(p);
}
