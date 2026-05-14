#include "module_cache.hpp"

#include "common.hpp"
#include "module_ast_summary.hpp"
#include "module_cache_format.hpp"
#include "module_loader.hpp"
#include "module_path.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace ari {

namespace {

std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw CompileError("cannot open input file '" + path + "'");
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool file_exists(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return static_cast<bool>(in);
}

std::string dirname(const std::string& path) {
    std::size_t split = path.find_last_of("/\\");
    if (split == std::string::npos) return ".";
    if (split == 0) return path.substr(0, 1);
    return path.substr(0, split);
}

std::string path_join(const std::string& left, const std::string& right) {
    if (left.empty() || left == ".") return right;
    char back = left[left.size() - 1];
    if (back == '/' || back == '\\') return left + right;
    return left + "/" + right;
}

void add_module_candidates(const std::string& dir,
                           const std::string& local_name,
                           std::vector<std::string>& candidates) {
    candidates.push_back(path_join(dir, local_name + ".ari"));
    candidates.push_back(path_join(dir, local_name + ".arih"));
    candidates.push_back(path_join(path_join(dir, local_name), "mod.ari"));
    candidates.push_back(path_join(path_join(dir, local_name), "mod.arih"));
}

std::string find_module_file_for_cache(const ModuleMetadataImport& import,
                                       const std::string& base_dir,
                                       const std::vector<std::string>& module_search_paths) {
    std::vector<std::string> candidates;
    add_module_candidates(base_dir, import.local_name, candidates);
    if (!import.owner_module.empty()) {
        add_module_candidates(
            path_join(base_dir, qualified_basename(import.owner_module)),
            import.local_name,
            candidates);
    }
    for (const auto& search_path : module_search_paths) {
        if (!search_path.empty()) add_module_candidates(search_path, import.local_name, candidates);
    }
    for (const auto& candidate : candidates) {
        if (file_exists(candidate)) return candidate;
    }
    return "";
}

std::string escape_field(const std::string& text) {
    std::string escaped;
    for (char c : text) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '\t': escaped += "\\t"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            default: escaped.push_back(c); break;
        }
    }
    return escaped;
}

std::string unescape_field(const std::string& text, const std::string& display_path, std::size_t line) {
    std::string unescaped;
    for (std::size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c != '\\') {
            unescaped.push_back(c);
            continue;
        }
        if (i + 1 >= text.size()) {
            throw CompileError("invalid module cache '" + display_path + "' at line " +
                               std::to_string(line) + ": dangling escape");
        }
        char next = text[++i];
        switch (next) {
            case '\\': unescaped.push_back('\\'); break;
            case 't': unescaped.push_back('\t'); break;
            case 'n': unescaped.push_back('\n'); break;
            case 'r': unescaped.push_back('\r'); break;
            default:
                throw CompileError("invalid module cache '" + display_path + "' at line " +
                                   std::to_string(line) + ": unknown escape");
        }
    }
    return unescaped;
}

std::vector<std::string> split_cache_line(const std::string& line,
                                          const std::string& display_path,
                                          std::size_t line_number) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    for (;;) {
        std::size_t split = line.find('\t', start);
        std::string field = split == std::string::npos
            ? line.substr(start)
            : line.substr(start, split - start);
        fields.push_back(unescape_field(field, display_path, line_number));
        if (split == std::string::npos) break;
        start = split + 1;
    }
    return fields;
}

bool parse_bool_field(const std::string& value, const std::string& display_path, std::size_t line) {
    if (value == "0") return false;
    if (value == "1") return true;
    throw CompileError("invalid module cache '" + display_path + "' at line " +
                       std::to_string(line) + ": expected boolean 0 or 1");
}

void write_line(std::ostringstream& out, const std::vector<std::string>& fields) {
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) out << '\t';
        out << escape_field(fields[i]);
    }
    out << '\n';
}

std::string bool_key(bool value) {
    return value ? "1" : "0";
}

std::string display_module_name(const std::string& module_name) {
    return module_name.empty() ? "<root>" : module_name;
}

void fail_stale(const std::string& display_path, const std::string& detail) {
    throw CompileError("module cache '" + display_path + "' is stale: " + detail +
                       "; regenerate it with --emit-module-cache");
}

void require_same_search_paths(const ModuleCache& cache,
                               const ModuleLoadOptions& options,
                               const std::string& display_path) {
    if (cache.metadata.module_search_paths.size() != options.module_search_paths.size()) {
        fail_stale(display_path, "module search path count changed");
    }
    for (std::size_t i = 0; i < options.module_search_paths.size(); ++i) {
        if (cache.metadata.module_search_paths[i] != options.module_search_paths[i]) {
            fail_stale(display_path,
                       "module search path #" + std::to_string(i + 1) + " changed from '" +
                       cache.metadata.module_search_paths[i] + "' to '" +
                       options.module_search_paths[i] + "'");
        }
    }
}

void require_same_cfg(const ModuleCache& cache,
                      const ModuleLoadOptions& options,
                      const std::string& display_path) {
    for (const auto& feature : options.cfg_features) {
        if (!cache.metadata.cfg_features.count(feature)) {
            fail_stale(display_path, "current source graph has new cfg feature '" + feature + "'");
        }
    }
    for (const auto& feature : cache.metadata.cfg_features) {
        if (!options.cfg_features.count(feature)) {
            fail_stale(display_path, "module cache still lists cfg feature '" + feature +
                       "' missing from the current source graph");
        }
    }
}

void require_same_target(const ModuleCache& cache,
                         const ModuleLoadOptions& options,
                         const std::string& display_path) {
    if (cache.metadata.target_triple != options.target_triple) {
        fail_stale(display_path, "target option changed from '" +
                   cache.metadata.target_triple + "' to '" + options.target_triple + "'");
    }
}

std::string source_key(const std::string& module_name, const std::string& path, bool is_root) {
    return module_name + "\t" + path + "\t" + bool_key(is_root);
}

std::string source_display(const std::string& module_name, const std::string& path) {
    return "'" + display_module_name(module_name) + "' at '" + path + "'";
}

std::map<std::string, const ModuleCacheSource*> cache_sources_by_key(const ModuleCache& cache) {
    std::map<std::string, const ModuleCacheSource*> by_key;
    for (const auto& source : cache.sources) {
        by_key.emplace(source_key(source.module_name, source.path, source.is_root), &source);
    }
    return by_key;
}

std::map<std::string, const ModuleCacheAstSummary*> cache_ast_summaries_by_key(const ModuleCache& cache) {
    std::map<std::string, const ModuleCacheAstSummary*> by_key;
    for (const auto& summary : cache.ast_summaries) {
        by_key.emplace(source_key(summary.module_name, summary.path, summary.is_root), &summary);
    }
    return by_key;
}

std::string count_key(std::uint64_t value) {
    return std::to_string(value);
}

std::uint64_t parse_count_field(const std::string& value,
                                const std::string& display_path,
                                std::size_t line) {
    if (value.empty()) {
        throw CompileError("invalid module cache '" + display_path + "' at line " +
                           std::to_string(line) + ": expected non-negative count");
    }
    std::uint64_t result = 0;
    for (char c : value) {
        if (c < '0' || c > '9') {
            throw CompileError("invalid module cache '" + display_path + "' at line " +
                               std::to_string(line) + ": expected non-negative count");
        }
        std::uint64_t digit = static_cast<std::uint64_t>(c - '0');
        if (result > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
            throw CompileError("invalid module cache '" + display_path + "' at line " +
                               std::to_string(line) + ": count is too large");
        }
        result = result * 10 + digit;
    }
    return result;
}

} // namespace

std::string serialize_module_cache(const ModuleCache& cache) {
    std::ostringstream out;
    out << kModuleCacheHeader << "\n";
    write_line(out, {"metadata", serialize_module_metadata(cache.metadata)});
    for (const auto& source : cache.sources) {
        write_line(out, {
            "source",
            source.module_name,
            source.path,
            source.is_root ? "1" : "0",
            source.content_hash,
            source.source,
        });
    }
    for (const auto& summary : cache.ast_summaries) {
        write_line(out, {
            "ast-summary",
            summary.module_name,
            summary.path,
            summary.is_root ? "1" : "0",
            summary.content_hash,
            summary.declaration_hash,
            summary.declaration_summary,
            count_key(summary.use_count),
            count_key(summary.module_import_count),
            count_key(summary.module_decl_count),
            count_key(summary.constant_count),
            count_key(summary.function_count),
            count_key(summary.struct_count),
            count_key(summary.enum_count),
            count_key(summary.trait_count),
            count_key(summary.impl_count),
        });
    }
    return out.str();
}

ModuleCache parse_module_cache_text(const std::string& text, const std::string& display_path) {
    std::istringstream in(text);
    std::string line;
    std::size_t line_number = 0;
    ModuleCache cache;
    bool saw_header = false;
    bool saw_metadata = false;
    std::set<std::string> seen_sources;
    std::set<std::string> seen_ast_summaries;

    while (std::getline(in, line)) {
        ++line_number;
        if (!saw_header) {
            if (line != kModuleCacheHeader) {
                throw CompileError("invalid module cache '" + display_path +
                                   "': expected " + std::string(kModuleCacheHeader) + " header");
            }
            cache.format_version = kModuleCacheFormatVersion;
            saw_header = true;
            continue;
        }
        if (line.empty()) continue;
        std::vector<std::string> fields = split_cache_line(line, display_path, line_number);
        const std::string& tag = fields[0];
        if (tag == "metadata") {
            if (fields.size() != 2) {
                throw CompileError("invalid module cache '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed metadata record");
            }
            if (saw_metadata) {
                throw CompileError("invalid module cache '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": duplicate metadata record");
            }
            cache.metadata = parse_module_metadata_text(fields[1], display_path + ":metadata");
            saw_metadata = true;
        } else if (tag == "source") {
            if (fields.size() != 6) {
                throw CompileError("invalid module cache '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed source record");
            }
            bool is_root = parse_bool_field(fields[3], display_path, line_number);
            std::string key = source_key(fields[1], fields[2], is_root);
            if (!seen_sources.insert(key).second) {
                throw CompileError("invalid module cache '" + display_path + "' at line " +
                                   std::to_string(line_number) +
                                   ": duplicate source record for module '" +
                                   display_module_name(fields[1]) + "' at '" + fields[2] + "'");
            }
            cache.sources.push_back(ModuleCacheSource{
                fields[1],
                fields[2],
                fields[4],
                fields[5],
                is_root,
            });
        } else if (tag == "ast-summary") {
            if (fields.size() != 16) {
                throw CompileError("invalid module cache '" + display_path + "' at line " +
                                   std::to_string(line_number) + ": malformed ast-summary record");
            }
            const std::size_t count_offset = 7;
            bool is_root = parse_bool_field(fields[3], display_path, line_number);
            std::string key = source_key(fields[1], fields[2], is_root);
            if (!seen_ast_summaries.insert(key).second) {
                throw CompileError("invalid module cache '" + display_path + "' at line " +
                                   std::to_string(line_number) +
                                   ": duplicate ast-summary record for module '" +
                                   display_module_name(fields[1]) + "' at '" + fields[2] + "'");
            }
            ModuleCacheAstSummary summary{
                fields[1],
                fields[2],
                fields[4],
                fields[5],
                fields[6],
                is_root,
                parse_count_field(fields[count_offset], display_path, line_number),
                parse_count_field(fields[count_offset + 1], display_path, line_number),
                parse_count_field(fields[count_offset + 2], display_path, line_number),
                parse_count_field(fields[count_offset + 3], display_path, line_number),
                parse_count_field(fields[count_offset + 4], display_path, line_number),
                parse_count_field(fields[count_offset + 5], display_path, line_number),
                parse_count_field(fields[count_offset + 6], display_path, line_number),
                parse_count_field(fields[count_offset + 7], display_path, line_number),
                parse_count_field(fields[count_offset + 8], display_path, line_number),
            };
            require_valid_module_cache_ast_summary_payload(summary, display_path);
            cache.ast_summaries.push_back(std::move(summary));
        } else {
            throw CompileError("invalid module cache '" + display_path + "' at line " +
                               std::to_string(line_number) + ": unknown record");
        }
    }

    if (!saw_header) {
        throw CompileError("invalid module cache '" + display_path +
                           "': expected " + std::string(kModuleCacheHeader) + " header");
    }
    if (!saw_metadata) {
        throw CompileError("invalid module cache '" + display_path + "': missing metadata record");
    }
    return cache;
}

ModuleCache read_module_cache_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw CompileError("cannot open module cache file '" + path + "'");
    std::ostringstream ss;
    ss << in.rdbuf();
    return parse_module_cache_text(ss.str(), path);
}

const ModuleCacheSource* find_module_cache_source(const ModuleCache& cache, const std::string& path) {
    for (const auto& source : cache.sources) {
        if (source.path == path) return &source;
    }
    return nullptr;
}

const ModuleCacheAstSummary* find_module_cache_ast_summary(const ModuleCache& cache,
                                                           const std::string& path) {
    for (const auto& summary : cache.ast_summaries) {
        if (summary.path == path) return &summary;
    }
    return nullptr;
}

void require_matching_module_cache_ast_summary(const ModuleCacheAstSummary& expected,
                                               const ModuleCacheAstSummary& actual) {
    auto fail = [&](const std::string& detail) {
        throw CompileError("module cache AST summary for " +
                           source_display(actual.module_name, actual.path) +
                           " does not match parsed source: " + detail +
                           "; regenerate it with --emit-module-cache");
    };
    if (expected.module_name != actual.module_name) fail("module name changed");
    if (expected.path != actual.path) fail("source path changed");
    if (expected.is_root != actual.is_root) fail("root-source flag changed");
    if (expected.content_hash != actual.content_hash) fail("content hash changed");
    if (expected.declaration_hash != actual.declaration_hash) {
        fail("declaration hash changed from '" + expected.declaration_hash +
             "' to '" + actual.declaration_hash + "'");
    }
    if (expected.declaration_summary != actual.declaration_summary) {
        fail("declaration summary changed");
    }
#define ARI_CHECK_SUMMARY_COUNT(field, label) \
    if (expected.field != actual.field) { \
        fail(std::string(label) + " count changed from " + \
             std::to_string(expected.field) + " to " + std::to_string(actual.field)); \
    }
    ARI_CHECK_SUMMARY_COUNT(use_count, "use");
    ARI_CHECK_SUMMARY_COUNT(module_import_count, "module import");
    ARI_CHECK_SUMMARY_COUNT(module_decl_count, "module declaration");
    ARI_CHECK_SUMMARY_COUNT(constant_count, "constant");
    ARI_CHECK_SUMMARY_COUNT(function_count, "function");
    ARI_CHECK_SUMMARY_COUNT(struct_count, "struct");
    ARI_CHECK_SUMMARY_COUNT(enum_count, "enum");
    ARI_CHECK_SUMMARY_COUNT(trait_count, "trait");
    ARI_CHECK_SUMMARY_COUNT(impl_count, "impl");
#undef ARI_CHECK_SUMMARY_COUNT
}

const ModuleMetadataImport* find_module_cache_import(const ModuleCache& cache, const ModuleImport& import) {
    for (const auto& cached : cache.metadata.imports) {
        if (cached.owner_module == import.module_name &&
            cached.module_name == import.name &&
            cached.local_name == import.local_name &&
            cached.is_public == import.is_public) {
            return &cached;
        }
    }
    return nullptr;
}

void require_matching_module_cache_inputs(const ModuleCache& cache,
                                          const std::string& root_input,
                                          const ModuleLoadOptions& options,
                                          const std::string& display_path) {
    if (cache.format_version != kModuleCacheFormatVersion) {
        fail_stale(display_path, "unsupported module cache format");
    }
    if (cache.metadata.format_version != kModuleCacheFormatVersion) {
        fail_stale(display_path, "embedded metadata format is not v0");
    }
    require_same_search_paths(cache, options, display_path);
    require_same_cfg(cache, options, display_path);
    require_same_target(cache, options, display_path);
    if (cache.metadata.implicit_std != options.implicit_std) {
        fail_stale(display_path, "implicit_std option changed from " +
                   bool_key(cache.metadata.implicit_std) + " to " +
                   bool_key(options.implicit_std));
    }

    const auto by_key = cache_sources_by_key(cache);
    const auto summaries_by_key = cache_ast_summaries_by_key(cache);
    std::set<std::string> metadata_source_keys;
    std::map<std::string, std::string> source_path_by_module;
    bool matched_root = false;
    for (const auto& source : cache.metadata.sources) {
        metadata_source_keys.insert(source_key(source.module_name, source.path, source.is_root));
        if (source.is_root) {
            matched_root = matched_root || source.path == root_input;
        }
        source_path_by_module[source.module_name] = source.path;

        auto cached_it = by_key.find(source_key(source.module_name, source.path, source.is_root));
        if (cached_it == by_key.end()) {
            fail_stale(display_path, "missing cached source '" +
                       display_module_name(source.module_name) + "' at '" + source.path + "'");
        }
        const ModuleCacheSource& cached = *cached_it->second;
        if (cached.content_hash != source.content_hash) {
            fail_stale(display_path, "cached source '" + display_module_name(source.module_name) +
                       "' hash does not match embedded metadata");
        }
        std::string cached_hash = module_metadata_source_hash(cached.source);
        if (cached_hash != cached.content_hash) {
            fail_stale(display_path, "cached source text for '" +
                       display_module_name(source.module_name) + "' hashes to '" +
                       cached_hash + "' instead of recorded '" + cached.content_hash + "'");
        }
        auto summary_it = summaries_by_key.find(source_key(source.module_name, source.path, source.is_root));
        if (summary_it == summaries_by_key.end()) {
            fail_stale(display_path, "missing AST summary for cached source " +
                       source_display(source.module_name, source.path));
        }
        const ModuleCacheAstSummary& summary = *summary_it->second;
        if (summary.content_hash != source.content_hash) {
            fail_stale(display_path, "AST summary for cached source " +
                       source_display(source.module_name, source.path) +
                       " hash does not match embedded metadata");
        }

        std::string current = read_file(source.path);
        std::string current_hash = module_metadata_source_hash(current);
        if (current_hash != source.content_hash) {
            fail_stale(display_path, "source '" + display_module_name(source.module_name) +
                       "' at '" + source.path + "' content hash changed from '" +
                       source.content_hash + "' to '" + current_hash + "'");
        }
    }
    if (!matched_root) {
        fail_stale(display_path, "root input changed to '" + root_input + "'");
    }

    for (const auto& source : cache.sources) {
        if (!metadata_source_keys.count(source_key(source.module_name, source.path, source.is_root))) {
            fail_stale(display_path, "cached source '" + display_module_name(source.module_name) +
                       "' at '" + source.path + "' is not listed by embedded metadata");
        }
    }

    for (const auto& summary : cache.ast_summaries) {
        if (!metadata_source_keys.count(source_key(summary.module_name, summary.path, summary.is_root))) {
            fail_stale(display_path, "cached AST summary for " +
                       source_display(summary.module_name, summary.path) +
                       " is not listed by embedded metadata");
        }
    }

    for (const auto& import : cache.metadata.imports) {
        auto owner_it = source_path_by_module.find(import.owner_module);
        if (owner_it == source_path_by_module.end()) {
            fail_stale(display_path, "cached import owner '" +
                       display_module_name(import.owner_module) + "' has no source record");
        }
        std::string resolved = find_module_file_for_cache(
            import,
            dirname(owner_it->second),
            options.module_search_paths
        );
        if (resolved.empty()) {
            fail_stale(display_path, "module import '" + import.module_name +
                       "' can no longer be resolved");
        }
        if (resolved != import.source_path) {
            fail_stale(display_path, "module import '" + import.module_name +
                       "' resolved to '" + resolved + "' instead of cached '" +
                       import.source_path + "'");
        }
    }
}

} // namespace ari
