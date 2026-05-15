#pragma once

#include <string>

namespace ari::lsp {

std::string workspace_symbols_response(const std::string& id,
                                       const std::string& root_path,
                                       const std::string& query);

} // namespace ari::lsp
