#include "lir/lir.h"

lir::Context lir::ctx_init(Mini_Allocator allocator,
                           const SemanticStorage *store)
{
    lir::Context context{.values = ValueStorage(allocator),
                         .blocks = BlockStorage(allocator)};
    context.allocator = allocator;
    context.store     = store;
    return context;
}

lir::Block *lir::Context::get_block(BlockId id) { return &blocks[usize(id)]; }

lir::BlockId lir::Context::new_block(std::optional<lir::BlockId> parent)
{
    blocks.append(Block{parent, Block::LocalMap(allocator)});
    return lir::BlockId(blocks.count() - 1);
}

void lir::ctx_destroy(lir::Context *context) { MINI_UNREACHABLE(); }

lir::Module lir::module_init(lir::Context *context)
{
    lir::Module mod{.globals = lir::Module::Globals(context->allocator)};
    mod.context = context;
    return mod;
}

static lir::Global lower_glob_variable(lir::Buildr *builder,
                                       const hir::Statement *stmt)
{
    const hir::stmt::Variable &variable = stmt->as.variable;
    lir::Global::Variable g_variable;
    g_variable.type = lir::lower_type(builder, variable.type_id);
    g_variable.initializer =
        lir::lower_expression(builder, variable.initializer);

    return lir::Global{
        .name     = variable.name,
        .kind     = lir::Global::Kind::Variable,
        .variable = g_variable,
    };
}

static lir::Global lower_glob_function(lir::Buildr *b,
                                       const hir::Statement *stmt)
{
    const hir::stmt::Function &function = stmt->as.function;

    lir::TypePtr return_type =
        lir::lower_type(b, function.prototype.return_type);

    lir::Global::Function g_function =
        lir::function_init(b, function.name, return_type, function.prototype.is_variadic);
    lir::Buildr new_builder = g_function.buildr(b->mod);

    new_builder.new_block();
    for (const auto &parameter : mini::iterate(function.prototype.parameters)) {
        lir::TypePtr param_type =
            lir::lower_type(&new_builder, parameter.type_id);
        auto param_value = new_builder.parameter(param_type, parameter.name);
        auto param_local =
            new_builder.create_alloca(parameter.name, param_type);
        new_builder.create_store(new_builder.create_deref(param_local),
                                 param_type, param_value);
    }

    g_function.body_is_defined = function.body_is_defined;
    if (function.body_is_defined) {
        lir::lower_statement(&new_builder, function.body);
    }

    new_builder.end_block();
    return lir::Global{.kind     = lir::Global::Kind::Function,
                       .function = g_function};
}

static lir::Global lower_glob_statement(lir::Buildr *builder,
                                        const hir::Statement *stmt)
{
    switch (stmt->kind) {
    case hir::Statement::Variable:
        return lower_glob_variable(builder, stmt);
    case hir::Statement::Function:
        return lower_glob_function(builder, stmt);
    default:
        MINI_UNREACHABLE();
    }
}

void lir::inflate_module(lir::Module *mod, const hir::Program program)
{
    lir::Buildr buildr = mod->buildr();
    for (const hir::Statement *stmt : mini::iterate(program)) {
        lir::Global global = lower_glob_statement(&buildr, stmt);
        mod->globals.append(global);
    }
}
