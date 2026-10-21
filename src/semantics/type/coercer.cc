#include "semantics/type/coercer.h"
#include "semantics/impl.h"

enum struct CoerceResult {
    Ok,
    Invalid,
    IntegerSignChange,
    ConstCast,
    Float_Truncating,
    Int_Float_Promotion,
};

CoerceResult check_type_coercesion(Semantic_Context *sema,
                                   sema::Type_Id target_id,
                                   sema::Type_Id source_id)
{
    sema::Type_Storage &types = sema->types();

    sema::Type &target = types.at_index((usize)target_id);
    sema::Type &source = types.at_index((usize)source_id);

    if (target.kind == source.kind) {
        return CoerceResult::Ok;
    }

    if (target.kind.is_signed_integer() && source.kind.is_signed_integer()) {
        bool can_fit = target.kind >= source.kind;
        if (can_fit)
            return CoerceResult::Ok;
    }

    if ((target.kind.is_unsigned_integer() &&
         source.kind.is_signed_integer()) ||
        (target.kind.is_signed_integer() &&
         source.kind.is_unsigned_integer())) {
        return CoerceResult::IntegerSignChange;
    }

    if (target.kind == sema::Type_Kind::Pointer &&
        source.kind == sema::Type_Kind::Pointer)
        MINI_UNREACHABLE("TODO");

    {
        auto target_is_integer =
            target.kind.is_unsigned_integer() ||
            target.kind.is_signed_integer();
        auto source_is_float =
            (source.kind == sema::Type_Kind::Float32) ||
            (source.kind == sema::Type_Kind::Float64);
        if (target_is_integer && source_is_float) {
            return CoerceResult::Float_Truncating;
        }
    }

    {
        auto source_is_integer =
            source.kind.is_unsigned_integer() ||
            source.kind.is_signed_integer();
        auto target_is_float =
            (target.kind == sema::Type_Kind::Float32) ||
            (target.kind == sema::Type_Kind::Float64);
        if (source_is_integer && target_is_float) {
            return CoerceResult::Int_Float_Promotion;
        }
    }

    return CoerceResult::Invalid;
}

const char *INVALID_COERCE_REASONS[] = {
    [(int)CoerceResult::IntegerSignChange] =
        "both types are of different signedness",
    [(int)CoerceResult::Invalid] = "incompatible type"};

static void render_coersion_error(Semantic_Context *sema, CoerceResult result,
                                  Severity sev, sema::Type_Id target_id,
                                  sema::Type_Id source_id, Locus target_locus,
                                  Locus source_locus, Mini_Allocator a)
{
    MINI_ASSERT(result != CoerceResult::Ok, );

    auto &store  = *sema->store;
    auto &types  = store.types;
    auto &target = types.at_index((usize)target_id);
    auto &source = types.at_index((usize)source_id);

    switch (result) {
    case CoerceResult::Invalid: {
        auto diag = diag_create(
            sev, target_locus, "unexpected type",
            mini_string_build(a, "expected `%s`", target.display(a, store)));
        sema::report(
            sema, diag_add_label(
                      diag, Label{mini_string_build(a, "got `%s`",
                                                    source.display(a, store)),
                                  source_locus}));
    } break;
    case CoerceResult::IntegerSignChange: {
        auto diag = diag_create(
            sev, target_locus, "unexpected type",
            mini_string_build(a, "expected `%s`", target.display(a, store)));
        sema::report(
            sema, diag_add_label(
                      diag, Label{mini_string_build(a, "got `%s`",
                                                    source.display(a, store)),
                                  source_locus}));
    } break;
    case CoerceResult::Float_Truncating: {
        /* todo: add label to show where the `int` was expected */
        auto diag = diag_create(
            sev, source_locus, "float truncating", "casting float to integer may loose precision");
        sema::report(sema, diag);
    } break;

    case CoerceResult::Int_Float_Promotion: {
        /* todo: add label to show where the `float` was expected */
        auto diag = diag_create(
            sev, source_locus, "integer promotion", "implicitly casting integer to float is not allowed");
        sema::report(sema, diag);
    } break;
    default:
        MINI_UNREACHABLE("%d", result);
    }
}

bool sema::coerce_type_into(Semantic_Context *sema, Type_Id target_id,
                            Type_Id source_id, Locus target_locus,
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

bool sema::try_coerce_type_into(Semantic_Context *sema, sema::Type_Id target_id,
                                sema::Type_Id source_id, bool strict)
{
    CoerceResult cr = check_type_coercesion(sema, target_id, source_id);
    if (strict)
        return cr == CoerceResult::Ok;
    return cr != CoerceResult::Invalid;
}
