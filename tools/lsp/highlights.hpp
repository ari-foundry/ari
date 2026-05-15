#pragma once

#include <string>

namespace ari::lsp {

std::string document_highlight_response(const std::string& id,
                                        const std::string& text,
                                        int line,
                                        int character);

} // namespace ari::lsp
