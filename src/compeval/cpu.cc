#include "semantics/impl.h"
#include "compeval/engine.h"

#define STACK_TOP     Cpu::R8
#define STACK_BOTTOME Cpu::R7

using namespace compeval;

using byte = uint8;

struct compeval::Cpu {
    enum Reg {
        R1, // function return values.
        R2, R3, R4, R5, R6,
        R7, // stack bottom.
        R8, // stack top.
        COUNT,
    };

    static constexpr usize STACK_SIZE = (1024*1024);

    byte   *memory;

    uint64 stack_top;
    uint64 stack_base;

    uint64 registers[Reg::COUNT];

    uint64 rip;                 // instruction pointer

    Mini_Allocator   allocator;

    Cpu(Mini_Allocator allocator);
    ~Cpu();

    Cpu &operator=(const Cpu& other) = delete;
    Cpu(const Cpu& other) = delete;

    Cpu &operator=(Cpu&& other);
    Cpu(Cpu&& other);
};


Cpu *compeval::cpu_init(Mini_Allocator allocator)
{
    Cpu *cpu = MINI_ALLOC(allocator, Cpu);
    new (cpu) Cpu(allocator);
    return cpu;
}

void compeval::cpu_deinit(Cpu *cpu);
void compeval::cpu_load(Instruction *instructions);

void compeval::cpu_step(Semantic_Context *sema, Cpu *cpu)
{
    MINI_UNREACHABLE();
}


Cpu::Cpu(Mini_Allocator allocator)
    : allocator(allocator),
      stack_top(0),
      stack_base(0)
{
    memory = MINI_ALLOC_MANY(allocator,
                             byte,
                             compeval::Cpu::STACK_SIZE);
    MINI_ASSERT(memory != nullptr, "unable to initialize cpu stack");

    registers[Reg::R7] = (int64)0;
    registers[Reg::R8] = (int64)0;
}

Cpu::~Cpu()
{
    if (memory)        MINI_FREE(allocator, memory);
}

Cpu::Cpu(Cpu&& other)
    : memory(other.memory),
      stack_top(other.stack_top),
      stack_base(other.stack_base),
      allocator(other.allocator)
{
    std::memcpy(registers, other.registers, sizeof(registers[0])*Reg::COUNT);
    other.memory    = nullptr;
    other.allocator = {};
}

compeval::Cpu &compeval::Cpu::operator=(Cpu&& other)
{
    if (this == &other) return *this;

    memory     = other.memory;
    stack_top  = other.stack_top;
    stack_base = other.stack_base;
    allocator  = other.allocator;
    std::memcpy(registers, other.registers, sizeof(registers[0])*Reg::COUNT);

    other.memory    = nullptr;
    other.allocator = {};
    return *this;
}
