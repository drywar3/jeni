#include "diagnostic.h"
#include "source.h"

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#define MINI_ISATTY(f) _isatty(_fileno(f))
#else
#include <unistd.h>
#define MINI_ISATTY(f) isatty(fileno(f))
#endif

constexpr usize CONTEXT_LINES = 2;
constexpr usize MERGE_WINDOW  = 5;

/* ------------------------------------------------------------------ */
/* colour helpers                                                      */
/* ------------------------------------------------------------------ */

static bool use_color()
{
    static int cached = -1;
    if (cached < 0) {
        const char *no_color = std::getenv("NO_COLOR");
        bool disabled        = no_color && *no_color;
        cached               = (!disabled && MINI_ISATTY(stdout)) ? 1 : 0;
    }
    return cached == 1;
}

static const char *paint(const char *code) { return use_color() ? code : ""; }

#define C_RESET "\033[0m"
#define C_RED "\033[0;31m"
#define C_YELLOW "\033[0;33m"
#define C_BRED "\033[1;31m"
#define C_BYEL "\033[1;33m"
#define C_BCYAN "\033[1;36m"

/* ------------------------------------------------------------------ */
/* source access                                                       */
/* ------------------------------------------------------------------ */

struct LineSpan {
    const char *ptr;
    usize len;
};

/* Returns the text of a line (0-based index) without its line terminator.
 * Handles both "\n" and "\r\n" endings and never indexes out of range. */
static LineSpan source_file_get_line(const SourceFile *file, usize line_idx)
{
    const usize line_count = mini_array_count(file->line_starts);
    if (line_idx >= line_count) {
        return LineSpan{"", 0};
    }

    const usize total = mini_string_count(file->content);
    const usize start = file->line_starts[line_idx];
    if (start >= total) {
        return LineSpan{"", 0};
    }

    usize end =
        (line_idx + 1 < line_count) ? file->line_starts[line_idx + 1] : total;
    if (end > total) {
        end = total;
    }
    if (end > start && file->content[end - 1] == '\n') {
        --end;
    }
    if (end > start && file->content[end - 1] == '\r') {
        --end;
    }
    return LineSpan{&file->content[start], end - start};
}

static void print_source_line(const SourceFile *file, usize line_num,
                              usize gutter)
{
    LineSpan line = source_file_get_line(file, line_num - 1);
    printf(" %*zu | %.*s\n", (int)gutter, line_num, (int)line.len, line.ptr);
}

/* ------------------------------------------------------------------ */
/* severity                                                            */
/* ------------------------------------------------------------------ */

static const char *severity_prefix(Severity sev)
{
    switch (sev) {
    case Severity::Error:
        return "error";
    case Severity::Warning:
        return "warning";
    case Severity::Note:
        return "note";
    }
    return "diagnostic";
}

static const char *severity_color(Severity sev)
{
    switch (sev) {
    case Severity::Error:
        return C_BRED;
    case Severity::Warning:
        return C_BYEL;
    case Severity::Note:
        return C_BCYAN;
    }
    return "";
}

/* ------------------------------------------------------------------ */
/* label ordering                                                      */
/* ------------------------------------------------------------------ */

/* Orders by file, then line, then column, then end column; primary first
 * on a full tie. Sorting by source first keeps multi-file diagnostics from
 * interleaving. */
static int label_compar(const void *_lbl1, const void *_lbl2)
{
    const Label *a = (const Label *)_lbl1;
    const Label *b = (const Label *)_lbl2;

    if (a->locus.source_id != b->locus.source_id) {
        return a->locus.source_id < b->locus.source_id ? -1 : 1;
    }
    if (a->locus.line != b->locus.line) {
        return a->locus.line < b->locus.line ? -1 : 1;
    }
    if (a->locus.begin != b->locus.begin) {
        return a->locus.begin < b->locus.begin ? -1 : 1;
    }
    if (a->locus.end != b->locus.end) {
        return a->locus.end < b->locus.end ? -1 : 1;
    }
    if (a->is_primary != b->is_primary) {
        return a->is_primary ? -1 : 1;
    }
    return 0;
}

static bool labels_share_group(const Label &a, const Label &b)
{
    return a.locus.source_id == b.locus.source_id &&
           b.locus.line >= a.locus.line &&
           (b.locus.line - a.locus.line) < MERGE_WINDOW;
}

/* ------------------------------------------------------------------ */
/* rendering                                                           */
/* ------------------------------------------------------------------ */

/* Prints the marker line: leading padding (tabs are copied from the source
 * line so the markers stay aligned), then head + tail characters. */
static void underline_locus(Locus locus, LineSpan line, char head, char tail)
{
    const usize start = locus.begin > 0 ? locus.begin - 1 : 0;

    for (usize i = 0; i < start; ++i) {
        putchar((i < line.len && line.ptr[i] == '\t') ? '\t' : ' ');
    }
    putchar(head);

    const usize length = locus_length(&locus);
    usize tails        = length > 0 ? length - 1 : 0;
    /* don't run past the end of the line for multi-line spans */
    if (start < line.len && start + 1 + tails > line.len) {
        tails = line.len - start - 1;
    }
    for (usize i = 0; i < tails; ++i) {
        putchar(tail);
    }
}

static void render_label(const Label *label, const SourceFile *file,
                         usize gutter, bool render_context,
                         bool render_line_text)
{
    const Locus locus    = label->locus;
    const usize line_num = locus.line > 0 ? locus.line : 1;

    if (render_context && line_num > 1) {
        usize first =
            (line_num > CONTEXT_LINES) ? (line_num - CONTEXT_LINES) : 1;
        for (usize n = first; n < line_num; ++n) {
            print_source_line(file, n, gutter);
        }
    }

    if (render_line_text) {
        print_source_line(file, line_num, gutter);
    }

    const LineSpan line = source_file_get_line(file, line_num - 1);
    const bool has_text = label->text && *label->text;

    printf(" %*s | ", (int)gutter, "");
    if (label->is_primary) {
        printf("%s", paint(C_RED));
        underline_locus(locus, line, '~', '~');
        if (has_text) {
            printf(" %s>%s %s", paint(C_RESET), paint(C_BRED), label->text);
        }
    } else {
        printf("%s", paint(C_YELLOW));
        underline_locus(locus, line, '^', '-');
        if (has_text) {
            printf(" > %s", label->text);
        }
    }
    printf("%s\n", paint(C_RESET));
}

/* ---- several labels on one source line ----------------------------
 *
 *   7 | a := 3.14;
 *     | ~    ^^^^
 *     | |    |
 *     | |    +-- secondary note
 *     | +-- primary error
 */

static usize label_start(const Label &l)
{
    return l.locus.begin > 0 ? l.locus.begin - 1 : 0;
}

static usize label_span(const Label &l, LineSpan line)
{
    Locus loc    = l.locus;
    usize length = locus_length(&loc);
    usize span   = length > 0 ? length : 1;
    usize start  = label_start(l);
    if (start < line.len && start + span > line.len) {
        span = line.len - start;
    }
    return span;
}

static const char *label_color(const Label &l)
{
    return l.is_primary ? C_RED : C_YELLOW;
}

static bool label_has_text(const Label &l) { return l.text && *l.text; }

static void put_pad(LineSpan line, usize col)
{
    putchar((col < line.len && line.ptr[col] == '\t') ? '\t' : ' ');
}

/* Which label owns this column? Primary wins overlaps, otherwise the
 * label that starts later (it is the more specific one). */
static const Label *label_at(const Label *labels, usize count, LineSpan line,
                             usize col)
{
    const Label *best = nullptr;
    for (usize i = 0; i < count; ++i) {
        const Label &l = labels[i];
        usize s        = label_start(l);
        if (col < s || col >= s + label_span(l, line)) {
            continue;
        }
        if (!best || (l.is_primary && !best->is_primary) ||
            (l.is_primary == best->is_primary && s >= label_start(*best))) {
            best = &l;
        }
    }
    return best;
}

/* One connector row. Labels [0, hang_idx) get a '|' stem; if hang_idx < count
 * the label at hang_idx gets its "+-- text". hang_idx == count: stems only. */
static void print_label_row(const Label *labels, usize count, usize hang_idx,
                            LineSpan line, usize gutter)
{
    printf(" %*s | ", (int)gutter, "");

    const usize stems = hang_idx < count ? hang_idx : count;
    const usize hang_start =
        hang_idx < count ? label_start(labels[hang_idx]) : (usize)-1;
    usize col = 0;

    for (usize i = 0; i < stems; ++i) {
        const Label &l = labels[i];
        usize s        = label_start(l);
        if (!label_has_text(l) || s < col || s >= hang_start) {
            continue;
        }
        while (col < s) {
            put_pad(line, col++);
        }
        printf("%s|%s", paint(label_color(l)), paint(C_RESET));
        col++;
    }

    if (hang_idx < count) {
        const Label &l = labels[hang_idx];
        while (col < hang_start) {
            put_pad(line, col++);
        }
        printf("%s+-- %s%s", paint(label_color(l)), l.text, paint(C_RESET));
    }
    putchar('\n');
}

/* `labels` are all on the same line, sorted by column. The source line itself
 * has already been printed by the caller. */
static void render_line_labels(const Label *labels, usize count,
                               const SourceFile *file, usize gutter)
{
    const usize line_num = labels[0].locus.line > 0 ? labels[0].locus.line : 1;
    const LineSpan line  = source_file_get_line(file, line_num - 1);

    usize width   = 0;
    bool any_text = false;
    for (usize i = 0; i < count; ++i) {
        usize end = label_start(labels[i]) + label_span(labels[i], line);
        if (end > width) {
            width = end;
        }
        any_text = any_text || label_has_text(labels[i]);
    }

    /* combined underline row */
    printf(" %*s | ", (int)gutter, "");
    for (usize col = 0; col < width; ++col) {
        const Label *l = label_at(labels, count, line, col);
        if (l) {
            printf("%s%c", paint(label_color(*l)), l->is_primary ? '~' : '^');
        } else {
            printf("%s", paint(C_RESET));
            put_pad(line, col);
        }
    }
    printf("%s\n", paint(C_RESET));

    if (!any_text) {
        return;
    }

    /* stems, then labels hanging from the rightmost to the leftmost */
    print_label_row(labels, count, count, line, gutter);
    for (usize k = count; k-- > 0;) {
        if (label_has_text(labels[k])) {
            print_label_row(labels, count, k, line, gutter);
        }
    }
}

/* Renders a run of labels that live in the same file and are close together.
 * Labels sharing a line are drawn together; lines between labels are printed
 * so the snippet reads continuously. */
static void render_group(const Label *labels, usize count,
                         const SourceManager *sm, usize gutter)
{
    usize prev_line = 0;

    for (usize i = 0; i < count;) {
        const usize line_num =
            labels[i].locus.line > 0 ? labels[i].locus.line : 1;
        usize j = i + 1;
        while (j < count && labels[j].locus.line == labels[i].locus.line) {
            ++j;
        }

        const SourceFile *file =
            sourcemgr_get_source(sm, labels[i].locus.source_id);
        const bool first = (i == 0);

        if (!first) {
            for (usize ln = prev_line + 1; ln < line_num; ++ln) {
                print_source_line(file, ln, gutter);
            }
        }

        if (j - i == 1) {
            render_label(&labels[i], file, gutter, first, true);
        } else {
            if (first && line_num > 1) {
                usize from =
                    line_num > CONTEXT_LINES ? line_num - CONTEXT_LINES : 1;
                for (usize n = from; n < line_num; ++n) {
                    print_source_line(file, n, gutter);
                }
            }
            print_source_line(file, line_num, gutter);
            render_line_labels(labels + i, j - i, file, gutter);
        }

        prev_line = line_num;
        i         = j;
    }
}

/* ------------------------------------------------------------------ */
/* public API                                                          */
/* ------------------------------------------------------------------ */

void diag_report(const Diagnostic *diagnostic, const SourceManager *sm)
{
    if (!diagnostic->labels || mini_array_count(diagnostic->labels) == 0)
        return;

    Label *labels           = diagnostic->labels;
    const usize label_count = mini_array_count(labels);

    /* pick the primary label before sorting reorders things */
    Label primary_label = labels[0];
    for (usize i = 0; i < label_count; ++i) {
        if (labels[i].is_primary) {
            primary_label = labels[i];
            break;
        }
    }
    const SourceFile *primary_file =
        sourcemgr_get_source(sm, primary_label.locus.source_id);

    std::qsort(labels, label_count, sizeof(Label), label_compar);

    /* gutter width from the largest line number that can be printed */
    usize max_line = 1;
    for (usize i = 0; i < label_count; ++i) {
        if (labels[i].locus.line > max_line) {
            max_line = labels[i].locus.line;
        }
    }
    usize gutter = 1;
    for (usize temp = max_line; temp >= 10; temp /= 10) {
        gutter++;
    }

    printf("%s%s%s: %s:%zu:%zu: %s\n",
           paint(severity_color(diagnostic->severity)),
           severity_prefix(diagnostic->severity), paint(C_RESET),
           primary_file->path, primary_label.locus.line,
           primary_label.locus.begin, diagnostic->message);

    printf(" %*s |\n", (int)gutter, "");

    auto current_source = primary_label.locus.source_id;
    for (usize i = 0; i < label_count;) {
        usize j = i + 1;
        while (j < label_count &&
               labels_share_group(labels[j - 1], labels[j])) {
            ++j;
        }

        const Label &first = labels[i];
        if (first.locus.source_id != current_source) {
            const SourceFile *file =
                sourcemgr_get_source(sm, first.locus.source_id);
            printf(" %*s--> %s:%zu:%zu\n", (int)gutter, "", file->path,
                   first.locus.line, first.locus.begin);
            current_source = first.locus.source_id;
        } else if (i != 0) {
            printf(" %*s...\n", (int)gutter + 1, "");
        }

        render_group(labels + i, j - i, sm, gutter);
        i = j;
    }

    printf(" %*s |\n", (int)gutter, "");

    /* end diagnostic */
    printf("\n");
}

void diag_destroy(void *_diag)
{
    Diagnostic *diag = (Diagnostic *)_diag;
    mini_array_destroy(diag->labels);
}

Diagnostic diag_create(DIAG_CTOR)
{
    Diagnostic diagnostic{};
    diagnostic.labels   = MINI_ARRAY_INIT(mini_default_allocator(), Label);
    diagnostic.message  = message;
    diagnostic.severity = severity;
    diagnostic          = diag_add_label(diagnostic, Label{text, locus, true});
    return diagnostic;
}

Diagnostic diag_add_label(Diagnostic diagnostic, Label label)
{
    if (diagnostic.labels) {
        mini_array_append(diagnostic.labels, label);
    }
    return diagnostic;
}

Diagnostic_Pool diagpool_create()
{
    Diagnostic_Pool pool{};
    pool.diagnostics = MINI_ARRAY_INIT(mini_default_allocator(), Diagnostic);
    mini_array_set_dtor(pool.diagnostics, diag_destroy);
    return pool;
}

void diagpool_report(Diagnostic_Pool *pool, DIAG_CTOR)
{
    Diagnostic diag = diag_create(severity, locus, message, text);
    mini_array_append(pool->diagnostics, diag);
}

void diagpool_destroy(Diagnostic_Pool *pool)
{
    mini_array_destroy(pool->diagnostics);
}

void diagpool_report_diag(Diagnostic_Pool *pool, Diagnostic diagnostic)
{
    mini_array_append(pool->diagnostics, diagnostic);
}

bool diagpool_is_empty(Diagnostic_Pool *pool)
{
    return mini_array_count(pool->diagnostics) == 0;
}
