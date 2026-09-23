#include "locus.h"


Locus locus_create(usize line, usize begin, usize end, usize fb, usize lb, const char *path) {
    Locus locus;
    locus.line       = line;
    locus.begin      = begin;
    locus.end        = end;
    locus.first_byte = fb;
    locus.last_byte  = lb;
    locus.file_path  = path;
    return locus;
}

usize locus_length(const Locus *locus) {
    if (locus == NULL)
        return 0;
    return locus->end - locus->begin;
}

Locus locus_merge(Locus locus, Locus other) {
    MINI_ASSERT(strcmp(locus.file_path, other.file_path) == 0, "locations point to two different sources");
    return locus_create(locus.line, locus.begin, other.end, locus.first_byte, other.last_byte, other.file_path);
}
