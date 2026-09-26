#include "source.h"
#include "diagnostic.h"

#include <cstdio>
#include <cstdlib>

constexpr usize CONTEXT_LINES = 2;
constexpr usize MERGE_WINDOW  = 5;

static mini::StringView source_file_get_line_text(const SourceFile *file, usize line_idx) {
    if (line_idx >= mini_array_count(file->line_starts)) {
        return mini::StringView{"", 0};
    }

    usize start = file->line_starts[line_idx];
    /* todo: account for \r\n line endings on platforms like windows */
    usize end = (line_idx + 1 < mini_array_count(file->line_starts))
                ? file->line_starts[line_idx + 1] - 1  // skip trailing '\n'
                : mini_string_count(file->content);

    return mini::StringView{&file->content[start], end - start};
}

static const char *severity_prefix(Severity sev) {
    switch (sev) {
        case DIAG_Error:   return "error";
        case DIAG_Warning: return "warning";
        case DIAG_Note:    return "note";
    }
    return "   DIAG";
}

static int label_compar(const void *_lbl1, const void *_lbl2) {
    const Label *lbl1 = (const Label*)_lbl1;
    const Label *lbl2 = (const Label*)_lbl2;
    return lbl1->locus.line - lbl2->locus.line;
}

void underline_locus(Locus locus, char head = '~', char tail = '~') {
    usize n = 0;
    while (n < locus.begin - 1) {
        printf(" ");
        n++;
    }
    printf("%c", head);
    n++;
    if (locus_length(&locus) > 0) {
        while (n < locus.end - 1) {
            printf("%c", tail);
            n++;
        }
    }
}

void single_render_label(const Label *label, const SourceManager *sm, bool render_context = true) {
    Locus locus = label->locus;
    const SourceFile *source_file = sourcemgr_get_source(sm, locus.source_id);

    /* fixme: if [locus.line == CONTEXT_LINES] integer overflow (to SIZE_MAX)
     * will occur on the first iteration */
    if (render_context && locus.line != 1) {
        /* todo: iterate the lines directly (instead of from 0 -> [CONTEXT_LINES]) */
        for (usize n = 0; n < CONTEXT_LINES; ++n) {
            usize line_num = (locus.line - CONTEXT_LINES) + n;
            mini::StringView line_text = source_file_get_line_text(source_file, line_num - 1);
            printf("  %zu | %.*s\n", line_num, SVARG(line_text.base()));
        }
    }

    mini::StringView line_text = source_file_get_line_text(source_file, locus.line - 1);
    printf("  %zu | %.*s\n", locus.line, SVARG(line_text.base()));
    printf("  %s  | ", "");
    if (label->is_primary) {
        printf("\033[0;31m");
        underline_locus(locus);
        printf(" \033[0m>\033[1;31m %s\n", label->text);
    } else {
        printf("\033[0;33m");
        underline_locus(locus, '^', '-');
        printf(" > %s\n", label->text);
    }
    printf("\033[0m");
}

void merge_render_labels(const Label *labels, const SourceManager *sm) {
    const usize label_count = mini_array_count((void*)labels);
    for (usize n = 0; n < label_count; ++n) {
        Label label = labels[n];
        single_render_label(&label, sm, false);
        if (n + 1 < label_count) {
            Label next_label = labels[n + 1];
            if (next_label.locus.source_id == label.locus.source_id) {
                const SourceFile *source_file = sourcemgr_get_source(sm, label.locus.source_id);
                usize line_difference = next_label.locus.line - label.locus.line;
                for (usize n = 1; n < line_difference; ++n) {
                    usize line_num = label.locus.line + n;
                    mini::StringView line_text = source_file_get_line_text(source_file, line_num - 1);
                    printf("  %zu | %.*s\n", line_num, SVARG(line_text.base()));
                }
            }
        }
    }
}

void diag_report(const Diagnostic *diagnostic, const SourceManager *sm) {
    if (mini_array_count(diagnostic->labels) == 0) return;

    Label primary_label = diagnostic->labels[0];
    const SourceFile *source_file = sourcemgr_get_source(sm, primary_label.locus.source_id);

    /* sort labels */
    Label *labels = diagnostic->labels;
    const usize label_count = mini_array_count(labels);

    std::qsort(labels, label_count, sizeof(Label), label_compar);

    printf("%s: %s:%zu:%zu: %s\n",
           severity_prefix(diagnostic->severity),
           source_file->path,
           primary_label.locus.line,
           primary_label.locus.begin,
           diagnostic->message);

    printf("  %s  |\n", "");
    MINI_ARRAY(Label) merge_render_group = MINI_ARRAY_INIT(mini_default_allocator(), Label);
    bool is_grouping_label = false;
    usize last_label_line  = 0;

    /* todo: handle multiple labels on the same line */
    for (usize n = 0; n < label_count; ++n) {
        Label label = labels[n];
        if (n + 1 < label_count) {
            Label next_label = labels[n + 1];
            usize line_difference = next_label.locus.line - label.locus.line;
            if (line_difference < MERGE_WINDOW) {
                mini_array_append(merge_render_group, label);
                if (!is_grouping_label) {
                    is_grouping_label = true;
                }
            } else {
                if (is_grouping_label) {
                    mini_array_append(merge_render_group, next_label);
                    merge_render_labels(merge_render_group, sm);
                    is_grouping_label = false;
                    mini_array_clear(merge_render_group);
                    n += 1;
                } else {
                    if (n != 0)
                        printf("   ...   \n");
                    single_render_label(&label, sm);
                }
            }
        } else {
            if (is_grouping_label) {
                usize line_difference = label.locus.line - last_label_line;
                if (line_difference < MERGE_WINDOW) {
                    mini_array_append(merge_render_group, label);
                    merge_render_labels(merge_render_group, sm);
                    is_grouping_label = false;
                    mini_array_clear(merge_render_group);
                } else {
                    single_render_label(&label, sm);
                }
            } else {
                if (n != 0)
                    printf("   ...   \n");
                single_render_label(&label, sm);
            }
        }
        last_label_line = label.locus.line;
    }
    printf("  %s  |\n", "");

    /* end diagnostic */
    printf("\n");

    mini_array_destroy(merge_render_group);
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
