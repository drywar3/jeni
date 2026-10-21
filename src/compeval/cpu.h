#pragma once

#include "compeval/instruction.h"

#include <mini.c/allocator.h>

struct Semantic_Context;

namespace compeval
{
    struct Cpu;

    Cpu *cpu_init(Mini_Allocator allocator);
    void cpu_deinit(Cpu *cpu);
    void cpu_load(Instruction *instructions);
    void cpu_step(Semantic_Context *sema, Cpu *cpu);
} // namespace compeval
