#pragma once

#include "lir/lir.h"

namespace lir
{
    static inline void print(const lir::Module *mod, const lir::TypePtr type)
    {
        switch (type->kind) {
        case lir::Type::Int32:  printf("int32");  break;
        case lir::Type::String: printf("string"); break;
        case lir::Type::Void:   printf("void");   break;
        default: MINI_UNREACHABLE();
        }
    }

    static inline void print(const lir::Module *mod, const lir::ValueId value_id)
    {
        const lir::Value &value = mod->context->values[usize(value_id)];
        switch (value.kind) {
        case lir::Value::Kind::Integer:
            printf("%ld", value.integer);
            break;
        case lir::Value::Kind::GlobalRef:
            printf("%.*s", SVARG(value.ident.base()));
            break;
        case lir::Value::Kind::LocalRef:
            printf("local_%zu", value.index);
            break;
        case lir::Value::Kind::ParamRef:
            printf("param_%zu", value.index);
            break;
        case lir::Value::Kind::Deref:
            printf("("); print(mod, value.valueid); printf(").*");
            break;
        default: MINI_UNREACHABLE();
        }
    }

    static inline void print(const lir::Module *mod, const lir::Instruction *inst)
    {
        if (inst->dst) {
            print(mod, *inst->dst);
            printf(" <- ");
        }

        switch (inst->inst.opcode) {
        case lir::InstructionKind::OpCode::Alloca:
            printf("alloca "); print(mod, inst->inst.as.alloca.type);
            break;
        case lir::InstructionKind::OpCode::Store:
            printf("store "); print(mod, inst->inst.as.store.type); printf(" "); print(mod, inst->inst.as.store.value);
            break;
        default: MINI_UNREACHABLE();
        }
    }

    static inline void print(const lir::Module *mod, const lir::Global *global)
    {
        switch (global->kind) {
        case lir::Global::Kind::Variable: {
            printf("%.*s : ", SVARG(global->name.base()));
            print(mod, global->variable.type);
            printf(" = "); print(mod, global->variable.initializer); printf(";\n");
        } break;
        case lir::Global::Kind::Function: {
            printf("func %.*s(\n", SVARG(global->function.name.base()));
            for (auto &p : global->function.prototype.parameters.iter()) {
                printf("    ");
                print(mod, p);
                printf("\n");
            }
            printf(") "); print(mod, global->function.prototype.return_type); printf(":\n");
            for (auto &inst : global->function.instructions.iter()) {
                printf("    ");
                print(mod, &inst);
                printf("\n");
            }
            printf("end\n");
        } break;
        default: MINI_UNREACHABLE();
        }
    }
} // namespace lir
