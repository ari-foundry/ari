#pragma once

#include "ast.hpp"

#include <vector>

namespace ari {

struct ItemMacroExpansion {
    std::vector<ConstDecl> constants;
    std::vector<FunctionDecl> functions;
};

ItemMacroExpansion expand_item_macro_items(const ItemMacroInvocation& invocation);

} // namespace ari
