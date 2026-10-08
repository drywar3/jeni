#include "lir/lir.h"


static void lower_block(lir::Buildr *b, const hir::Statement *stmt);
static void lower_variable(lir::Buildr *b, const hir::Statement *stmt);
static void lower_if_stmt(lir::Buildr *b, const hir::Statement *stmt);

void lir::lower_statement(lir::Buildr *b, const hir::Statement *stmt)
{
    switch (stmt->kind) {
    case hir::Statement::Kind::Block:
        return lower_block(b, stmt);
    case hir::Statement::Kind::Variable:
        return lower_variable(b, stmt);
    case hir::Statement::Kind::If: return lower_if_stmt(b, stmt);
    case hir::Statement::Kind::Expr:
        lower_expression(b, stmt->as.expr);
        return;
    case hir::Statement::Kind::Function:
        b->add_global(lower_glob_function(b, stmt));
        return;
    case hir::Statement::Kind::Return: {
        const hir::stmt::Return &ret = stmt->as.ret;
        auto type  = lir::lower_type(b, ret.type_id);
        auto value = lir::lower_expression(b, ret.value);
        b->create_ret(type, value);
        return;
    }
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
    auto value = b->create_alloca(variable.name, var_type);
    auto init  = lir::lower_expression(b, variable.initializer);
    b->create_store(b->create_deref(value), var_type, init);
}

void lower_if_stmt(lir::Buildr *b, const hir::Statement *stmt)
{
    const hir::stmt::If &if_ = stmt->as.if_;

    auto condition = lir::lower_expression(b, if_.condition);
    auto true_v    = b->get_const_true();

    lir::Label then_label  = b->new_label();
    lir::Label merge_label = b->new_label();

    usize branch_index = 0;

    auto put_next_branch = [&] (this auto &&self) -> void {
        if (branch_index < if_.branches.count()) {
            lir::Label then_label  = b->new_label();
            const auto &branch = if_.branches[branch_index++];
            auto condition = lir::lower_expression(b, branch.condition);
            b->jmp_if_eq(condition, true_v, then_label);
                self();
                b->jmp_to_label(merge_label);
            b->put_label(then_label);
                lir::lower_statement(b, branch.then);
                b->jmp_to_label(merge_label);
        } else {
            if (if_.else_) {
                lir::lower_statement(b, if_.else_);
            }
            b->jmp_to_label(merge_label);
        }
    };

    b->jmp_if_eq(condition, true_v, then_label);
        put_next_branch();
    b->put_label(then_label);
        lir::lower_statement(b, if_.then);
        b->jmp_to_label(merge_label);
    b->put_label(merge_label);
}
