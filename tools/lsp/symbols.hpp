#pragma once

#include <string>

namespace ari::lsp {

std::string document_symbols_response(const std::string& id, const std::string& text);
std::string hover_response(const std::string& id, const std::string& text, int line, int character);

} // namespace ari::lsp
