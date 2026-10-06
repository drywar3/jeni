#include "codegen/backends/c.h"
#include "lir/types/instruction.h"
#include "lir/types/type.h"
#include "mini.c/string_view.h"

#include <libtcc.h>
#include <mini.c/string.h>
#include <mini.cc/scope_guard.h>

#define writec(bk, ...) mini_string_append_fmt(&(bk)->code, ##__VA_ARGS__)
#define writef(bk, ...) mini_string_append_fmt(&(bk)->fwd_decl, ##__VA_ARGS__)
#define writeo(out, ...) mini_string_append_fmt(out, ##__VA_ARGS__)

#define INDENT "    "

struct CBackend {
    Mini_String headers;
    Mini_String fwd_decl;
    Mini_String code;
    codegen::Context *context;
};

bool is_constant_value(const lir::Module *mod, lir::ValueId val_id)
{
    const auto &val = mod->context->values[size_t(val_id)];
    switch (val.kind) {
    case lir::Value::Kind::Integer:
    case lir::Value::Kind::String:
        return true;

    case lir::Value::Kind::GlobalRef:
    case lir::Value::Kind::LocalRef:
    case lir::Value::Kind::ParamRef:
        return false;

    default:
        return false;
    }
}

static void write_type(Mini_String *out, lir::TypePtr type);
static void write_value(CBackend *bk, Mini_String *out, lir::ValueId value_id);
static void write_instruction(CBackend *bk, Mini_String *out,
                              lir::Instruction inst);

static void emit_module_prelude(CBackend *bk)
{
    mini_string_append_string(
        &bk->headers,
        "/*--------------------------------------------------------*/\n");
    mini_string_append_string(&bk->headers, "#include <stddef.h>\n");
    mini_string_append_string(&bk->headers, "#include <stdint.h>\n");
    mini_string_append_string(&bk->headers, "#include <stdbool.h>\n\n");
    mini_string_append_string(&bk->headers,
                              "typedef struct _JeniVoid { } _JeniVoid;\n");
    mini_string_append_string(
        &bk->headers,
        "/*--------------------------------------------------------*/\n\n");
}

static void emit_function_fwd_declaration(CBackend *bk,
                                          const lir::Global::Function *function)
{
    write_type(&bk->fwd_decl, function->prototype.return_type);
    writef(bk, " %.*s (", SVARG(function->name.base()));
    usize n = 0;
    for (const auto &param_type : function->prototype.parameters.iter()) {
        if (n != 0)
            writef(bk, ", ");
        write_type(&bk->fwd_decl, param_type);
        writef(bk, " param_%zu", n);
        n += 1;
    }
    if (function->is_variadic)
        writef(bk, ", ...");
    writef(bk, ");\n");
}

static void emit_global_declarations(CBackend *bk)
{
    const auto &mod = *bk->context->mod;
    for (const auto &global : mod.globals.iter()) {
        if (global.kind != lir::Global::Kind::Variable)
            continue;

        const auto &variable = global.variable;
        write_type(&bk->fwd_decl, variable.type);

        if (is_constant_value(&mod, variable.initializer)) {
            writef(bk, " %.*s = ", SVARG(global.name.base()));
            write_value(bk, &bk->fwd_decl, variable.initializer);
            writef(bk, ";\n");
        } else {
            writef(bk, " %.*s;\n", SVARG(global.name.base()));
        }
    }
}

static void emit_global_init_function(CBackend *bk)
{
    const auto &mod          = *bk->context->mod;
    //bool has_dynamic_globals = false;

    for (const auto &global : mod.globals.iter()) {
        if (global.kind == lir::Global::Kind::Variable) {
            if (!is_constant_value(&mod, global.variable.initializer)) {
                //has_dynamic_globals = true;
                break;
            }
        }
    }

    // if (!has_dynamic_globals)
    //     return;

    writef(bk, "void __jeni_global_init(void);\n");

    writec(bk, "void __jeni_global_init(void) {\n");
    for (const auto &global : mod.globals.iter()) {
        if (global.kind != lir::Global::Kind::Variable)
            continue;

        const auto &variable = global.variable;
        if (!is_constant_value(&mod, variable.initializer)) {
            writec(bk, "    %.*s = ", SVARG(global.name.base()));
            write_value(bk, &bk->code, variable.initializer);
            writec(bk, ";\n");
        }
    }
    writec(bk, "}\n\n");
}

static void emit_forward_declarations(CBackend *bk)
{
    const auto &mod = *bk->context->mod;
    for (const auto &global : mod.globals.iter()) {
        switch (global.kind) {
        case lir::Global::Kind::Variable:
            continue;
        case lir::Global::Kind::Function:
            emit_function_fwd_declaration(bk, &global.function);
            break;
        default:
            MINI_UNREACHABLE();
        }
    }
}

static void emit_function_definitions(CBackend *bk);

Mini_String codegen::c_backend_entry_point(codegen::Context *context)
{
    CBackend backend;
    backend.headers  = mini_string_init(context->allocator);
    backend.fwd_decl = mini_string_init(context->allocator);
    backend.code     = mini_string_init(context->allocator);
    backend.context  = context;

    emit_module_prelude(&backend);
    emit_global_declarations(&backend);
    emit_forward_declarations(&backend);
    emit_global_init_function(&backend);
    emit_function_definitions(&backend);

    Mini_String output = mini_string_init(context->allocator);
    mini_string_append_string(&output, backend.headers);
    mini_string_append_string(&output, backend.fwd_decl);
    mini_string_append_string(&output, backend.code);

    return output;
}

void write_type(Mini_String *out, lir::TypePtr type)
{
    switch (type->kind) {
    case lir::Type::Kind::Int8:
        mini_string_append_string(out, "int8_t");
        break;
    case lir::Type::Kind::Int32:
        mini_string_append_string(out, "int32_t");
        break;
    case lir::Type::Kind::Int64:
        mini_string_append_string(out, "int64_t");
        break;
    case lir::Type::Kind::String:
        mini_string_append_string(out, "char *");
        break;
    case lir::Type::Kind::Void:
        mini_string_append_string(out, "_JeniVoid");
        break;
    case lir::Type::Kind::Pointer:
        if (type->pointer.mutability) {
            writeo(out, "const ");
        }
        write_type(out, type->pointer.inner);
        mini_string_append_string(out, "*");
        break;
    case lir::Type::Kind::Function:
    default:
        MINI_UNREACHABLE("%d", type->kind);
    }
}

void write_value(CBackend *bk, Mini_String *out, lir::ValueId value_id)
{
    const auto *lir_context = bk->context->mod->context;
    const auto &value       = lir_context->values[usize(value_id)];
    switch (value.kind) {
    case lir::Value::Kind::Integer:
        mini_string_append_fmt(out, "%ld", value.integer);
        break;
    case lir::Value::Kind::ParamRef:
        mini_string_append_fmt(out, "param_%zu", value.index);
        break;
    case lir::Value::Kind::LocalRef:
        mini_string_append_fmt(out, "local_%zu", value.index);
        break;
    case lir::Value::Kind::GlobalRef:
        mini_string_append_fmt(out, "%.*s", SVARG(value.ident.base()));
        break;
    case lir::Value::Kind::Deref:
        mini_string_append(out, '*');
        write_value(bk, out, value.valueid);
        break;
    case lir::Value::Kind::CString:
        mini_string_append_fmt(out, "%.*s", SVARG(value.string.base()));
        break;
    default:
        MINI_UNREACHABLE();
    }
}

void emit_stack_slots(CBackend *bk, Mini_String *out,
                      const lir::Global::Function *function)
{
    writeo(out, INDENT "//"
                       "-------------------------------------------------------"
                       "--------------\n");
    for (const auto &inst_ : function->instructions.iter()) {
        auto &inst = inst_.inst;
        if (inst.opcode == lir::InstructionKind::OpCode::Alloca) {
            writeo(out, INDENT);
            MINI_ASSERT(inst_.dst.has_value(),
                        "no valid destination for alloca instruction");
            write_type(out, inst.as.alloca.type);
            writeo(out, " ");
            write_value(bk, out, *inst_.dst);
            writeo(out, "_slot;\n");
        }
    }
    writeo(out, INDENT "//"
                       "-------------------------------------------------------"
                       "--------------\n");
}

void emit_function_definition(CBackend *bk,
                              const lir::Global::Function *function)
{
    write_type(&bk->code, function->prototype.return_type);
    writec(bk, " %.*s (", SVARG(function->name.base()));
    usize n = 0;
    for (const auto &param_type : function->prototype.parameters.iter()) {
        if (n != 0)
            writec(bk, ", ");
        write_type(&bk->code, param_type);
        writec(bk, " param_%zu", n);
        n += 1;
    }

    if (function->is_variadic) {
        writec(bk, ", ...");
    }

    writec(bk, ")");

    if (function->body_is_defined) {
        writec(bk, " {\n");
        if (function->name == "main")
            writec(bk, INDENT"__jeni_global_init();\n");
        emit_stack_slots(bk, &bk->code, function);

        for (const auto &inst : function->instructions.iter()) {
            writec(bk, INDENT);
            write_instruction(bk, &bk->code, inst);
        }
        if (function->name == "main" &&
            function->prototype.return_type->kind == lir::Type::Kind::Int32) {
            writec(bk, INDENT "return 0;\n");
        }
        writec(bk, "}");
    } else {
        writec(bk, ";");
    }
    writec(bk, "\n");
}

void emit_function_definitions(CBackend *bk)
{
    for (const auto &g : bk->context->mod->globals.iter()) {
        if (g.kind != lir::Global::Kind::Function)
            continue;
        const lir::Global::Function &function = g.function;
        emit_function_definition(bk, &function);
    }
}

void write_instruction(CBackend *bk, Mini_String *out, lir::Instruction inst_)
{
    auto &inst = inst_.inst;
    switch (inst.opcode) {
    case lir::InstructionKind::OpCode::Alloca:
        MINI_ASSERT(inst_.dst.has_value(),
                    "no valid destination for alloca instruction");

        write_type(out, inst.as.alloca.type);
        writeo(out, " *");
        write_value(bk, out, *inst_.dst);
        writeo(out, " = &");
        write_value(bk, out, *inst_.dst);
        writeo(out, "_slot;\n");
        break;
    case lir::InstructionKind::OpCode::Store:
        MINI_ASSERT(inst_.dst.has_value(),
                    "no valid destination for store instruction");
        write_value(bk, out, *inst_.dst);
        writeo(out, " = (");
        write_type(out, inst.as.store.type);
        writeo(out, ")");
        write_value(bk, out, inst.as.store.value);
        writeo(out, ";\n");
        break;
    case lir::InstructionKind::OpCode::Call: {
        if (inst_.dst.has_value()) {
            write_value(bk, out, *inst_.dst);
            writeo(out, " = ");
        }
        write_value(bk, out, inst.as.call.callee);
        writeo(out, "(");
        usize n = 0;
        for (auto arg : inst.as.call.arguments.iter()) {
            if (n != 0)
                writeo(out, ", ");
            write_value(bk, out, arg);
            n += 1;
        }
        writeo(out, ");\n");
    } break;
    default:
        MINI_UNREACHABLE();
    }
}

bool codegen::c_backend_finalize(Mini_String blob, const char *output_name)
{
    TCCState *s = tcc_new();
    if (!s) {
        fprintf(stderr, "error: failed to initialize tcc\n");
        return false;
    }
    //printf("%s", blob);

    auto scope_guard = mini::ScopeGuard([&]() { tcc_delete(s); });
    tcc_set_lib_path(s, "./vendor/libtcc");
    tcc_set_output_type(s, TCC_OUTPUT_EXE);
    tcc_set_options(s, "-w");

    tcc_add_include_path(s, "./vendor/libtcc/include");
    tcc_add_library_path(s, "./vendor/libtcc/");

    tcc_add_include_path(s, "/usr/include");
    tcc_add_include_path(s, "/usr/local/include");

    tcc_add_library(s, "raylib");
    tcc_add_library(s, "m");

    if (tcc_compile_string(s, blob) < 0) {
        fprintf(stderr, "error: tcc compilation failed\n");
        return false;
    }

    if (tcc_output_file(s, output_name) < 0) {
        fprintf(stderr, "error: failed to write executable '%s'\n",
                output_name);
        return false;
    }

    return true;
}
