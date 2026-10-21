#pragma once

#include "misc/misc.h"

#include <mini.cc/array.h>

namespace ast {
    enum struct Calling_Convention {
        Default,
        C,
        System,
    };

    struct Attribute {
        enum struct Kind {
            Extern,
            No_Context,
            Inline,
            No_Inline,
            Macro,
            Canon_Name,
        };

        struct Extern {
            Calling_Convention abi = Calling_Convention::Default;
            Opt<mini::StringView> export_name;
        };

        Kind kind;
        union {
            Extern            extern_;
            mini::StringView canon_name;
        };
    };

    using Attributes = mini::Array<ast::Attribute>;
}
