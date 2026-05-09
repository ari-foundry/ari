#pragma once

#include "ast.hpp"
#include "token.hpp"

#include <set>
#include <string>
#include <vector>

namespace ari {

Program parse_tokens(std::vector<Token> tokens);
Program parse_tokens(std::vector<Token> tokens, std::set<std::string> cfg_features);
Program parse_tokens_in_module(std::vector<Token> tokens, std::vector<std::string> module_path);
Program parse_tokens_in_module(std::vector<Token> tokens,
                               std::vector<std::string> module_path,
                               std::set<std::string> cfg_features);
std::vector<ExprPtr> parse_macro_argument_expressions(std::vector<Token> tokens, SourceLocation loc);

} // namespace ari
