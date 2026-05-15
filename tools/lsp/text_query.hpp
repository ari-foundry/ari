#pragma once

#include <optional>
#include <string>

namespace ari::lsp {

struct TextRange {
    int line = 0;
    int start = 0;
    int end = 0;
};

bool identifier_char(char c);
std::string line_at(const std::string& text, int target_line);
std::optional<TextRange> word_range_at_position(const std::string& text, int line, int character);
std::string word_at_position(const std::string& text, int line, int character);

} // namespace ari::lsp
