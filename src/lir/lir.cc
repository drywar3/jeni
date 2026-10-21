#include "lir/lir.h"
#include "lir/types/type.h"

lir::Context lir::ctx_init(Mini_Allocator allocator,
                           const Semantic_Storage *store)
{
    lir::Context context{.values = ValueStorage(allocator),
                         .blocks = BlockStorage(allocator),
                         .types  = Type_Storage(allocator)};
    context.allocator = allocator;
    context.store     = store;

    context.const_true  = valuestore_index(&context.values, Value::True());
    context.const_false = valuestore_index(&context.values, Value::False());
    return context;
}

lir::Block *lir::Context::get_block(BlockId id) { return &blocks[usize(id)]; }

lir::BlockId lir::Context::new_block(std::optional<lir::BlockId> parent)
{
    blocks.append(Block{parent, Block::LocalMap(allocator)});
    return lir::BlockId(blocks.count() - 1);
}

lir::Type_Id lir::Context::new_type(lir::Type_Builder type_builder)
{
    types.append(type_builder);
    return lir::Type_Id(types.count() - 1);
}

lir::Type_Builder *lir::Context::get_type(lir::Type_Id type_id)
{
    return &types[usize(type_id)];
}

lir::Type_Builder *lir::Context::get_type(mini::StringView name)
{
    for (auto &type_builder : types.iter()) {
        if (type_builder.name == name) return &type_builder;
    }
    return nullptr;
}

lir::Type_Id lir::Context::get_id_of_type(mini::StringView name)
{
    usize n = 0;
    for (auto &type_builder : types.iter()) {
        if (type_builder.name == name) return lir::Type_Id(n);
        n += 1;
    }

    MINI_UNREACHABLE("cannot find type: %.*s", SVARG(name.base()));
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

lir::Global lir::lower_glob_function(lir::Buildr *b, const hir::Statement *stmt)
{
    const hir::stmt::Function &function = stmt->as.function;

    lir::TypePtr return_type =
        lir::lower_type(b, function.prototype.return_type);

    lir::Global::Function g_function = lir::function_init(
        b, function.name, return_type, function.prototype.is_variadic, function.profile);
    lir::Buildr new_builder = g_function.buildr(b->mod);

    //printf("canon name = %s\n", function.profile.canonical_name);

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

static lir::Global lower_struct_type(lir::Buildr *b,
                                     const hir::Statement *stmt)
{
    const auto &strukt = stmt->as.strukt;
    lir::Global g{};
    lir::Type_Builder type_builder(b->allocator(), strukt.name);

    for (const auto field : strukt.fields.iter()) {
        lir::Type_Member member{};
        member.name = field.name;
        member.type = lir::lower_type(b, field.type_id);
        type_builder.add_member(member);
    }

    lir::Type_Id type_id = b->mod->context->new_type(type_builder);
    g.kind = lir::Global::Kind::Type;
    g.type_id = type_id;
    return g;
}

static lir::Global lower_glob_statement(lir::Buildr *builder,
                                        const hir::Statement *stmt)
{
    switch (stmt->kind) {
    case hir::Statement::Variable:
        return lower_glob_variable(builder, stmt);
    case hir::Statement::Function:
        return lower_glob_function(builder, stmt);
    case hir::Statement::Struct:
        return lower_struct_type(builder, stmt);
    default:
        MINI_UNREACHABLE("%d", stmt->kind);
    }
}

void lir::inflate_module(lir::Module *mod, const hir::Program program)
{
    lir::Buildr buildr = mod->new_buildr();

    const auto deflate_group = [&] (this auto &self, auto *stmt) -> bool {
        if (stmt->kind == hir::Statement::Group) {
            const auto &block = stmt->as.block;
            for (auto &n_stmt : block.body.iter()) {
                if (self(n_stmt)) continue;
                mod->globals.append(lower_glob_statement(&buildr, n_stmt));
            }
            return true;
        }
        return false;
    };

    for (const hir::Statement *stmt : mini::iterate(program)) {
        if (deflate_group(stmt)) continue;
        if (stmt->kind == hir::Statement::Struct) {
            lir::Global global = lower_glob_statement(&buildr, stmt);
            mod->globals.append(global);
        }
    }

    for (const hir::Statement *stmt : mini::iterate(program)) {
        if (stmt->kind == hir::Statement::Struct ||
            stmt->kind == hir::Statement::Nop ||
            stmt->kind == hir::Statement::Group) continue;
        lir::Global global = lower_glob_statement(&buildr, stmt);
        mod->globals.append(global);
    }
}
