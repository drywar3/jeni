#include "ast.h"

Program program_init(Mini_Allocator allocator) {
    Program program = {0};
    program.ast     = MINI_ARRAY_INIT(allocator, StatementPointer);
    return program;
}

void program_add(Program *program, StatementPointer stmt) {
    mini_array_append(program->ast, stmt);
}

void program_destroy(Program *program, Mini_Allocator allocator);
