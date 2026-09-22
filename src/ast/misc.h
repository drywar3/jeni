#pragma once

#include <mini.c/string_view.h>
#include "locus.h"

typedef struct Name {
    Mini_StringView value;
    Locus           locus;
} Name;

typedef enum Mutability {
    MUT_Constant = 1,
    MUT_Mutable  = 0,
} Mutability;

static inline Name name_create(Mini_StringView value, Locus locus) {
    return (Name) { value, locus };
}
