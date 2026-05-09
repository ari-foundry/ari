#pragma once

#include "common.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace ari {

bool is_qualified_path(const std::string& path);
std::vector<std::string> split_qualified_path(const std::string& path);
std::string join_qualified_path(const std::vector<std::string>& parts);
std::string join_qualified_path(const std::vector<std::string>& parts,
                                std::size_t begin,
                                std::size_t end);
std::string qualified_basename(const std::string& path);
std::string qualified_parent(const std::string& path);
std::string resolve_relative_path(SourceLocation loc,
                                  const std::string& current_module,
                                  const std::string& path);

} // namespace ari
