#include "lir/lir.h"
#include "lir/types/buildr.h"
#include "lir/types/function.h"

lir::Buildr lir::Function::buildr(Module *mod)
{
    return lir::Buildr{mod, this, .current_block = block };
}

lir::Function lir::function_init(lir::Buildr *b, mini::StringView name, TypePtr return_type)
{
    auto allocator = b->allocator();

    lir::Function function {
        .instructions = mini::Array<Instruction>(allocator),
        .prototype    = {
            .parameters  = mini::Array<TypePtr>(allocator),
            .return_type = return_type,
        },
        .current_local_index = 0,
        .block = b->mod->context->new_block(std::nullopt),
    };
    return function;
}

void lir::Function::add_instruction(Instruction inst)
{
    instructions.append(inst);
}
