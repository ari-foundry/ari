#pragma once

#include "ast.hpp"

#include <set>
#include <string>
#include <vector>

namespace ari {

struct ModuleMetadataSource {
    std::string module_name;
    std::string path;
    std::string content_hash;
    bool is_root = false;
};

struct ModuleMetadataImport {
    std::string owner_module;
    std::string module_name;
    std::string local_name;
    std::string source_path;
    bool is_public = false;
};

struct ModuleMetadataItem {
    std::string module_name;
    std::string kind;
    std::string name;
    bool is_public = false;
};

struct ModuleMetadata {
    int format_version = 2;
    std::vector<std::string> module_search_paths;
    std::set<std::string> cfg_features;
    bool implicit_std = true;
    std::vector<ModuleMetadataSource> sources;
    std::vector<ModuleMetadataImport> imports;
    std::vector<ModuleMetadataItem> items;
};

void collect_module_metadata_source(ModuleMetadata& metadata,
                                    const std::string& path,
                                    std::string content_hash,
                                    const std::vector<std::string>& module_path,
                                    const Program& program,
                                    bool is_root);

void add_module_metadata_import(ModuleMetadata& metadata,
                                const ModuleImport& import,
                                const std::string& source_path);

std::string module_metadata_source_hash(const std::string& source);
std::string serialize_module_metadata(const ModuleMetadata& metadata);
ModuleMetadata parse_module_metadata_text(const std::string& text, const std::string& display_path);
ModuleMetadata read_module_metadata_file(const std::string& path);
void require_matching_module_metadata(const ModuleMetadata& expected,
                                      const ModuleMetadata& actual,
                                      const std::string& path);

} // namespace ari
