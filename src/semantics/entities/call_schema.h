#pragma once

#include "misc/map.h"
#include "ast/expr.h"
#include "semantics/entities/type.h"
#include "semantics/entities/symbol.h"

#include <mini.cc/string_view.h>

namespace sema
{
    struct ParameterSpec {
        Locus               locus;
        usize               index;
        sema::TypeId        type_id;
        const ::Expression *default_expression;
    };

    struct FunctionArity {
        usize min;
        usize max;
    };

    struct FunctionCallSchema {
        using Parameters = HashMap<mini::StringView, ParameterSpec>;
        using IndexToParameterName = HashMap<usize, mini::StringView>;

        Parameters parameters;
        FunctionArity arity;
        sema::SymbolId symbol_id;
        bool is_variadic;
        IndexToParameterName parameter_names;

        FunctionCallSchema(Mini_Allocator allocator = mini_default_allocator())
            : parameters(allocator), parameter_names(allocator) {}

        ParameterSpec get_parameter_at_index(usize index) const
        {
            auto name = *parameter_names.find(index);
            return *parameters.find(name);
        }
    };
} // namespace sema
