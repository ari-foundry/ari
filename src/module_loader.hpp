#pragma once

#include "ast.hpp"

#include <set>
#include <string>
#include <vector>

namespace ari {

Program parse_file_with_modules(const std::string& input,
                                const std::vector<std::string>& module_search_paths);
Program parse_file_with_modules(const std::string& input,
                                const std::vector<std::string>& module_search_paths,
                                std::set<std::string> cfg_features);

} // namespace ari
