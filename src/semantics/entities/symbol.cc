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
    resolve_state = state;
}

bool sema::Symbol::is(Symbol_Kind kind) const
{
    return this->kind == kind;
}
