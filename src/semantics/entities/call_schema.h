#pragma once

#include "ast/expr.h"
#include "misc/map.h"
#include "semantics/entities/symbol.h"
#include "semantics/entities/type.h"

#include <mini.cc/string_view.h>

namespace sema
{
    struct Parameter_Spec {
        Locus locus;
        usize index;
        sema::Type_Id type_id;
        const ::Expression *default_expression;
    };

    struct Function_Arity {
        usize min;
        usize max;
    };

    struct Function_Call_Schema {
        using Parameters = HashMap<mini::StringView, Parameter_Spec>;
        using Index_To_Parameter_Name = HashMap<usize, mini::StringView>;

        Parameters parameters;
        Function_Arity arity;
        sema::Symbol_Id symbol_id;
        bool is_variadic;
        Index_To_Parameter_Name parameter_names;

        Function_Call_Schema(
            Mini_Allocator allocator = mini_default_allocator())
            : parameters(allocator), parameter_names(allocator)
        {
        }

        Parameter_Spec get_parameter_at_index(usize index) const
        {
            auto name = *parameter_names.find(index);
            return *parameters.find(name);
        }
    };
} // namespace sema
