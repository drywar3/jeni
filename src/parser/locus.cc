#include "locus.h"

#include <mini.c/mini_def.h>

Locus locus_create(usize line, usize begin, usize end, usize fb, usize lb, SourceId id) {
    Locus locus;
    locus.line       = line;
    locus.begin      = begin;
    locus.end        = end;
    locus.first_byte = fb;
    locus.last_byte  = lb;
    locus.source_id  = id;
    return locus;
}

usize locus_length(const Locus *locus) {
    if (locus == NULL)
        return 0;
    return locus->end - locus->begin;
}

Locus locus_merge(Locus locus, Locus other) {
    MINI_ASSERT(locus.source_id == other.source_id, "locations point to two different sources");
    return locus_create(locus.line, locus.begin, other.end, locus.first_byte, other.last_byte, other.source_id);
}
