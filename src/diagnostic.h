#pragma once

#include "parser/locus.h"
#include "source.h"

#include <mini.c/array.h>

struct Label {
    const char *text;
    Locus locus;
    bool is_primary = false;
};

enum struct Severity {
    Error,
    Warning,
    Note,
};

typedef struct {
    Severity severity;
    const char *message;
    MINI_ARRAY(Label) labels;
} Diagnostic;

typedef struct {
    MINI_ARRAY(Diagnostic) diagnostics;
} Diagnostic_Pool;

#define DIAG_CTOR                                                              \
    Severity severity, Locus locus, const char *message, const char *text

Diagnostic diag_create(DIAG_CTOR);
Diagnostic diag_add_label(Diagnostic diagnostic, Label label);

void diag_report(const Diagnostic *diagnostic, const SourceManager *sm);

Diagnostic_Pool diagpool_create();
void diagpool_report(Diagnostic_Pool *pool, DIAG_CTOR);
void diagpool_report_diag(Diagnostic_Pool *pool, Diagnostic diagnostic);

bool diagpool_is_empty(Diagnostic_Pool *pool);

void diagpool_destroy(Diagnostic_Pool *pool);
