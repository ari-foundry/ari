#pragma once

#include "ast.hpp"

#include <string>

namespace ari {

bool is_supported_meta_ast_expression_condition(const Expr& expr,
                                                const std::string& input_name,
                                                std::string& reason);
bool is_meta_ast_expression_value_helper(const Expr& expr, const std::string& input_name);
std::string meta_ast_expression_value_helper_scope_message();

ExprPtr expand_meta_ast_expression_return(const Expr& returned_ast,
                                          const std::string& input_name,
                                          const Expr& input_ast,
                                          const std::string& hygiene_prefix);

} // namespace ari
