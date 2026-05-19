#pragma once

#include "ast.hpp"

#include <string>
#include <vector>

namespace ari {

struct ItemMacroExpansion {
    std::vector<UseDecl> uses;
    std::vector<ModuleDecl> modules;
    std::vector<ConstDecl> constants;
    std::vector<TypeAliasDecl> type_aliases;
    std::vector<FunctionDecl> functions;
    std::vector<StructDecl> structs;
    std::vector<EnumDecl> enums;
    std::vector<TraitDecl> traits;
    std::vector<ImplDecl> impls;
};

ItemMacroExpansion expand_item_macro_items(const ItemMacroInvocation& invocation);
ItemMacroExpansion expand_item_macro_token_return(const ItemMacroInvocation& invocation,
                                                  const std::string& input_name,
                                                  const Expr& returned_tokens);
ItemMacroExpansion expand_item_macro_decl_constructor(const ItemMacroInvocation& invocation,
                                                      const std::string& input_name,
                                                      const Expr& returned_ast);
ItemMacroExpansion expand_attribute_macro_token_return(const std::vector<Token>& declaration_tokens,
                                                       const Attribute& attr,
                                                       const std::string& module_name,
                                                       SourceLocation loc,
                                                       const std::string& input_name,
                                                       const Expr& returned_tokens);
ItemMacroExpansion expand_attribute_macro_decl_constructor(const std::vector<Token>& declaration_tokens,
                                                           const std::string& module_name,
                                                           SourceLocation loc,
                                                           const std::string& input_name,
                                                           const Expr& returned_ast);
Pattern expand_pattern_macro_invocation(const Pattern& invocation);
Pattern expand_pattern_macro_token_return(const Pattern& invocation,
                                          const std::string& input_name,
                                          const Expr& returned_tokens);
Pattern expand_pattern_macro_constructor(const Pattern& invocation,
                                         const std::string& input_name,
                                         const Expr& returned_ast);
TypeRef expand_type_macro_invocation(const TypeRef& invocation);
TypeRef expand_type_macro_constructor(const TypeRef& invocation,
                                      const std::string& input_name,
                                      const Expr& returned_type);
ExprPtr expand_expression_macro_token_return(const std::vector<Token>& invocation_tokens,
                                             SourceLocation invocation_loc,
                                             const std::string& input_name,
                                             const Expr& returned_tokens);
ExprPtr expand_ast_expression_return(const Expr& returned_ast,
                                      const std::string& input_name,
                                      const Expr& input_ast,
                                      SourceLocation invocation_loc);
std::vector<ImplDecl> expand_derive_impls_for_struct(const StructDecl& decl);
std::vector<ImplDecl> expand_derive_impls_for_enum(const EnumDecl& decl);

} // namespace ari
