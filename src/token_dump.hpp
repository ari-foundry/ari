#pragma once

#include "token.hpp"

#include <string>
#include <vector>

namespace ari {

std::string dump_tokens(const std::vector<Token>& tokens, const std::string& source_name);

} // namespace ari
