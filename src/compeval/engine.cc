#include "semantics/sema.h"
#include "semantics/impl.h"
#include "compeval/engine.h"
#include "ast/statements.h"
#include "ast/expressions.h"

compeval::Bytecode_Function *
compeval::Engine::swap_function_context(compeval::Bytecode_Function *next)
{
    auto *previous = current_function;
    current_function = next;
    return previous;
}

void compeval::Engine::add_instruction(compeval::Instruction instruction)
{
    MINI_ASSERT(current_function != nullptr, "cannot add cpu instruction without function context");
    current_function->instructions.append(instruction);
}

compeval::Engine::Engine(Mini_Allocator allocator)
    : allocator(allocator)
{
}

static Opt<compeval::Value> eval_function_call(Semantic_Context *sema, compeval::Engine *vm, Expression *function_call);

void compile_identifier(Semantic_Context *sema, compeval::Engine *vm, Expression *expr);
void compile_binary_op (Semantic_Context *sema, compeval::Engine *vm, Expression *expr);
void compile_function_call(Semantic_Context *sema, compeval::Engine *vm, Expression *function_call);

static Opt<compeval::Value> eval_identifier(Semantic_Context *sema, compeval::Engine *vm, Expression *expr);
static Opt<compeval::Value> eval_binary_op(Semantic_Context *sema, compeval::Engine *vm, Expression *expr);
void compile_stmt_to_bytecode(Semantic_Context *sema, compeval::Engine *vm, Statement *stmt);

void compeval::Engine::compile(Semantic_Context *sema, Expression *expr)
{
    switch (expr->kind) {
    case EXPR_Integer:
        add_instruction(Instruction(Instruction::Push,
                                    Value(((ast::expr::Integer*)expr)->value)));
        break;
    case EXPR_Float:
        add_instruction(Instruction(Instruction::Push,
                                    Value(((ast::expr::Float*)expr)->value)));
        break;
    case EXPR_Identifier:
        compile_identifier(sema, this, expr);
        break;
    case EXPR_Binop:
        compile_binary_op(sema, this, expr);
        break;
    case EXPR_Function_Call:
        compile_function_call(sema, this, expr);
        break;
    case EXPR_Cast:
        return compile(sema, ((ast::expr::Cast*)expr)->expr);
    default: MINI_UNREACHABLE("%d", expr->kind);
    }
}

Opt<compeval::Value> compeval::Engine::eval(Semantic_Context *sema, Statement  *stmt)
{
    compeval::Bytecode_Function fn{};
    fn.instructions = mini::Array<compeval::Instruction>(allocator);
    auto *save = swap_function_context(&fn);
    compile_stmt_to_bytecode(sema, this, stmt);
    swap_function_context(save);

    //cpu.push_call_frame();
    for (auto inst : fn.instructions.iter()) {
        cpu_step(sema, cpu);
    }
    //cpu.pop_call_frame();
    return None;
}

Opt<compeval::Value> compeval::Engine::eval(Semantic_Context *sema, Expression *expr)
{
    MINI_UNREACHABLE();
//     compeval::Bytecode_Function fn{};
//     fn.instructions = mini::Array<compeval::Instruction>(allocator);
//     auto *save = swap_function_context(&fn);
//     compile(sema, expr);
//     swap_function_context(save);
//
//     //cpu.push_call_frame();
//     for (auto inst : fn.instructions.iter()) {
//         cpu_step(sema, cpu);
//     }
//     //cpu.pop_call_frame();
//
//     if (expr->kind == EXPR_Function_Call) {
//         return cpu.get_register(Vm_Cpu::R1);
//     }
//     return cpu.pop_from_stack();
}

void compile_variable_to_bytecode(Semantic_Context *sema, compeval::Engine *vm, Statement *stmt)
{
    auto *var = (ast::stmt::Variable *)stmt;
    auto symbol = *sema::get_symbol_at_locus(sema, var->name.locus);
    sema::Symbol_Id symbol_id = symbol.get_id();
    vm->compile(sema, var->initializer);
    vm->add_instruction(compeval::Instruction(compeval::Instruction::Store,
                                              compeval::Value(compeval::Value::Symbol_Id(symbol_id))));
}

void compile_return_to_bytecode(Semantic_Context *sema, compeval::Engine *vm, Statement *stmt)
{
    auto *ret = (ast::stmt::Return *)stmt;
    vm->compile(sema, ret->value);
    vm->add_instruction(compeval::Instruction(compeval::Instruction::Ret));
}

void compile_stmt_to_bytecode(Semantic_Context *sema, compeval::Engine *vm, Statement *stmt)
{
    switch (stmt->kind) {
    case STMT_Variable: return compile_variable_to_bytecode(sema, vm, stmt);
    case STMT_Return:   return compile_return_to_bytecode(sema, vm, stmt);
    default: MINI_UNREACHABLE("TODO");
    }
}

void compile_block_to_bytecode(Semantic_Context *sema, compeval::Engine *vm, Statement *stmt)
{
    auto *block = (ast::stmt::Block *)stmt;
    for (auto *stmt : block->body.iter()) {
        compile_stmt_to_bytecode(sema, vm, stmt);
    }
}

compeval::Bytecode_Function compile_function_to_bytecode(Semantic_Context *sema, compeval::Engine *vm, Statement *stmt)
{
    auto *function = ((ast::expr::Function*)((ast::stmt::Variable *)stmt)->initializer);
    MINI_ASSERT(function->body != nullptr, "cannot evaluate funtion without a body at compiletime");

    compeval::Bytecode_Function bcfn{};
    bcfn.instructions  = mini::Array<compeval::Instruction>(vm->allocator);
    bcfn.param_symbols = mini::Array<sema::Symbol_Id>(vm->allocator);

    auto *save        = vm->swap_function_context(&bcfn);

    for (const auto param : function->prototype.parameters.iter()) {
        bcfn.param_symbols.append(sema::Symbol_Id(*sema->symbols().get_id(param.name.locus)));
    }

    int index = bcfn.param_symbols.count() - 1;
    while (index >= 0) {
        vm->add_instruction(compeval::Instruction(compeval::Instruction::Param, compeval::Instruction::Operand(bcfn.param_symbols[index])));
        index -= 1;
    }

    compile_block_to_bytecode(sema, vm, function->body);

    vm->swap_function_context(save);
    return bcfn;
}

Opt<compeval::Value> eval_function(Semantic_Context *sema, compeval::Engine *vm, compeval::Bytecode_Function *fn)
{
    MINI_UNREACHABLE();
//     for (auto inst : fn->instructions.iter()) {
//         cpu_step(sema, vm->cpu);
//     }
//     return vm->cpu.get_register(compeval::Vm_Cpu::R1);
}

void compile_function_call(Semantic_Context *sema, compeval::Engine *vm, Expression *expr)
{
    MINI_UNREACHABLE();
    // auto *function_call = (ast::expr::Function_Call*)expr;
    // auto *callee        = function_call->callee;
    // auto callee_sym_opt = sema::get_symbol_at_locus(sema, callee->locus);
    // MINI_ASSERT(callee_sym_opt.has_value(), );
    //
    // auto callee_sym = *callee_sym_opt;
    // auto callee_id   = callee_sym.get_id();
    //
    // MINI_ASSERT(callee_sym->kind == sema::Symbol_Kind::Function, "symbol is not callable");
    //
    // if (!vm->cpu.functions.contains(callee_id)) {
    //     vm->cpu.functions[callee_id] = compile_function_to_bytecode(sema, vm, callee_sym->function.statement);
    // }
    // for (auto arg : function_call->arguments.iter()) {
    //     MINI_ASSERT(arg.is_positional, );
    //     vm->compile(sema, arg.argument);
    // }
    // vm->add_instruction(compeval::Instruction(compeval::Instruction::Call,
    //                                              callee_id));
}

Opt<compeval::Value> eval_function_call(Semantic_Context *sema, compeval::Engine *vm, Expression *expr)
{
    MINI_UNREACHABLE();
    // auto *function_call = (ast::expr::Function_Call*)expr;
    // auto *callee        = function_call->callee;
    // auto callee_sym_opt = sema::get_symbol_at_locus(sema, callee->locus);
    // MINI_ASSERT(callee_sym_opt.has_value(), );
    // auto callee_sym = *callee_sym_opt;
    // auto callee_id   = callee_sym.get_id();
    //
    // MINI_ASSERT(callee_sym->kind == sema::Symbol_Kind::Function, "symbol is not callable");
    //
    // if (!vm->cpu.functions.contains(callee_id)) {
    //     vm->cpu.functions[callee_id] = compile_function_to_bytecode(sema, vm, callee_sym->function.statement);
    // }
    //
    // auto *fn = &vm->cpu.functions[callee_id];
    //
    // vm->cpu.push_call_frame();
    // auto sp = vm->cpu.get_register(compeval::STACK_TOP);
    // auto result = eval_function(sema, vm, fn);
    // vm->cpu.set_register(compeval::STACK_TOP, sp);
    // vm->cpu.pop_call_frame();

    //return result;
}

void compile_binary_op(Semantic_Context *sema, compeval::Engine *vm, Expression *expr)
{
    auto *binop = (ast::expr::Binary_Operation*)expr;
    vm->compile(sema, binop->left);
    vm->compile(sema, binop->right);
    auto opcode = (compeval::Instruction::Opcode)binop->op;
    vm->add_instruction(compeval::Instruction(opcode));
#if 0
    // hack:
    if ((lhs->kind != compeval::Value::Int && lhs->kind != compeval::Value::Float) &&
        (rhs->kind != compeval::Value::Int && rhs->kind != compeval::Value::Float)) {
        MINI_UNREACHABLE();
    }

    double lhs_v;
    double rhs_v;

    if (lhs->kind == compeval::Value::Int)
        lhs_v = lhs->integer;
    else lhs_v = lhs->floating;

    if (rhs->kind == compeval::Value::Int)
        rhs_v = rhs->integer;
    else rhs_v = rhs->floating;

    if (lhs->kind == compeval::Value::Int) {
        switch (binop->op) {
        case ast::Operator::Add:
            return compeval::Value((int64)(lhs_v + rhs_v));
        case ast::Operator::Sub:
            return compeval::Value((int64)(lhs_v - rhs_v));
        case ast::Operator::Mul:
            return compeval::Value((int64)(lhs_v * rhs_v));
        case ast::Operator::Div:
            return compeval::Value((int64)(lhs_v / rhs_v));
        default: MINI_UNREACHABLE();
        }
    } else {
        switch (binop->op) {
        case ast::Operator::Add:
            return compeval::Value((double)(lhs_v + rhs_v));
        case ast::Operator::Sub:
            return compeval::Value((double)(lhs_v - rhs_v));
        case ast::Operator::Mul:
            return compeval::Value((double)(lhs_v * rhs_v));
        case ast::Operator::Div:
            return compeval::Value((double)(lhs_v / rhs_v));
        default: MINI_UNREACHABLE();
        }
    }
    // hack-end
    MINI_UNREACHABLE();
#endif
}

void compile_identifier(Semantic_Context *sema, compeval::Engine *vm, Expression *expr)
{
    auto *ident = (ast::expr::Identifier *)expr;
    auto symbol_opt = sema::get_symbol_at_locus(sema, expr->locus);
    MINI_ASSERT(symbol_opt.has_value(), );
    auto symbol = *symbol_opt;

    if (symbol->kind != sema::Symbol_Kind::Variable &&
        symbol->kind != sema::Symbol_Kind::Parameter) {
        sema::report_error(sema, *expr, "expected a variable");
        return;
    }

    /* ensure the variable was defined as a constant */
    if (!symbol->variable.mutability) {
        sema::report_error(sema, *ident, "expected a constant value", "expression is not comptime");
        return;
    }


    vm->add_instruction(compeval::Instruction(compeval::Instruction::Load_Symbol,
                                              compeval::Instruction::Operand(symbol.get_id())));
}
