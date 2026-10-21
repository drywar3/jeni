#pragma once

#include "ast/expr.h"
#include "ast/stmt.h"
#include "semantics/store.h"
#include "semantics/entities/symbol.h"

struct Semantic_Context;
struct Worker_Status {
    enum V {
        /* the statement/expression was able to be processed completely */
        Done,
        /* the statement/expression was not able to be processed but did not
           fail */
        Pending,
        /* the statement/expression was not able to be processed due to an error
         */
        Failed,
    } kind;

    Worker_Status()
        : kind((V)0),
          waiting_on(MINI_ARRAY_INIT(mini_default_allocator(), sema::Symbol_Id))
    {
    }
    Worker_Status(V v)
        : kind(v),
          waiting_on(MINI_ARRAY_INIT(mini_default_allocator(), sema::Symbol_Id))
    {
    }

    operator V() const { return kind; }

    MINI_ARRAY(sema::Symbol_Id) waiting_on;
    Opt<sema::Scope_Id> working_scope;

    ~Worker_Status() { mini_array_destroy(waiting_on); }

    Worker_Status(const Worker_Status &other)
        : kind(other.kind),
          waiting_on(MINI_ARRAY_INIT(mini_default_allocator(), sema::Symbol_Id))
    {
        mini_array_copy(waiting_on, other.waiting_on);
    }

    Worker_Status &operator=(const Worker_Status &other)
    {
        if (this != &other) {
            kind = other.kind;
            mini_array_clear(waiting_on);
            mini_array_copy(waiting_on, other.waiting_on);
        }
        return *this;
    }

    Worker_Status(Worker_Status &&other) noexcept
        : kind(other.kind), waiting_on(other.waiting_on)
    {
        other.waiting_on =
            MINI_ARRAY_INIT(mini_default_allocator(), sema::Symbol_Id);
    }

    Worker_Status &operator=(Worker_Status &&other) noexcept
    {
        if (this != &other) {
            mini_array_destroy(waiting_on);

            kind       = other.kind;
            waiting_on = other.waiting_on;

            other.waiting_on =
                MINI_ARRAY_INIT(mini_default_allocator(), sema::Symbol_Id);
        }
        return *this;
    }

    Worker_Status &wait_for(sema::Symbol_Id id)
    {
        mini_array_append(waiting_on, id);
        return *this;
    }

    bool is_done() const { return kind == V::Done; }

    void at_scope(sema::Scope_Id scope) { working_scope = scope; }

    bool is_failed() const { return kind == V::Failed; }
};

struct Worker_Payload {
    enum Kind {
        Stmt,
        Expr,
    };

    Kind kind;
    Statement  **stmt;
    Expression **expr;

    Worker_Payload(Statement *&stmt)  : kind(Stmt), stmt(&stmt) {}
    Worker_Payload(Expression *&expr) : kind(Expr), expr(&expr) {}

    Statement *&as_stmt()  { return *stmt; }
    Expression *&as_expr() { return *expr; }
};

typedef Worker_Status (*Worker_Func)(Semantic_Context *ctx, Worker_Payload data,
                                     bool is_resumption);

struct Worker {
    Worker_Payload data;
    Worker_Func func;
    sema::Scope_Id scope_id;
    sema::Semantic_Id sema_id;

    Worker(Worker_Payload data, Worker_Func func, sema::Scope_Id current_scope, sema::Semantic_Id sema_id);

    Worker_Status resume(Semantic_Storage *store);
};
