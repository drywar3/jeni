#include "lir/lir.h"

static void lower_block(lir::Buildr *b, const hir::Statement *stmt);
static void lower_variable(lir::Buildr *b, const hir::Statement *stmt);

void lir::lower_statement(lir::Buildr *b, const hir::Statement *stmt)
{
    switch (stmt->kind) {
    case hir::Statement::Kind::Block:
        return lower_block(b, stmt);
    case hir::Statement::Kind::Variable:
        return lower_variable(b, stmt);
    default:
        MINI_UNREACHABLE();
    }
}

void lower_block(lir::Buildr *b, const hir::Statement *stmt)
{
    const hir::stmt::Block &block = stmt->as.block;

    b->new_block();
    for (const auto stmt : block.body.iter()) {
        lir::lower_statement(b, stmt);
    }
    b->end_block();
}

void lower_variable(lir::Buildr *b, const hir::Statement *stmt)
{
    const hir::stmt::Variable &variable = stmt->as.variable;
    lir::TypePtr var_type = lir::lower_type(b, variable.type_id);
    MINI_UNREACHABLE();
}
