#pragma once

#include "lir/types/type.h"
#include "lir/types/value.h"
#include "mini.cc/array.h"

#include <optional>

namespace lir
{
    struct InstructionKind {
        enum struct OpCode {
            Alloca,
            Store,
            Call,
        };

        OpCode opcode;

        union {
            struct Alloca {
                lir::TypePtr type;
            } alloca;

            struct Store {
                lir::TypePtr type;
                ValueId value;
            } store;

            struct Call {
                lir::TypePtr type;
                ValueId callee;
                mini::Array<ValueId> arguments;
            } call;
        } as;

        static InstructionKind Call(lir::TypePtr type, ValueId callee,
                                    mini::Array<ValueId> args)
        {
            InstructionKind kind{};
            kind.opcode            = OpCode::Call;
            kind.as.call.callee    = callee;
            kind.as.call.arguments = args;
            kind.as.call.type      = type;
            return kind;
        }

        static InstructionKind Alloca(lir::TypePtr type)
        {
            InstructionKind kind{};
            kind.opcode         = OpCode::Alloca;
            kind.as.alloca.type = type;
            return kind;
        }

        static InstructionKind Store(lir::TypePtr type, ValueId value)
        {
            InstructionKind kind{};
            kind.opcode         = OpCode::Store;
            kind.as.store.type  = type;
            kind.as.store.value = value;
            return kind;
        }
    };

    struct Instruction {
        std::optional<ValueId> dst;
        InstructionKind inst;
    };
} // namespace lir
