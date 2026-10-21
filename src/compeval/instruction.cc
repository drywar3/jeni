#include "compeval/instruction.h"

compeval::Instruction::Operand::Operand(::compeval::Value value)
    : kind(Kind::Value), value(value)
{

}

compeval::Instruction::Operand::Operand(int   reg)
    : kind(Kind::Register), reg(reg)
{
}


compeval::Instruction::Operand::Operand(sema::Symbol_Id id)
    : kind(Kind::Symbol), symbol_id(id)
{
}

compeval::Instruction::Instruction(Opcode opcode,
            Opt<Operand> arg1,
            Opt<Operand> arg2,
            Opt<Operand> arg3)
    : opcode(opcode), arg1(arg1), arg2(arg2), arg3(arg3)
{
}

const char *compeval::Instruction::name() const
{
    switch (opcode) {
    case Opcode::Add: return "ADD";
    case Opcode::Ret: return "RET";
    case Opcode::Call: return "CALL";
    case Opcode::Param: return "PARAM";
    case Opcode::Nop: return "NOP";
    case Opcode::Push: return "PUSH";
    case Opcode::Load_Symbol: return "LOAD_SYMBOL";
    default: MINI_UNREACHABLE("%d", opcode);
    }
}
