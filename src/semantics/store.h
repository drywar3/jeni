#pragma once

#include "ast/ast.h"
#include "misc/id_allocator.h"
#include "semantics/entities/call_schema.h"

struct Semantic_Context;

namespace sema {
    struct Type;
    struct Symbol;
    struct Scope;

    using Type_Storage   = DenseMap<Locus, Type>;
    using Symbol_Storage = DenseMap<Locus, Symbol>;
    using Scope_Storage  = DenseMap<Locus, Scope>;

    using Program_Storage = DenseMap<Locus, Program>;

    enum struct Semantic_Id : usize {};
    using Semantic_Contexts = Id_Allocator<Semantic_Context,
                                           Semantic_Id>;
}

struct Semantic_Storage {
    using Call_Schemas = HashMap<sema::Symbol_Id, sema::Function_Call_Schema>;

    sema::Scope_Storage scopes;
    sema::Symbol_Storage symbols;
    sema::Type_Storage types;
    sema::Semantic_Contexts semantic_contexts;
    sema::Program_Storage   programs;
    Call_Schemas call_schemas;
};
