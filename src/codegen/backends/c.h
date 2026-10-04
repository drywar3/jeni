#pragma once

#include "codegen/codegen.h"

namespace codegen
{
    Mini_String c_backend_entry_point(Context *context);
    bool c_backend_finalize(Mini_String block, const char *output_name);
} // namespace codegen::backend
