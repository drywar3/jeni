#pragma once

typedef struct SemanticContext SemanticContext;

typedef enum WorkerStatus {
    /* the statement/expression was able to be processed completely */
    WORKER_Done,
    /* the statement/expression was not able to be processed but did not fail */
    WORKER_Pending,
    /* the statement/expression was not able to be processed due to an error */
    WORKER_Failed,
} WorkerStatus;

typedef WorkerStatus (*WorkerFunc)(SemanticContext *ctx, void *data);

typedef struct Worker {
    void *data;
    WorkerFunc func;
} Worker;
