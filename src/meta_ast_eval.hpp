#pragma once

#include "ast.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace ari {

struct MetaAstDeclInput {
    std::vector<Token> tokens;
    std::size_t count = 0;
    std::string kind = "empty";
    std::string name;
    bool is_public = false;
    std::size_t generic_count = 0;
    std::size_t param_count = 0;
    std::size_t field_count = 0;
    std::size_t case_count = 0;
    std::size_t method_count = 0;
    std::size_t associated_type_count = 0;
};

MetaAstDeclInput summarize_meta_ast_decl_input(const std::vector<Token>& input_tokens,
                                               const Program& parsed_input);

bool is_supported_meta_decl_return_expr(const Expr& expr,
                                        const std::string& input_name,
                                        std::string& reason);

std::vector<Token> evaluate_meta_decl_return_expr(const Expr& expr,
                                                  const std::string& input_name,
                                                  const MetaAstDeclInput& input);

} // namespace ari
