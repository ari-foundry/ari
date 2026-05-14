#pragma once

#include "ast.hpp"

#include <vector>

namespace ari {

struct ItemMacroFunctionExpansion {
    std::vector<FunctionDecl> functions;
};

ItemMacroFunctionExpansion expand_item_macro_functions(const ItemMacroInvocation& invocation);

} // namespace ari
