#include "print.h"
#include "ast/expr.h"
#include "expressions.h"
#include "statements.h"
#include "type.h"

static void print_indent(usize indent)
{
    for (usize n = 0; n < indent; n++) {
        printf("    ");
    }
}

static void print_string(Mini_StringView string)
{
    printf("\"");

    for (usize n = 0; n < string.length; n++) {
        const char c = string.data[n];

        switch (c) {
        case '\\':
            printf("\\\\");
            break;

        case '"':
            printf("\\\"");
            break;

        case '\n':
            printf("\\n");
            break;

        case '\r':
            printf("\\r");
            break;

        case '\t':
            printf("\\t");
            break;

        default:
            putchar(c);
            break;
        }
    }

    printf("\"");
}

void print_type(const Typehint *typehint, usize indent)
{
    if (!typehint) {
        printf("null");
        return;
    }

    switch (typehint->kind) {
    case TypehintKind::TYPEHINT_Void:
        printf("void");
        break;
    case TypehintKind::TYPEHINT_Integer:
        printf("int");
        break;
    case TypehintKind::TYPEHINT_String:
        printf("string");
        break;
    default:
        MINI_UNREACHABLE("%d", typehint->kind);
    }
}

void print_expr(const Expression *expr, usize indent)
{
    switch (expr->kind) {
    case EXPR_Integer: {
        const ExprInteger *integer = (const ExprInteger *)expr;

        printf("{\n");

        print_indent(indent + 1);
        printf("\"kind\": \"integer\",\n");

        print_indent(indent + 1);
        printf("\"value\": %ld\n", integer->value);

        print_indent(indent);
        printf("}");
    } break;

    case EXPR_Identifier: {
        const ExprIdentifier *identifier = (const ExprIdentifier *)expr;

        printf("{\n");

        print_indent(indent + 1);
        printf("\"kind\": \"identifier\",\n");

        print_indent(indent + 1);
        printf("\"value\": ");
        print_string(identifier->value);
        printf("\n");

        print_indent(indent);
        printf("}");
    } break;

    case EXPR_FunctionCall: {
        const ExprFunctionCall *function_call = (const ExprFunctionCall *)expr;

        printf("{\n");

        print_indent(indent + 1);
        printf("\"kind\": \"function_call\",\n");

        print_indent(indent + 1);
        printf("\"callee\": ");
        print_expr(function_call->callee, indent + 1);
        printf(",\n");

        print_indent(indent + 1);
        printf("\"arguments\": [");

        const usize count = function_call->arguments.count();

        if (count > 0)
            printf("\n");

        for (usize n = 0; n < count; n++) {
            print_indent(indent + 2);

            print_expr(function_call->arguments[n].argument, indent + 2);

            if (n + 1 < count)
                printf(",");

            printf("\n");
        }

        if (count > 0)
            print_indent(indent + 1);

        printf("]\n");

        print_indent(indent);
        printf("}");
    } break;
    case EXPR_Function: {
        const ExprFunction *function = (const ExprFunction *)expr;

        printf("{\n");

        print_indent(indent + 1);
        printf("\"kind\": \"function\",\n");

        print_indent(indent + 1);
        printf("\"return type\": ");
        print_type(function->prototype.return_type, indent + 1);
        printf(",\n");

        print_indent(indent + 1);
        printf("\"parameters\": [");

        const usize count = function->prototype.parameters.count();

        if (count > 0)
            printf("\n");

        for (usize n = 0; n < count; n++) {
            print_indent(indent + 2);

            printf("\"name\": %.*s,\n",
                   SVARG(function->prototype.parameters[n].name.value));
            print_indent(indent + 2);
            printf("\"type\":  ");
            print_type(function->prototype.parameters[n].typehint, indent + 2);

            if (n + 1 < count)
                printf(",");

            printf("\n");
        }

        if (count > 0)
            print_indent(indent + 1);

        printf("]\n");

        print_indent(indent);
        printf("}");
    } break;

    default:
        MINI_UNREACHABLE();
    }
}

void ast_print(const Statement *statement, usize indent)
{
    switch (statement->kind) {
    case STMT_Variable: {
        const StmtVariable *variable = (const StmtVariable *)statement;

        printf("{\n");

        print_indent(indent + 1);
        printf("\"kind\": \"variable\",\n");

        print_indent(indent + 1);
        printf("\"name\": ");
        print_string(variable->name.value);
        printf(",\n");

        print_indent(indent + 1);
        printf("\"type\": ");

        if (variable->type_is_defined) {
            print_type(variable->typehint, indent + 1);
        } else {
            printf("null");
        }

        printf(",\n");

        print_indent(indent + 1);
        printf("\"value\": ");

        if (variable->initializer) {
            print_expr(variable->initializer, indent + 1);
        } else {
            printf("null");
        }

        printf("\n");

        print_indent(indent);
        printf("}");
    } break;

    default:
        MINI_UNREACHABLE();
    }
    printf("\n");
}
