#pragma once

#include <string>

namespace ari::lsp {

std::string code_actions_response(const std::string& id,
                                  const std::string& uri,
                                  const std::string& text);

} // namespace ari::lsp
