#pragma once

#include "lir/lir.h"
#include "codegen/backend.h"

#include <mini.c/string.h>

namespace codegen
{
    struct Context {
        const lir::Module *mod;
        Backend            backend;
        Mini_Allocator     allocator;
    };

    Context ctx_init(const lir::Module *mod,
                     Mini_Allocator allocator,
                     Backend backend);
    bool ctx_generate(Context *context, const char *output_name);
} // namespace codegen
