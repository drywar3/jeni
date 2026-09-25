#pragma once

#include <mini.c/array.h>

#include "stmt.h"

typedef struct Program {
    MINI_ARRAY(StatementPointer) ast;
} Program;


Program program_init(Mini_Allocator allocator);
void program_add(Program *program, StatementPointer stmt);

void program_destroy(Program *program, Mini_Allocator allocator);
