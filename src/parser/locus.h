#pragma once

#include <cstring>

#include "source.h"
#include "misc/misc.h"
#include "misc/map.h"
/*
 * type alias matching C [usize] to [std::size_t]
 */
using usize = std::size_t;

typedef struct {
    usize line;
    usize begin, end;
    usize first_byte, last_byte;
    SourceId source_id;
} Locus;

/*
 * equality operator required for [std::unordered_map] lookup.
 * uses [std::strcmp] for [file_path] pointer equality fallback.
 */
inline bool operator==(const Locus& a, const Locus& b) {
    if (a.line != b.line ||
        a.begin != b.begin ||
        a.end != b.end ||
        a.first_byte != b.first_byte ||
        a.last_byte != b.last_byte ||
        a.source_id != b.source_id) {
        return false;
    }
    return true;
}

/*
 * specialization of [std::hash] for [Locus]
 */
template <>
struct Hash<Locus> {
    std::size_t operator()(const Locus& loc) const noexcept {
        std::size_t seed = 0;

        hash_combine(seed, Hash<usize>{}(loc.line));
        hash_combine(seed, Hash<usize>{}(loc.begin));
        hash_combine(seed, Hash<usize>{}(loc.end));
        hash_combine(seed, Hash<usize>{}(loc.first_byte));
        hash_combine(seed, Hash<usize>{}(loc.last_byte));
        hash_combine(seed, Hash<usize>{}((usize)loc.source_id));

        return seed;
    }
};

Locus locus_create(usize line, usize begin, usize end, usize fb, usize lb, SourceId id);
usize locus_length(const Locus *locus);
Locus locus_merge(Locus locus, Locus other);
