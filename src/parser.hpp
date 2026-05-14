#pragma once

#include "ast.hpp"
#include "token.hpp"

#include <set>
#include <string>
#include <vector>

namespace ari {

Program parse_tokens(std::vector<Token> tokens);
Program parse_tokens(std::vector<Token> tokens,
                     std::set<std::string> cfg_features,
                     std::string target_triple = {});
Program parse_tokens_in_module(std::vector<Token> tokens, std::vector<std::string> module_path);
Program parse_tokens_in_module(std::vector<Token> tokens,
                               std::vector<std::string> module_path,
                               std::set<std::string> cfg_features,
                               std::string target_triple = {});
std::vector<ExprPtr> parse_macro_argument_expressions(std::vector<Token> tokens, SourceLocation loc);
ExprPtr parse_macro_expression(std::vector<Token> tokens, SourceLocation loc);
TypeRef parse_macro_type_ref(std::vector<Token> tokens, SourceLocation loc);
Pattern parse_macro_pattern(std::vector<Token> tokens, SourceLocation loc);

} // namespace ari
