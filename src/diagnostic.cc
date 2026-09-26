#include "diagnostic.h"

#include <mini.c/default_allocator.h>

void diag_destroy(void *_diag) {
    Diagnostic *diag = (Diagnostic*)_diag;
    mini_array_destroy(diag->labels);
}

Diagnostic diag_create(DIAG_CTOR) {
    Diagnostic diagnostic{};
    diagnostic.labels     = MINI_ARRAY_INIT(mini_default_allocator(), Label);
    diagnostic.message    = message;
    diagnostic.severity   = severity;
    diagnostic =  diag_add_label(diagnostic, (Label){text, locus, true});
    return diagnostic;
}

Diagnostic diag_add_label(Diagnostic diagnostic, Label label) {
    if (diagnostic.labels) {
        mini_array_append(diagnostic.labels, label);
    }
    return diagnostic;
}

DiagnosticPool diagpool_create() {
    DiagnosticPool pool = {0};
    pool.diagnostics    = MINI_ARRAY_INIT(mini_default_allocator(), Diagnostic);
    mini_array_set_dtor(pool.diagnostics, diag_destroy);
    return pool;
}

void diagpool_report(DiagnosticPool *pool, DIAG_CTOR) {
    Diagnostic diag = diag_create(severity, locus, message, text);
    mini_array_append(pool->diagnostics, diag);
}

void diagpool_destroy(DiagnosticPool *pool) {
    mini_array_destroy(pool->diagnostics);
}

void diagpool_report_diag(DiagnosticPool *pool, Diagnostic diagnostic) {
    mini_array_append(pool->diagnostics, diagnostic);
}

bool diagpool_is_empty(DiagnosticPool *pool) {
    return mini_array_count(pool->diagnostics) == 0;
}

void diag_report(const Diagnostic *diagnostic) {
    for (usize n = 0; n < mini_array_count(diagnostic->labels); n++) {
        Label label = diagnostic->labels[n];
        printf("%s:%zu:%zu: error: ", label.locus.file_path, label.locus.line, label.locus.begin);
        if (label.is_primary)
            printf("%s:", diagnostic->message);
        printf("\n");
        printf("  -> %s\n", label.text);
    }
}
