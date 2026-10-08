#include "hir/convert.h"
#include "semantics/type/ids.h"
#include "hir/convert_variable.h"

hir::Expression *hir::Context::create_true_expr()
{
    return new_expr(hir::Expression::Kind::Boolean, sema::type_id::Bool,
                    hir::expr::Boolean{true});
}

hir::Context hir::ctx_init(Mini_Allocator allocator,
                           const Semantic_Storage *store)
{
    return {allocator, store};
}

hir::Program hir::program_from_raw(Context *ctx, ::Program ast)
{
    auto *program = MINI_ARRAY_INIT(ctx->allocator, hir::Statement *);
    for (usize n = 0; n < mini_array_count(ast.ast); ++n) {
        const ::Statement *stmt = ast.ast[n];
        auto *hir_stmt          = hir::convert_statement(ctx, stmt);
        mini_array_append(program, hir_stmt);
    }
    return program;
}

hir::Statement *hir::convert_statement(hir::Context *ctx,
                                       const ::Statement *stmt)
{
    switch (stmt->kind) {
    case STMT_Variable:
        return convert_variable_stmt(ctx, stmt);
    case STMT_Block:
        return convert_block(ctx, (ast::stmt::Block *)stmt);
    case STMT_If:
        return convert_if_stmt(ctx, (ast::stmt::If *)stmt);
    case STMT_Return: {
        auto *ret_ = (ast::stmt::Return *)stmt;
        hir::stmt::Return ret{};
        ret.value   = hir::convert_expression(ctx, ret_->value);
        ret.type_id = sema::Type_Id(*ctx->types().get_id(ret_->value->locus));
        return ctx->new_stmt(hir::Statement::Kind::Return, ret);
    } break;
    case STMT_For_Ever: return hir::convert_for_ever_stmt(ctx, (ast::stmt::For_Ever *)stmt);
    case STMT_Break:    return ctx->new_stmt(hir::Statement::Kind::Break, hir::stmt::Break{});
    case STMT_Expr: {
        hir::Expression *expression =
            convert_expression(ctx, (const ::Expression *)stmt);
        return ctx->new_stmt(hir::Statement::Kind::Expr, expression);
    } break;
    default:
        MINI_UNREACHABLE();
    }
}
