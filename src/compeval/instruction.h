#pragma once

#include "misc/misc.h"
#include "compeval/value.h"
#include "ast/expressions.h"
#include "semantics/entities/symbol.h"

namespace compeval
{
    struct Instruction {
        enum Opcode {
            Add = (int)ast::Operator::Add,
            Sub = (int)ast::Operator::Sub,
            Mul = (int)ast::Operator::Mul,
            Div = (int)ast::Operator::Div,

            Nop,
            Mov,
            Push,
            Pop,
            Call,
            Param,
            JmpIfTrue,
            Ret,
            Load_Symbol,
            Store,
        };

        struct Operand {
            enum Kind {
                Value,
                Register,
                Symbol,
            };

            Kind kind;

            ::compeval::Value value;
            int   reg;
            sema::Symbol_Id symbol_id;

            Operand(::compeval::Value value);
            Operand(int   reg);
            Operand(sema::Symbol_Id   id);
        };

        Opcode opcode;
        Opt<Operand> arg1;
        Opt<Operand> arg2;
        Opt<Operand> arg3;

        Instruction(Opcode opcode,
                    Opt<Operand> arg1 = None,
                    Opt<Operand> arg2 = None,
                    Opt<Operand> arg3 = None);

        const char *name() const;
    };
} // namespace compeval
