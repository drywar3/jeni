#pragma once

struct SemanticContext;

enum struct WorkerStatus {
    /* the statement/expression was able to be processed completely */
    Done,
    /* the statement/expression was not able to be processed but did not fail */
    Pending,
    /* the statement/expression was not able to be processed due to an error */
    Failed,
};

typedef WorkerStatus (*WorkerFunc)(SemanticContext *ctx, void *data);

struct Worker {
    void *data;
    WorkerFunc func;
};
