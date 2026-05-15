#include "symbols.hpp"

#include "../ari_tooling/diagnostic.hpp"

#include <cctype>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ari::lsp {
namespace {

struct Symbol {
    std::string name;
    int kind = 12;
    int line = 0;
    int start = 0;
    int end = 0;
};

std::string trim_left(const std::string& text, std::size_t& indent) {
    indent = 0;
    while (indent < text.size() && std::isspace(static_cast<unsigned char>(text[indent]))) ++indent;
    return text.substr(indent);
}

bool identifier_char(char c) {
    unsigned char ch = static_cast<unsigned char>(c);
    return std::isalnum(ch) || c == '_';
}

std::string read_identifier_after(const std::string& line, std::size_t pos) {
    while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) ++pos;
    std::size_t start = pos;
    while (pos < line.size() && identifier_char(line[pos])) ++pos;
    return line.substr(start, pos - start);
}

bool starts_with_word(const std::string& line, const std::string& word) {
    if (line.rfind(word, 0) != 0) return false;
    if (line.size() == word.size()) return true;
    return !identifier_char(line[word.size()]);
}

bool parse_symbol_line(const std::string& line, int line_number, Symbol& symbol) {
    std::size_t indent = 0;
    std::string trimmed = trim_left(line, indent);
    if (starts_with_word(trimmed, "pub")) {
        std::size_t pos = 3;
        while (pos < trimmed.size() && std::isspace(static_cast<unsigned char>(trimmed[pos]))) ++pos;
        trimmed = trimmed.substr(pos);
        indent += pos;
    }
    if (starts_with_word(trimmed, "extern")) {
        std::size_t fn_pos = trimmed.find("fn ");
        if (fn_pos == std::string::npos) return false;
        symbol.name = read_identifier_after(trimmed, fn_pos + 2);
        symbol.kind = 12;
    } else if (starts_with_word(trimmed, "fn")) {
        symbol.name = read_identifier_after(trimmed, 2);
        symbol.kind = 12;
    } else if (starts_with_word(trimmed, "struct")) {
        symbol.name = read_identifier_after(trimmed, 6);
        symbol.kind = 23;
    } else if (starts_with_word(trimmed, "enum")) {
        symbol.name = read_identifier_after(trimmed, 4);
        symbol.kind = 10;
    } else if (starts_with_word(trimmed, "trait")) {
        symbol.name = read_identifier_after(trimmed, 5);
        symbol.kind = 11;
    } else if (starts_with_word(trimmed, "impl")) {
        symbol.name = "impl " + read_identifier_after(trimmed, 4);
        symbol.kind = 5;
    } else if (starts_with_word(trimmed, "mod")) {
        symbol.name = read_identifier_after(trimmed, 3);
        symbol.kind = 2;
    } else {
        return false;
    }
    if (symbol.name.empty()) return false;
    symbol.line = line_number;
    symbol.start = static_cast<int>(indent);
    symbol.end = static_cast<int>(line.size());
    return true;
}

std::vector<Symbol> collect_symbols(const std::string& text) {
    std::vector<Symbol> symbols;
    std::istringstream in(text);
    std::string line;
    int line_number = 0;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        Symbol symbol;
        if (parse_symbol_line(line, line_number, symbol)) symbols.push_back(std::move(symbol));
        ++line_number;
    }
    return symbols;
}

std::string symbol_json(const Symbol& symbol) {
    std::ostringstream out;
    out << "{";
    out << "\"name\":\"" << tooling::json_escape(symbol.name) << "\",";
    out << "\"kind\":" << symbol.kind << ",";
    out << "\"range\":{";
    out << "\"start\":{\"line\":" << symbol.line << ",\"character\":" << symbol.start << "},";
    out << "\"end\":{\"line\":" << symbol.line << ",\"character\":" << symbol.end << "}";
    out << "},";
    out << "\"selectionRange\":{";
    out << "\"start\":{\"line\":" << symbol.line << ",\"character\":" << symbol.start << "},";
    out << "\"end\":{\"line\":" << symbol.line << ",\"character\":" << symbol.end << "}";
    out << "}";
    out << "}";
    return out.str();
}

} // namespace

std::string document_symbols_response(const std::string& id, const std::string& text) {
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    out << "\"result\":[";
    bool first = true;
    for (const Symbol& symbol : collect_symbols(text)) {
        if (!first) out << ",";
        first = false;
        out << symbol_json(symbol);
    }
    out << "]}";
    return out.str();
}

} // namespace ari::lsp
