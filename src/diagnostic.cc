#include "diagnostic.h"
#include "source.h"
#include <cstdio>

static mini::StringView source_file_get_line_text(const SourceFile *file, usize line_idx) {
    if (line_idx >= mini_array_count(file->line_starts)) {
        return mini::StringView{"", 0};
    }

    usize start = file->line_starts[line_idx];
    usize end = (line_idx + 1 < mini_array_count(file->line_starts))
                ? file->line_starts[line_idx + 1] - 1  // skip trailing '\n'
                : mini_string_count(file->content);

    return mini::StringView{&file->content[start], end - start};
}

static const char *severity_prefix(Severity sev) {
    switch (sev) {
        case DIAG_Error:   return "   ERROR";
        case DIAG_Warning: return "   WARNING";
        case DIAG_Note:    return "   NOTE";
    }
    return "   DIAG";
}

void diag_report(const Diagnostic *diagnostic, const SourceManager *sm) {
    if (mini_array_count(diagnostic->labels) == 0) return;

    constexpr usize CONTEXT_LINES = 2;

    Label primary_label = diagnostic->labels[0];
    const SourceFile *file = sourcemgr_get_source(sm, primary_label.locus.source_id);

    printf("%s [%s:%zu:%zu] %s\n",
           severity_prefix(diagnostic->severity),
           file->path,
           primary_label.locus.line,
           primary_label.locus.begin,
           diagnostic->message);

    for (usize n = 0; n < mini_array_count(diagnostic->labels); n++) {
        Label label = diagnostic->labels[n];
        const SourceFile *src = sourcemgr_get_source(sm, label.locus.source_id);

        usize target_line_idx = label.locus.line > 0 ? label.locus.line - 1 : 0;
        usize start_line_idx = (target_line_idx >= CONTEXT_LINES)
                               ? target_line_idx - CONTEXT_LINES
                               : 0;

        printf("      │\n");

        for (usize l_idx = start_line_idx; l_idx < target_line_idx; l_idx++) {
            mini::StringView ctx_line = source_file_get_line_text(src, l_idx);
            printf("%5zu │   %.*s\n",
                   l_idx + 1,
                   (int)ctx_line.base().length,
                   ctx_line.base().data);
        }

        mini::StringView line_str = source_file_get_line_text(src, target_line_idx);
        printf("%5zu │   %.*s\n",
               target_line_idx + 1,
               (int)line_str.base().length,
               line_str.base().data);

        printf("      │   ");

        usize col_start = label.locus.begin > 0 ? label.locus.begin - 1 : 0;
        for (usize i = 0; i < col_start; i++) {
            putchar(' ');
        }

        usize span_len = (label.locus.end > label.locus.begin)
                         ? (label.locus.end - label.locus.begin)
                         : 1;

        usize mid = span_len / 2;
        for (usize i = 0; i < span_len; i++) {
            if (i == mid) putchar('^');
            else putchar('~');
        }

        if (label.text && label.text[0] != '\0') {
            printf(" %s", label.text);
        } else if (label.is_primary && diagnostic->message) {
            printf(" %s", diagnostic->message);
        }
        printf("\n");
    }
    printf("      │\n\n");
}

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
