#pragma once

#include <string>
#include <vector>

namespace ari::lsp {

struct Symbol {
    std::string name;
    std::string label;
    std::string declaration;
    int kind = 12;
    int line = 0;
    int start = 0;
    int end = 0;
};

std::vector<Symbol> collect_symbols(const std::string& text);
std::string symbol_location_json(const std::string& uri, const Symbol& symbol);

std::string document_symbols_response(const std::string& id, const std::string& text);
std::string hover_response(const std::string& id, const std::string& text, int line, int character);
std::string definition_response(const std::string& id,
                                const std::string& text,
                                const std::string& uri,
                                int line,
                                int character);
std::string completion_response(const std::string& id, const std::string& text, int line, int character);

} // namespace ari::lsp
