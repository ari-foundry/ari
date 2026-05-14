#pragma once

#include "ast.hpp"

#include <vector>

namespace ari {

struct ItemMacroExpansion {
    std::vector<ConstDecl> constants;
    std::vector<FunctionDecl> functions;
    std::vector<StructDecl> structs;
    std::vector<EnumDecl> enums;
    std::vector<TraitDecl> traits;
    std::vector<ImplDecl> impls;
};

ItemMacroExpansion expand_item_macro_items(const ItemMacroInvocation& invocation);

} // namespace ari
