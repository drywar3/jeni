#include "stmt.h"

const char *statement_name(Statement_Kind kind)
{
    return (const char *[]){
        [STMT_Variable] = "variable-declaration",
        [STMT_If]       = "if-statement",
        [STMT_For]      = "for-statement",
        [STMT_Expr]     = "unused-expression",
    }[(int)kind];
}
