#pragma once

#include "lir/types/value.h"

#include <optional>

namespace lir
{
    struct InstructionKind {
        enum struct OpCode {
            Alloca,
            Store,
        };

        OpCode opcode;

        union {
            // type l_index;
            struct Alloca {
                lir::TypePtr type;
            } alloca;

            struct Store {
                lir::TypePtr type;
                ValueId      value;
            } store;
        } as;

        static InstructionKind Alloca(lir::TypePtr type) {
            InstructionKind kind;
            kind.opcode = OpCode::Alloca;
            kind.as.alloca.type = type;
            return kind;
        }

        static InstructionKind Store(lir::TypePtr type, ValueId value) {
            InstructionKind kind;
            kind.opcode = OpCode::Store;
            kind.as.store.type  = type;
            kind.as.store.value = value;
            return kind;
        }
    };

    struct Instruction {
        std::optional<ValueId> dst;
        InstructionKind        inst;
    };
} // namespace lir
