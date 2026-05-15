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
    std::string label;
    std::string declaration;
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
        symbol.label = "function";
        symbol.kind = 12;
    } else if (starts_with_word(trimmed, "fn")) {
        symbol.name = read_identifier_after(trimmed, 2);
        symbol.label = "function";
        symbol.kind = 12;
    } else if (starts_with_word(trimmed, "struct")) {
        symbol.name = read_identifier_after(trimmed, 6);
        symbol.label = "struct";
        symbol.kind = 23;
    } else if (starts_with_word(trimmed, "enum")) {
        symbol.name = read_identifier_after(trimmed, 4);
        symbol.label = "enum";
        symbol.kind = 10;
    } else if (starts_with_word(trimmed, "trait")) {
        symbol.name = read_identifier_after(trimmed, 5);
        symbol.label = "trait";
        symbol.kind = 11;
    } else if (starts_with_word(trimmed, "impl")) {
        symbol.name = "impl " + read_identifier_after(trimmed, 4);
        symbol.label = "impl";
        symbol.kind = 5;
    } else if (starts_with_word(trimmed, "mod")) {
        symbol.name = read_identifier_after(trimmed, 3);
        symbol.label = "module";
        symbol.kind = 2;
    } else {
        return false;
    }
    if (symbol.name.empty()) return false;
    symbol.declaration = trimmed;
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

const Symbol* find_symbol_for_word(const std::vector<Symbol>& symbols, const std::string& word) {
    for (const Symbol& symbol : symbols) {
        if (symbol.name == word) return &symbol;
        if (symbol.name.rfind("impl ", 0) == 0 && symbol.name.substr(5) == word) return &symbol;
    }
    return nullptr;
}

std::string location_json(const std::string& uri, const Symbol& symbol) {
    std::ostringstream out;
    out << "{";
    out << "\"uri\":\"" << tooling::json_escape(uri) << "\",";
    out << "\"range\":{";
    out << "\"start\":{\"line\":" << symbol.line << ",\"character\":" << symbol.start << "},";
    out << "\"end\":{\"line\":" << symbol.line << ",\"character\":" << symbol.end << "}";
    out << "}";
    out << "}";
    return out.str();
}

int completion_kind(const Symbol& symbol) {
    if (symbol.label == "function") return 3;
    if (symbol.label == "module") return 9;
    if (symbol.label == "struct") return 22;
    if (symbol.label == "enum") return 13;
    if (symbol.label == "trait") return 8;
    if (symbol.label == "impl") return 6;
    return 6;
}

bool matches_prefix(const Symbol& symbol, const std::string& prefix) {
    if (prefix.empty()) return true;
    if (symbol.name.rfind(prefix, 0) == 0) return true;
    return symbol.name.rfind("impl ", 0) == 0 && symbol.name.substr(5).rfind(prefix, 0) == 0;
}

std::string completion_item_json(const Symbol& symbol) {
    std::ostringstream out;
    out << "{";
    out << "\"label\":\"" << tooling::json_escape(symbol.name) << "\",";
    out << "\"kind\":" << completion_kind(symbol) << ",";
    out << "\"detail\":\"Ari " << tooling::json_escape(symbol.label) << "\"";
    if (!symbol.declaration.empty()) {
        out << ",\"documentation\":{";
        out << "\"kind\":\"markdown\",";
        out << "\"value\":\"" << tooling::json_escape("```ari\n" + symbol.declaration + "\n```") << "\"";
        out << "}";
    }
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

std::string hover_response(const std::string& id, const std::string& text, int line, int character) {
    std::string word = word_at_position(text, line, character);
    const std::vector<Symbol> symbols = collect_symbols(text);
    const Symbol* symbol = find_symbol_for_word(symbols, word);
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    if (!symbol) {
        out << "\"result\":null";
    } else {
        std::string value = "Ari " + symbol->label + " `" + symbol->name + "`";
        if (!symbol->declaration.empty()) {
            value += "\n\n```ari\n" + symbol->declaration + "\n```";
        }
        out << "\"result\":{";
        out << "\"contents\":{\"kind\":\"markdown\",\"value\":\"" << tooling::json_escape(value) << "\"}";
        out << "}";
    }
    out << "}";
    return out.str();
}

std::string definition_response(const std::string& id,
                                const std::string& text,
                                const std::string& uri,
                                int line,
                                int character) {
    std::string word = word_at_position(text, line, character);
    const std::vector<Symbol> symbols = collect_symbols(text);
    const Symbol* symbol = find_symbol_for_word(symbols, word);
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    if (!symbol) {
        out << "\"result\":null";
    } else {
        out << "\"result\":" << location_json(uri, *symbol);
    }
    out << "}";
    return out.str();
}

std::string completion_response(const std::string& id, const std::string& text, int line, int character) {
    std::string prefix = word_at_position(text, line, character);
    const std::vector<Symbol> symbols = collect_symbols(text);
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    out << "\"result\":{";
    out << "\"isIncomplete\":false,";
    out << "\"items\":[";
    bool first = true;
    for (const Symbol& symbol : symbols) {
        if (!matches_prefix(symbol, prefix)) continue;
        if (!first) out << ",";
        first = false;
        out << completion_item_json(symbol);
    }
    out << "]}";
    out << "}";
    return out.str();
}

} // namespace ari::lsp
