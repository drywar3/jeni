#pragma once

#include "misc/map.h"
#include "lir/types/type.h"
#include "hir/types/profile.h"
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

        hir::Profile profile;
        mini::StringView name;
        Prototype prototype;
        BlockId block;
        mini::Array<Instruction> instructions;
        usize current_local_index;
        usize current_label_index;
        bool body_is_defined;
        bool is_variadic;

        Buildr buildr(Module *mod);
        void add_instruction(Instruction inst);
    };

    Function function_init(lir::Buildr *b, mini::StringView name,
                           TypePtr return_type, bool is_variadic,
                           hir::Profile profile);
} // namespace lir
