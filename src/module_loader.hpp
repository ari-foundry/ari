#pragma once

#include "ast.hpp"
#include "module_cache.hpp"
#include "module_metadata.hpp"

#include <set>
#include <string>
#include <vector>

namespace ari {

struct ModuleLoadResult {
    Program program;
    ModuleMetadata metadata;
    ModuleCache cache;
};

struct ModuleLoadOptions {
    std::vector<std::string> module_search_paths;
    std::set<std::string> cfg_features;
    const ModuleCache* input_cache = nullptr;
    bool implicit_std = true;
};

Program parse_file_with_modules(const std::string& input,
                                const std::vector<std::string>& module_search_paths);
Program parse_file_with_modules(const std::string& input,
                                const std::vector<std::string>& module_search_paths,
                                std::set<std::string> cfg_features);
ModuleLoadResult parse_file_with_module_metadata(const std::string& input,
                                                 const std::vector<std::string>& module_search_paths,
                                                 std::set<std::string> cfg_features);
ModuleLoadResult parse_file_with_module_metadata(const std::string& input,
                                                 ModuleLoadOptions options);

} // namespace ari
