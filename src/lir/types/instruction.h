#pragma once

#include "lir/types/type.h"
#include "lir/types/value.h"
#include "mini.cc/array.h"

#include <optional>

namespace lir
{
    enum struct Label : int {};
    struct InstructionKind {
        enum struct OpCode {
            Alloca,
            Store,
            Call,
            PutLabel,
            JmpIfEquals,
            Jmp,
            Ret,
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

            Label label;

            struct JmpIfEquals {
                ValueId first;
                ValueId second;
                Label   label;
            } jmp_if_eq;

            struct Ret {
                ValueId value;
                lir::TypePtr type;
            } ret;
        } as;

        static InstructionKind Ret(lir::TypePtr type, ValueId value)
        {
            InstructionKind kind{};
            kind.opcode       = OpCode::Ret;
            kind.as.ret.value = value;
            kind.as.ret.type  = type;
            return kind;
        }

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

        static InstructionKind PutLabel(Label label)
        {
            InstructionKind kind{};
            kind.opcode         = OpCode::PutLabel;
            kind.as.label       = label;
            return kind;
        }

        static InstructionKind Jmp(Label label)
        {
            InstructionKind kind{};
            kind.opcode         = OpCode::Jmp;
            kind.as.label       = label;
            return kind;
        }

        static InstructionKind JmpIfEquals(Label label, ValueId first, ValueId second)
        {
            InstructionKind kind{};
            kind.opcode              = OpCode::JmpIfEquals;
            kind.as.jmp_if_eq.first  = first;
            kind.as.jmp_if_eq.second = second;
            kind.as.jmp_if_eq.label  = label;
            return kind;
        }
    };

    struct Instruction {
        std::optional<ValueId> dst;
        InstructionKind inst;
    };
} // namespace lir
