#include "semantics/impl.h"
#include "semantics/type/coercer.h"

enum struct CoerceResult {
    Ok,
    Invalid,
    IntegerSignChange,
    ConstCast,
};

CoerceResult check_type_coercesion(SemanticContext *sema,
                                   sema::TypeId target_id,
                                   sema::TypeId source_id)
{
    sema::TypeStorage &types = sema->types();

    sema::Type &target = types.at_index((usize)target_id);
    sema::Type &source = types.at_index((usize)source_id);

    if (target.kind == source.kind) {
        return CoerceResult::Ok;
    }

    if (target.kind.is_signed_integer() && source.kind.is_signed_integer()) {
        bool can_fit = target.kind >= source.kind;
        if (can_fit) return CoerceResult::Ok;
    }

    if ((target.kind.is_unsigned_integer() &&
         source.kind.is_signed_integer()) ||
        (target.kind.is_signed_integer() &&
         source.kind.is_unsigned_integer())) {
        return CoerceResult::IntegerSignChange;
    }

    if (target.kind == sema::TypeKind::Pointer &&
        source.kind == sema::TypeKind::Pointer)
        MINI_UNREACHABLE("TODO");
    return CoerceResult::Invalid;
}

const char *INVALID_COERCE_REASONS[] = {
    [(int)CoerceResult::IntegerSignChange] =
        "both types are of different signedness",
    [(int)CoerceResult::Invalid] = "incompatible type"};

static void render_coersion_error(SemanticContext *sema, CoerceResult result,
                                  Severity sev, sema::TypeId target_id,
                                  sema::TypeId source_id, Locus target_locus,
                                  Locus source_locus, Mini_Allocator a)
{
    MINI_ASSERT(result != CoerceResult::Ok, );

    sema::TypeStorage &types = sema->types();
    sema::Type &target       = types.at_index((usize)target_id);
    sema::Type &source       = types.at_index((usize)source_id);

    switch (result) {
    case CoerceResult::Invalid: {
        auto diag = diag_create(
            sev, target_locus, "unexpected type",
            mini_string_build(a, "expected `%s`", target.display(a, types)));
        sema::report(
            sema, diag_add_label(
                      diag, Label{mini_string_build(a, "got `%s`",
                                                    source.display(a, types)),
                                  source_locus}));
    } break;
    case CoerceResult::IntegerSignChange: {
        auto diag = diag_create(
            sev, target_locus, "unexpected type",
            mini_string_build(a, "expected `%s`", target.display(a, types)));
        sema::report(
            sema, diag_add_label(
                      diag, Label{mini_string_build(a, "got `%s`",
                                                    source.display(a, types)),
                                  source_locus}));
    } break;
    default:
        MINI_UNREACHABLE("%d", result);
    }
}

bool sema::coerce_type_into(SemanticContext *sema, TypeId target_id,
                            TypeId source_id, Locus target_locus,
                            Locus source_locus, bool strict)
{
    CoerceResult cr = check_type_coercesion(sema, target_id, source_id);

    Severity dlvl = Severity::Warning;
    if (strict)
        dlvl = Severity::Error;

    if (cr != CoerceResult::Ok)
        render_coersion_error(sema, cr, dlvl, target_id, source_id,
                              target_locus, source_locus, sema->allocator);

    if (strict)
        return cr == CoerceResult::Ok;
    return cr != CoerceResult::Invalid;
}

bool sema::try_coerce_type_into(SemanticContext *sema, sema::TypeId target_id, sema::TypeId source_id,
                                bool strict)
{
    CoerceResult cr = check_type_coercesion(sema, target_id, source_id);
    if (strict)
        return cr == CoerceResult::Ok;
    return cr != CoerceResult::Invalid;
}
