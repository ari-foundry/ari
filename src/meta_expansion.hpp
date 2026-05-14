#pragma once

#include "ast.hpp"

#include <vector>

namespace ari {

struct ItemMacroExpansion {
    std::vector<UseDecl> uses;
    std::vector<ModuleDecl> modules;
    std::vector<ConstDecl> constants;
    std::vector<FunctionDecl> functions;
    std::vector<StructDecl> structs;
    std::vector<EnumDecl> enums;
    std::vector<TraitDecl> traits;
    std::vector<ImplDecl> impls;
};

ItemMacroExpansion expand_item_macro_items(const ItemMacroInvocation& invocation);
Pattern expand_pattern_macro_invocation(const Pattern& invocation);
std::vector<ImplDecl> expand_derive_impls_for_struct(const StructDecl& decl);
std::vector<ImplDecl> expand_derive_impls_for_enum(const EnumDecl& decl);

} // namespace ari
