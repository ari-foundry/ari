#include "text_query.hpp"

#include <cctype>
#include <sstream>

namespace ari::lsp {

bool identifier_char(char c) {
    unsigned char ch = static_cast<unsigned char>(c);
    return std::isalnum(ch) || c == '_';
}

std::string line_at(const std::string& text, int target_line) {
    std::istringstream in(text);
    std::string line;
    int line_number = 0;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line_number == target_line) return line;
        ++line_number;
    }
    return "";
}

std::string word_at_position(const std::string& text, int line, int character) {
    std::string value = line_at(text, line);
    if (value.empty()) return "";
    if (character < 0) character = 0;
    std::size_t pos = static_cast<std::size_t>(character);
    if (pos >= value.size()) pos = value.size() - 1;
    if (!identifier_char(value[pos]) && pos > 0 && identifier_char(value[pos - 1])) --pos;
    if (!identifier_char(value[pos])) return "";
    std::size_t start = pos;
    while (start > 0 && identifier_char(value[start - 1])) --start;
    std::size_t end = pos + 1;
    while (end < value.size() && identifier_char(value[end])) ++end;
    return value.substr(start, end - start);
}

} // namespace ari::lsp
