#pragma once

#include "expr.h"
#include "mini.c/string_view.h"
#include "misc.h"
#include "parser/token.h"
#include "stmt.h"
#include "type.h"

#include <mini.cc/array.h>

namespace ast
{
    struct Function_Parameter {
        Name name;
        Expression *typehint;
        Expression *default_expression;
    };

    struct Function_Prototype {
        using Parameters = mini::Array<ast::Function_Parameter>;

        Parameters  parameters;
        Expression *return_type;
        /* consider moving this to its own struct because the
         * variadic mark carry a name also
         *
         * ```
         * foobar :: func(x : int, args:...) {
         * ```
         *
         * but it is all still speculations. for now this is ok.
         */
        bool is_variadic = false;
    };

    enum struct Operator : uint {
        Add               = TOKEN_OP_Add,
        Sub               = TOKEN_OP_Minus,
        Mul               = TOKEN_OP_Star,
        Div               = TOKEN_OP_Div,
        Equals            = TOKEN_OP_Equals,
        NotEquals         = TOKEN_OP_NotEquals,
        Not               = TOKEN_OP_Bang,
        Assign            = TOKEN_OP_Assign,
        LessThanEquals    = TOKEN_OP_LessEq,
        GreaterThanEquals = TOKEN_OP_GreaterEq,
        LessThan          = TOKEN_OP_Less,
        GreaterThan       = TOKEN_OP_Greater,
    };

    struct Function_Call_Argument {
        bool is_positional = true;
        ExpressionPointer argument;
        /* some_function_name(:some_parameter_name argument) */
        Name name;
    };

    struct Struct_Field {
        mini::Array<Name> names;
        Expression       *typehint;
        Expression       *default_value;
        bool              type_is_defined;
        bool              has_default_value;
    };

    namespace expr
    {
        struct Integer : Expression {
            long value;
        };

        struct Float : Expression {
            double value;
        };

        struct Identifier : Expression {
            Mini_StringView value;
        };

        struct String : Expression {
            mini::StringView value;
        };

        struct Function : Expression {
            ast::Function_Prototype prototype;
            Statement *body;
            bool body_is_defined;
        };

        struct Binary_Operation : Expression {
            ast::Operator op;
            Expression *left;
            Expression *right;
        };

        struct Unary_Operation : Expression {
            ast::Operator op;
            Expression *expression;
        };

        struct Function_Call : Expression {
            Expression *callee;
            mini::Array<ast::Function_Call_Argument> arguments;
        };

        struct ExprError : Expression {};

        struct Struct : Expression {
            using Fields = mini::Array<Struct_Field>;

            Fields fields;
        };

        struct Member_Access : Expression {
            Expression *receiver;
            Name member;
        };

        struct AddressOf : Expression {
            Expression *value;
            Mutability  mutability;
        };

        struct Dereference : Expression {
            Expression *value;
        };

        struct Subscript : Expression {
            Expression *expr;
            Expression *index;
        };

        struct Cast : Expression {
            Expression *typehint;
            Expression *value;
        };

        template <typename Derived_Expresssion>
        static inline Expression *
        alloc_expression(Mini_Allocator allocator, Expression_Kind kind,
                         Locus locus, Derived_Expresssion e)
        {
            Expression *expression =
                (Expression *)MINI_ALLOC(allocator, Derived_Expresssion);
            new (expression) Derived_Expresssion(std::move(e));
            expression->kind  = kind;
            expression->locus = locus;
            return expression;
        }

    } // namespace expr
} // namespace ast
