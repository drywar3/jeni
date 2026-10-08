#pragma once

#include <mini.c/allocator.h>

#include "ast/expressions.h"
#include "ast/statements.h"
#include "hir/types/stmt.h"
#include "semantics/sema.h"

namespace hir
{
    using Program = MINI_ARRAY(hir::Statement *);

    struct Context {
        Mini_Allocator allocator;
        const Semantic_Storage *store;

        auto &types() { return store->types; }
        auto &scopes() { return store->scopes; }
        auto &symbols() { return store->symbols; }

        const auto &types() const { return store->types; }
        const auto &scopes() const { return store->scopes; }
        const auto &symbols() const { return store->symbols; }

        hir::Expression *create_true_expr();

        template <typename T>
        hir::Statement *new_stmt(hir::Statement::Kind kind, T obj)
        {
            static_assert(sizeof(T) <= sizeof(hir::Statement::as),
                          "Type T exceeds union storage size");

            hir::Statement *stmt = MINI_ALLOC(allocator, hir::Statement);
            stmt->kind           = kind;

            ::new (static_cast<void *>(&stmt->as)) T(std::move(obj));

            return stmt;
        }

        template <typename T>
        hir::Expression *new_expr(hir::Expression::Kind kind,
                                  sema::Type_Id type_id, T obj)
        {
            hir::Expression *expr = MINI_ALLOC(allocator, hir::Expression);
            expr->kind            = kind;
            expr->type_id         = type_id;

            T *mem = (T *)&expr->as;
            *mem   = obj;
            return expr;
        }
    };

    Context ctx_init(Mini_Allocator allocator, const Semantic_Storage *store);
    Program program_from_raw(Context *ctx, ::Program ast);
    hir::Statement *convert_statement(hir::Context *ctx,
                                      const ::Statement *stmt);

    hir::Expression *convert_expression(hir::Context *ctx,
                                        const ::Expression *expression);
    hir::Statement *convert_block(hir::Context *ctx, ast::stmt::Block *block);
    hir::Expression *
    convert_function_call(hir::Context *ctx,
                          const ast::expr::Function_Call *call);
    hir::Expression *
    convert_binary_op(hir::Context *ctx,
                      const ast::expr::Binary_Operation *binop);

    hir::Statement *convert_if_stmt(hir::Context *ctx,
                                    const ast::stmt::If *if_stmt);
    hir::Statement *convert_for_ever_stmt(hir::Context *ctx,
                                          const ast::stmt::For_Ever *for_ever);
} // namespace hir
