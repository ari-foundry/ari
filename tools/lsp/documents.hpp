#pragma once

#include "../ari_tooling/diagnostic.hpp"

#include <string>
#include <vector>

namespace ari::lsp {

std::string uri_to_path(const std::string& uri);
std::string path_to_uri(const std::string& path);
std::string diagnostics_notification(const std::string& uri, const std::vector<tooling::Diagnostic>& diagnostics);

} // namespace ari::lsp
