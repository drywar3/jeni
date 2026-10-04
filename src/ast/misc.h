#pragma once

#include "parser/locus.h"
#include <mini.c/string_view.h>

typedef struct Name {
    Mini_StringView value;
    Locus locus;
} Name;

typedef enum Mutability {
    MUT_Constant = true,
    MUT_Mutable  = false,
} Mutability;

static inline Name name_create(Mini_StringView value, Locus locus)
{
    return (Name){value, locus};
}
