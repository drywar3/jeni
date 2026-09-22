#pragma once

#include <mini.c/mini_def.h>

typedef struct {
    usize line;
    usize begin;
    usize end;
    const char *file_path;
} Locus;

Locus locus_create(usize line, usize begin, usize end, const char *path);
usize locus_length(const Locus *locus);
Locus locus_merge(Locus locus, Locus other);
