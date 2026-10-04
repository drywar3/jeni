#include "lir/types/buildr.h"
#include "lir/lir.h"
#include "lir/types/function.h"
#include "lir/types/instruction.h"
#include "lir/types/value.h"
#include "mini.cc/string_view.h"

const SemanticStorage *lir::Buildr::store() const
{
    return mod->context->store;
}
Mini_Allocator lir::Buildr::allocator() const
{
    return mod->context->allocator;
}

lir::ValueId lir::Buildr::create_integer(int64 value)
{
    return valuestore_index(&mod->context->values, Value::Integer(value));
}

lir::ValueId lir::Buildr::create_cstring(mini::StringView value)
{
    return valuestore_index(&mod->context->values, Value::CString(value));
}

std::optional<lir::Local> lir::Buildr::find_local(mini::StringView name)
{
    lir::Context *context                = mod->context;
    std::optional<BlockId> current_block = this->current_block;
    while (current_block) {
        Block &block = *context->get_block(*current_block);
        if (block.locals.contains(name))
            return *block.locals.find(name);

        current_block = block.parent;
    }

    return std::nullopt;
}

lir::ValueId lir::Buildr::create_local_ref(lir::Local local)
{
    if (local.is_parameter)
        return valuestore_index(&mod->context->values,
                                Value::ParamRef(local.index));
    else
        return valuestore_index(&mod->context->values,
                                Value::LocalRef(local.index));
}

lir::ValueId lir::Buildr::create_glob_ref(mini::StringView name)
{
    return valuestore_index(&mod->context->values, Value::GlobalRef(name));
}

lir::ValueId lir::Buildr::parameter(TypePtr type, mini::StringView name)
{
    MINI_ASSERT(function != nullptr,
                "cannot add parameter when there is no function context");
    usize index = function->prototype.parameters.append(type);
    Local local{.name = name, .index = index, .is_parameter = true};
    return create_local_ref(local);
}

void lir::Buildr::new_block()
{
    current_block = mod->context->new_block(current_block);
}

void lir::Buildr::end_block()
{
    Block &block = *mod->context->get_block(current_block);
    if (block.parent) {
        current_block = *block.parent;
    } else {
        current_block = function->block;
    }
}

usize lir::Buildr::new_local(mini::StringView name)
{
    MINI_ASSERT(function != nullptr,
                "cannot add local when there is no function context");
    usize current_local_index = function->current_local_index++;
    Block &block              = *mod->context->get_block(current_block);
    block.locals[name]        = Local{
        .name = name, .index = current_local_index, .is_parameter = false};
    return current_local_index;
}

lir::ValueId lir::Buildr::create_alloca(mini::StringView name,
                                        lir::TypePtr type)
{
    MINI_ASSERT(function != nullptr,
                "cannot add instruction when there is no function context");
    Instruction inst{};
    usize local_index = new_local(name);
    inst.dst =
        valuestore_index(&mod->context->values, Value::LocalRef(local_index));
    inst.inst = InstructionKind::Alloca(type);
    function->add_instruction(inst);
    return *inst.dst;
}

void lir::Buildr::create_store(ValueId dst, lir::TypePtr type, ValueId value)
{
    MINI_ASSERT(function != nullptr,
                "cannot add instruction when there is no function context");
    Instruction inst{};
    inst.dst  = dst;
    inst.inst = InstructionKind::Store(type, value);
    function->add_instruction(inst);
}

lir::ValueId lir::Buildr::create_deref(ValueId value)
{
    return valuestore_index(&mod->context->values, Value::Deref(value));
}

lir::ValueId lir::Buildr::create_temporary(lir::TypePtr type)
{
    MINI_ASSERT(function != nullptr,
                "cannot add instruction when there is no function context");
    usize current_local_index = function->current_local_index++;
    Instruction inst{};
    lir::ValueId dst = valuestore_index(&mod->context->values,
                                        Value::LocalRef(current_local_index));
    inst.dst         = dst;
    inst.inst        = InstructionKind::Alloca(type);
    function->add_instruction(inst);
    return dst;
}

lir::ValueId lir::Buildr::create_call(lir::TypePtr type, ValueId callee,
                                      mini::Array<ValueId> args)
{
    MINI_ASSERT(function != nullptr,
                "cannot add instruction when there is no function context");
    lir::ValueId result = create_temporary(type);
    lir::Instruction inst{};
    inst.inst = InstructionKind::Call(type, callee, args);
    inst.dst  = create_deref(result);
    function->add_instruction(inst);
    return result;
}
