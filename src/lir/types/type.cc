#include "lir/types/type.h"

lir::Type_Builder::Type_Builder(Mini_Allocator allocator, mini::StringView name)
    : name(name), members(allocator)
{

}

void lir::Type_Builder::add_member(lir::Type_Member member)
{
    members.append(member);
}

lir::Type_Member *lir::Type_Builder::get_member(mini::StringView name)
{
    for (auto &member : members.iter()) {
        if (member.name == name) return &member;
    }
    return nullptr;
}

const
lir::Type_Member *lir::Type_Builder::get_member(mini::StringView name) const
{
    for (const auto &member : members.iter()) {
        if (member.name == name) return &member;
    }
    return nullptr;
}
