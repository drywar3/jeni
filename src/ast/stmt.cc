#include "stmt.h"

const char *statement_name(StatementKind kind) {
    return (const char *[]) {
        [STMT_Variable] = "variable-declaration",
        [STMT_If]       = "if-statement",
        [STMT_For]      = "for-statement",
        [STMT_Expr]     = "unused-expression",
    } [(int)kind];
}
