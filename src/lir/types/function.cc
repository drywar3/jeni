#include "lir/lir.h"
#include "lir/types/buildr.h"
#include "lir/types/function.h"

lir::Buildr lir::Function::buildr(Module *mod)
{
    return lir::Buildr{mod, this, .current_block = block };
}

lir::Function lir::function_init(lir::Buildr *b, mini::StringView name, TypePtr return_type, bool is_variadic)
{
    auto allocator = b->allocator();

    lir::Function function {
        .name  = name,
        .prototype    = {
            .parameters  = mini::Array<TypePtr>(allocator),
            .return_type = return_type,
        },
        .block = b->mod->context->new_block(std::nullopt),
        .instructions = mini::Array<Instruction>(allocator),
        .current_local_index = 0,
        .is_variadic = is_variadic,
    };
    return function;
}

void lir::Function::add_instruction(Instruction inst)
{
    instructions.append(inst);
}
