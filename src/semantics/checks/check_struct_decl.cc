#include "semantics/impl.h"
#include "ast/expressions.h"
#include "semantics/type/resolver.h"
#include "semantics/checks/check_struct_decl.h"

Worker_Status sema::check_struct_decl(Semantic_Context *sema, ast::stmt::Variable *variable, bool is_resumption)
{
    auto sym_opt = sema::eagerly_find_symbol_in(sema, sema->current_scope,
                                                variable->name.value);
    MINI_ASSERT(sym_opt.has_value(), );

    sema::Symbol_Proxy symbol = *sym_opt;
    symbol->set_state(sema::Symbol_State::Resolving);

    auto *initializer = variable->initializer;
    auto *strukt = (ast::expr::Struct *)initializer;

    /* ensure all fields are resolvable first */
    for (const auto &field : strukt->fields.iter()) {
        if (field.type_is_defined) {
            auto *typehint = field.typehint;
            auto status    = sema::resolve_typehint(sema, typehint, typehint->locus);
            if (!status.is_done()) return status;
            for (const auto &name : field.names.iter()) {
                sema::link_locus_to_type_at_locus(sema, typehint->locus, name.locus);
            }
        }
    }

    auto field_names = HashMap<mini::StringView, Locus>(sema->allocator);

    Worker_Status status = Worker_Status::Done;

    auto fields = sema::Struct_Type::Fields(sema->persistent_allocator);

    for (const auto &field : strukt->fields.iter()) {
        Struct_Type::Field_Data field_data{};
        field_data.type_id = sema::get_type_at_locus(sema, field.typehint->locus);
        for (const auto &name : field.names.iter()) {
            field_data.name = field.names[0];
            if (field_names.contains(name.value)) {
                sema::report_error(sema,
                                   name.locus,
                                   "redeclaration of field",
                                   "field of same name is already defined",
                                   {{"first defined here", field_names[name.value]}});
                status = Worker_Status::Failed;
            }
            fields.append(field_data);
        }
    }
    if (!status.is_done()) return status;
    sema::Type_Id type_id = sema::register_or_get_type(sema, sema::Type::Struct({symbol->name, symbol->locus}, fields));
    sema::link_locus_to_type(sema, symbol->locus, type_id);
    symbol->type.type_id = type_id;
    symbol->kind = sema::Symbol_Kind::Type;
    symbol->set_state(sema::Symbol_State::Resolved);
    sema->wake_up_workers(symbol.get_id());
    return Worker_Status::Done;
}
