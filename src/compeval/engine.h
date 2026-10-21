#pragma once

#include "ast/expr.h"
#include "compeval/cpu.h"
#include "compeval/value.h"
#include "compeval/function.h"
#include "compeval/call_frame.h"
#include "compeval/instruction.h"
#include "semantics/entities/symbol.h"

namespace compeval
{
    struct Engine {
        using SymbolEvalCache = HashMap<sema::Symbol_Id, compeval::Value>;

        Mini_Allocator  allocator;
        SymbolEvalCache cache;
        Cpu *cpu;

        compeval::Bytecode_Function *current_function;

        Engine(Mini_Allocator allocator);

        void compile(Semantic_Context *sema, Expression *expression);

        Opt<compeval::Value> eval(Semantic_Context *sema, Expression *expression);
        Opt<compeval::Value> eval(Semantic_Context *sema, Statement  *stmt);

        compeval::Bytecode_Function *swap_function_context(compeval::Bytecode_Function *next);
        void add_instruction(compeval::Instruction instruction);
    };
} // namespace compeval
