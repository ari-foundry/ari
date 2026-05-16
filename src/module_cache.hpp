#pragma once

#include "module_metadata.hpp"

#include <cstdint>
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

struct ModuleCacheAstSummary {
    std::string module_name;
    std::string path;
    std::string content_hash;
    std::string declaration_hash;
    std::string declaration_summary;
    bool is_root = false;
    std::uint64_t use_count = 0;
    std::uint64_t module_import_count = 0;
    std::uint64_t module_decl_count = 0;
    std::uint64_t constant_count = 0;
    std::uint64_t function_count = 0;
    std::uint64_t struct_count = 0;
    std::uint64_t enum_count = 0;
    std::uint64_t trait_count = 0;
    std::uint64_t impl_count = 0;
};

struct ModuleCacheIrSummary {
    std::string module_name;
    std::string path;
    std::string content_hash;
    std::string ir_hash;
    std::string ir_summary;
    bool is_root = false;
    std::uint64_t function_count = 0;
};

struct ModuleCache {
    int format_version = 0;
    ModuleMetadata metadata;
    std::vector<ModuleCacheSource> sources;
    std::vector<ModuleCacheAstSummary> ast_summaries;
    std::vector<ModuleCacheIrSummary> ir_summaries;
};

std::string serialize_module_cache(const ModuleCache& cache);
ModuleCache parse_module_cache_text(const std::string& text, const std::string& display_path);
ModuleCache read_module_cache_file(const std::string& path);
const ModuleCacheSource* find_module_cache_source(const ModuleCache& cache, const std::string& path);
const ModuleCacheAstSummary* find_module_cache_ast_summary(const ModuleCache& cache,
                                                           const std::string& path);
const ModuleCacheIrSummary* find_module_cache_ir_summary(const ModuleCache& cache,
                                                         const std::string& path);
void require_matching_module_cache_ast_summary(const ModuleCacheAstSummary& expected,
                                               const ModuleCacheAstSummary& actual);
const ModuleMetadataImport* find_module_cache_import(const ModuleCache& cache, const ModuleImport& import);
void require_matching_module_cache_inputs(const ModuleCache& cache,
                                          const std::string& root_input,
                                          const ModuleLoadOptions& options,
                                          const std::string& display_path);

} // namespace ari
