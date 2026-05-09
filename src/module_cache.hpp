#pragma once

#include "module_metadata.hpp"

#include <string>
#include <vector>

namespace ari {

struct ModuleLoadOptions;

struct ModuleCacheSource {
    std::string module_name;
    std::string path;
    std::string content_hash;
    std::string source;
    bool is_root = false;
};

struct ModuleCache {
    int format_version = 1;
    ModuleMetadata metadata;
    std::vector<ModuleCacheSource> sources;
};

std::string serialize_module_cache(const ModuleCache& cache);
ModuleCache parse_module_cache_text(const std::string& text, const std::string& display_path);
ModuleCache read_module_cache_file(const std::string& path);
const ModuleCacheSource* find_module_cache_source(const ModuleCache& cache, const std::string& path);
const ModuleMetadataImport* find_module_cache_import(const ModuleCache& cache, const ModuleImport& import);
void require_matching_module_cache_inputs(const ModuleCache& cache,
                                          const std::string& root_input,
                                          const ModuleLoadOptions& options,
                                          const std::string& display_path);

} // namespace ari
