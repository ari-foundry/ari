#pragma once

#include <string>

namespace ari::lsp {

bool identifier_char(char c);
std::string line_at(const std::string& text, int target_line);
std::string word_at_position(const std::string& text, int line, int character);

} // namespace ari::lsp
