#include "semantics/entities/symbol.h"

const char *state_name(sema::Symbol_State s)
{
    switch (s) {
    case sema::Symbol_State::Resolved:
        return "resolved";
    case sema::Symbol_State::Unresolved:
        return "unresolved";
    case sema::Symbol_State::Failed:
        return "failed";
    case sema::Symbol_State::Resolving:
        return "resolving";
    }
}

bool sema::Symbol::is_state(Symbol_State state) const
{
    return resolve_state == state;
}

void sema::Symbol::set_state(Symbol_State state)
{
    // printf("(%.*s) %s => %s\n", SVARG(name.base()),
    // state_name(resolve_state), state_name(state));
    resolve_state = state;
}
