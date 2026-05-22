#pragma once

#include "token.hpp"

#include <string>
#include <vector>

namespace ari {

std::vector<Token> lex_source(std::string source);
std::vector<Token> lex_source(std::string source, std::string source_name);

} // namespace ari
