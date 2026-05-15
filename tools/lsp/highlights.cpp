#include "highlights.hpp"

#include "text_query.hpp"

#include <sstream>
#include <string>

namespace ari::lsp {
namespace {

bool is_identifier_match(const std::string& line, std::size_t start, const std::string& word) {
    if (start > 0 && identifier_char(line[start - 1])) return false;
    std::size_t end = start + word.size();
    return end >= line.size() || !identifier_char(line[end]);
}

std::string highlight_json(int line, int start, int end) {
    std::ostringstream out;
    out << "{";
    out << "\"range\":{";
    out << "\"start\":{\"line\":" << line << ",\"character\":" << start << "},";
    out << "\"end\":{\"line\":" << line << ",\"character\":" << end << "}";
    out << "},";
    out << "\"kind\":1";
    out << "}";
    return out.str();
}

} // namespace

std::string document_highlight_response(const std::string& id,
                                        const std::string& text,
                                        int line,
                                        int character) {
    std::string word = word_at_position(text, line, character);

    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    out << "\"result\":[";
    bool first = true;
    if (!word.empty()) {
        std::istringstream in(text);
        std::string current_line;
        int line_number = 0;
        while (std::getline(in, current_line)) {
            if (!current_line.empty() && current_line.back() == '\r') current_line.pop_back();
            std::size_t pos = current_line.find(word);
            while (pos != std::string::npos) {
                if (is_identifier_match(current_line, pos, word)) {
                    if (!first) out << ",";
                    first = false;
                    out << highlight_json(line_number, static_cast<int>(pos), static_cast<int>(pos + word.size()));
                }
                pos = current_line.find(word, pos + word.size());
            }
            ++line_number;
        }
    }
    out << "]}";
    return out.str();
}

} // namespace ari::lsp
