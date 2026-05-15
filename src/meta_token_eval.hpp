#pragma once

#include "ast.hpp"

#include <string>
#include <vector>

namespace ari {

std::vector<Token> substitute_meta_input_tokens(const std::vector<Token>& constructor_tokens,
                                                const std::string& input_name,
                                                const std::vector<Token>& input_tokens);

bool is_supported_meta_token_return_expr(const Expr& expr,
                                         const std::string& input_name,
                                         std::string& reason);

std::vector<Token> evaluate_meta_token_return_expr(const Expr& expr,
                                                   const std::string& input_name,
                                                   const std::vector<Token>& input_tokens);

} // namespace ari
