#pragma once

namespace codegen
{
    struct Context;
    using BackendEntryPoint = Mini_String(*)(Context *context);
    using BackendFinalize   = bool(*)(Mini_String blob, const char *output_name);

    struct Backend {
        BackendEntryPoint entry;
        BackendFinalize   exit;
    };
} // namespace codegen
