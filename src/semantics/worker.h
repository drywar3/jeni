#pragma once

#include "semantics/entities/symbol.h"

struct SemanticContext;
struct WorkerStatus {
    enum V {
        /* the statement/expression was able to be processed completely */
        Done,
        /* the statement/expression was not able to be processed but did not fail */
        Pending,
        /* the statement/expression was not able to be processed due to an error */
        Failed,
    } kind;

    WorkerStatus() : kind((V)0), waiting_on(MINI_ARRAY_INIT(mini_default_allocator(), sema::SymbolId)) {}
    WorkerStatus(V v) : kind(v), waiting_on(MINI_ARRAY_INIT(mini_default_allocator(), sema::SymbolId)) {}

    operator V() const { return kind; }

    MINI_ARRAY(sema::SymbolId) waiting_on;
    bool is_handled = false;

    ~WorkerStatus() {
        mini_array_destroy(waiting_on);
    }

    WorkerStatus(const WorkerStatus& other)
        : kind(other.kind),
          waiting_on(MINI_ARRAY_INIT(mini_default_allocator(), sema::SymbolId))
    {
        mini_array_copy(waiting_on, other.waiting_on);
    }

    WorkerStatus& operator=(const WorkerStatus& other) {
        if (this != &other) {
            kind = other.kind;
            mini_array_clear(waiting_on);
            mini_array_copy(waiting_on, other.waiting_on);
        }
        return *this;
    }

    WorkerStatus(WorkerStatus&& other) noexcept
        : kind(other.kind), waiting_on(other.waiting_on)
    {
        other.waiting_on = MINI_ARRAY_INIT(mini_default_allocator(), sema::SymbolId);
    }

    WorkerStatus& operator=(WorkerStatus&& other) noexcept {
        if (this != &other) {
            mini_array_destroy(waiting_on);

            kind = other.kind;
            waiting_on = other.waiting_on;

            other.waiting_on = MINI_ARRAY_INIT(mini_default_allocator(), sema::SymbolId);
        }
        return *this;
    }

    WorkerStatus &wait_for(sema::SymbolId id) {
        mini_array_append(waiting_on, id);
        return *this;
    }

    void handled(bool value) {
        is_handled = value;
    }
};

typedef WorkerStatus (*WorkerFunc)(SemanticContext *ctx, void *data, bool is_resumption);

struct Worker {
    void *data;
    WorkerFunc func;
};
