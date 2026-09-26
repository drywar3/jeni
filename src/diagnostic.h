#pragma once

#include "source.h"
#include "parser/locus.h"

#include <mini.c/array.h>

struct Label {
    const char *text;
    Locus locus;
    bool is_primary = false;
};

typedef enum {
    DIAG_Error,
    DIAG_Warning,
    DIAG_Note,
} Severity;

typedef struct {
    Severity    severity;
    const char *message;
    MINI_ARRAY(Label) labels;
} Diagnostic;

typedef struct {
    MINI_ARRAY(Diagnostic) diagnostics;
} DiagnosticPool;

#define DIAG_CTOR Severity severity, Locus locus, const char *message, const char *text

Diagnostic diag_create(DIAG_CTOR);
Diagnostic diag_add_label(Diagnostic diagnostic, Label label);

void diag_report(const Diagnostic *diagnostic, const SourceManager *sm);

DiagnosticPool diagpool_create();
void diagpool_report(DiagnosticPool *pool, DIAG_CTOR);
void diagpool_report_diag(DiagnosticPool *pool, Diagnostic diagnostic);

bool diagpool_is_empty(DiagnosticPool *pool);

void diagpool_destroy(DiagnosticPool *pool);
