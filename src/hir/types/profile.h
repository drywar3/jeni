#pragma once

namespace hir
{
    struct Profile {
        Mini_String name;
        Mini_String canonical_name;
        Mini_String repr_name;

        constexpr Profile() : name(), canonical_name(), repr_name() {}
    };
} // namespace hir
