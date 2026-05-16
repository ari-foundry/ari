#pragma once

#include "meta_ast_decl_reflection.hpp"

#include <string>
#include <vector>

namespace ari {

std::vector<Token> substitute_meta_decl_tokens(const std::vector<Token>& constructor_tokens,
                                               const std::string& input_name,
                                               const MetaAstDeclInput& input);

} // namespace ari
