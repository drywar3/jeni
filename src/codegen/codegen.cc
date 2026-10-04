#include "codegen/codegen.h"

codegen::Context codegen::ctx_init(const lir::Module *mod,
                                   Mini_Allocator allocator,
                                   Backend backend)
{
    codegen::Context context { .mod = mod, .backend = backend };
    context.allocator = allocator;
    return context;
}

bool codegen::ctx_generate(Context *context, const char *output_name)
{
    Mini_String blob = context->backend.entry(context);
    if (context->backend.exit)
        return context->backend.exit(blob, output_name);
    return true;
}
