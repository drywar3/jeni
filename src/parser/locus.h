#pragma once

#include <cstring>

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
    const char *file_path;
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
        a.last_byte != b.last_byte) {
        return false;
    }

    if (a.file_path == b.file_path) {
        return true;
    }
    if (!a.file_path || !b.file_path) {
        return false;
    }
    return std::strcmp(a.file_path, b.file_path) == 0;
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

        /*
         * hash string content if [file_path] is non-null, else hash zero
         */
        if (loc.file_path) {
            /* [std::string_view] hashes string content without allocating */
            hash_combine(seed, Hash<Mini_StringView>{}(mini_sv_from_cstr(loc.file_path)));
        } else {
            hash_combine(seed, 0);
        }

        return seed;
    }
};

Locus locus_create(usize line, usize begin, usize end, usize fb, usize lb, const char *path);
usize locus_length(const Locus *locus);
Locus locus_merge(Locus locus, Locus other);
