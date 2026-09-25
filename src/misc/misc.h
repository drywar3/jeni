#pragma once

#include <mini.c/map_defs.h>
#include <mini.c/string_view.h>

#include "misc/map.h"

/*
 * hash helper function combining field hashes using bit-mixing
 */
inline void hash_combine(std::size_t& seed, std::size_t value) {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

/* todo: add this to mini.c */
static inline usize mini_strview_hash(const Mini_StringView *s) {
    usize hash = 0;
    for (usize n = 0; n < s->length; n++) {
        hash_combine(hash, mini_char_hash(&s->data[n]));
    }
    return hash;
}

/*
 * specialization of [std::hash] for [Mini_StringView]
 */
template <>
struct Hash<Mini_StringView> {
    std::size_t operator()(const Mini_StringView& s) const noexcept {
        return mini_strview_hash(&s);
    }
};

inline bool operator==(const Mini_StringView& a, const Mini_StringView& b) {
    return mini_sv_equals(a, b);
}
