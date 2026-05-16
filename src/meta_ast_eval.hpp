#pragma once

#include "ast.hpp"
#include "meta_ast_decl_reflection.hpp"

#include <string>
#include <vector>

namespace ari {

bool is_supported_meta_decl_return_expr(const Expr& expr,
                                        const std::string& input_name,
                                        std::string& reason);

std::vector<Token> evaluate_meta_decl_return_expr(const Expr& expr,
                                                  const std::string& input_name,
                                                  const MetaAstDeclInput& input);

} // namespace ari
