#pragma once

#include "misc/id.h"

#include <mini.c/string_view.h>
#include <optional>

CREATE_ID(SymbolId);
CREATE_ID(ScopeId);

enum struct SymbolKind
{
    Variable,
    Function,
    Type,
    Namespace,
};

struct SymbolVariable {
    bool            is_initialized;
};

struct Symbol {
    SymbolKind      kind;
    Mini_StringView name;
    ScopeId         scope_id;

    union {
        SymbolVariable variable;
    } as;
};

typedef Symbol *SymbolPointer;
using SymbolStorage = DenseMap<Locus, Symbol>;
