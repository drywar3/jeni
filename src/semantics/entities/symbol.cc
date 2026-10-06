#include "semantics/entities/symbol.h"

const char *state_name(sema::SymbolState s)
{
    switch (s) {
    case sema::SymbolState::Resolved: return "resolved";
    case sema::SymbolState::Unresolved: return "unresolved";
    case sema::SymbolState::Failed: return "failed";
    case sema::SymbolState::Resolving: return "resolving";
    }
}

bool sema::Symbol::is_state(SymbolState state) const
{
    return resolve_state == state;
}

void sema::Symbol::set_state(SymbolState state)
{
    //printf("(%.*s) %s => %s\n", SVARG(name.base()), state_name(resolve_state), state_name(state));
    resolve_state = state;
}
