#pragma once

#include "lir/types/type.h"
#include "misc/map.h"
// #include "lir/types/buildr.h"
#include "lir/types/instruction.h"

#include <mini.cc/array.h>
#include <mini.cc/string_view.h>

namespace lir
{
    struct Local {
        mini::StringView name;
        usize index;
        bool is_parameter;
        // lir::TypePtr     type;
    };

    enum struct BlockId : usize {};
    struct Block {
        using LocalMap = HashMap<mini::StringView, Local>;

        std::optional<BlockId> parent;
        LocalMap locals;
    };
    using BlockStorage = mini::Array<Block>;

    struct Buildr;
    struct Module;

    struct Function {
        struct Prototype {
            mini::Array<TypePtr> parameters;
            TypePtr return_type;
        };

        mini::StringView name;
        Prototype prototype;
        BlockId block;
        mini::Array<Instruction> instructions;
        usize current_local_index;
        bool body_is_defined;

        Buildr buildr(Module *mod);
        void add_instruction(Instruction inst);
    };

    Function function_init(lir::Buildr *b, mini::StringView name,
                           TypePtr return_type);
} // namespace lir
