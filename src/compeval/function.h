#pragma once

#include "compeval/instruction.h"

namespace compeval
{
    using Pc = int64;

    struct Bytecode_Function {
        mini::Array<sema::Symbol_Id>       param_symbols;
        mini::Array<compeval::Instruction> instructions;
    };
} // namespace compeval
