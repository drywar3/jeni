#pragma once

#include <mini.c/mini_def.h>

#define CREATE_ID(Name)                             \
    struct Name {                                   \
        constexpr explicit Name(usize id) : id(id) {}                  \
        usize id;                                   \
        bool operator==(const Name &other) const {  \
            return id == other.id;                  \
        }                                           \
    };
