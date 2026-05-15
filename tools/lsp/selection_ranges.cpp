#include "selection_ranges.hpp"

#include "text_query.hpp"

#include <optional>
#include <sstream>
#include <string>

namespace ari::lsp {
namespace {

std::string range_json(int line, int start, int end) {
    std::ostringstream out;
    out << "{";
    out << "\"start\":{\"line\":" << line << ",\"character\":" << start << "},";
    out << "\"end\":{\"line\":" << line << ",\"character\":" << end << "}";
    out << "}";
    return out.str();
}

std::string selection_json(const TextRange& range, const std::string& line_text) {
    std::ostringstream out;
    out << "{";
    out << "\"range\":" << range_json(range.line, range.start, range.end);
    out << ",\"parent\":{";
    out << "\"range\":" << range_json(range.line, 0, static_cast<int>(line_text.size()));
    out << "}";
    out << "}";
    return out.str();
}

} // namespace

std::string selection_ranges_response(const std::string& id,
                                      const std::string& text,
                                      int line,
                                      int character) {
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    out << "\"result\":[";

    std::string line_text = line_at(text, line);
    std::optional<TextRange> word = word_range_at_position(text, line, character);
    if (word) {
        out << selection_json(*word, line_text);
    } else if (!line_text.empty()) {
        TextRange range{line, 0, static_cast<int>(line_text.size())};
        out << "{\"range\":" << range_json(range.line, range.start, range.end) << "}";
    }

    out << "]}";
    return out.str();
}

} // namespace ari::lsp
