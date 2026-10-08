#pragma once

#include "parser/locus.h"
#include <mini.c/string_view.h>

typedef struct Name {
    Mini_StringView value;
    Locus locus;
} Name;

enum Mutability {
    Constant = true,
    Mutable  = false,
};

static inline Name name_create(Mini_StringView value, Locus locus)
{
    return (Name){value, locus};
}
