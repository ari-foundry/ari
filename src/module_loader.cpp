#include "module_loader.hpp"

#include "common.hpp"
#include "lexer.hpp"
#include "module_ast_summary.hpp"
#include "module_cache.hpp"
#include "module_metadata.hpp"
#include "module_path.hpp"
#include "parser.hpp"

#include <fstream>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

struct ModuleFileSearch {
    std::string path;
    std::vector<std::string> searched;
};

struct ParsedModuleFile {
    Program program;
    std::string content_hash;
    std::string source;
};

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

std::string searched_paths_text(const std::vector<std::string>& paths) {
    std::string text;
    for (std::size_t i = 0; i < paths.size(); ++i) {
        if (i > 0) text += ", ";
        text += paths[i];
    }
    return text;
}

ModuleFileSearch find_module_file(const ModuleImport& import,
                                  const std::string& base_dir,
                                  const std::vector<std::string>& module_search_paths) {
    ModuleFileSearch result;
    add_module_candidates(base_dir, import.local_name, result.searched);
    for (const auto& search_path : module_search_paths) {
        if (!search_path.empty()) add_module_candidates(search_path, import.local_name, result.searched);
    }

    for (const auto& candidate : result.searched) {
        if (file_exists(candidate)) {
            result.path = candidate;
            return result;
        }
    }

    throw CompileError(where(import.loc) + ": cannot find module file for '" + import.name +
                       "'; searched " + searched_paths_text(result.searched));
}

std::optional<std::string> find_standard_header_file() {
    const std::string path = "lib/std.arih";
    if (file_exists(path)) return path;
    return std::nullopt;
}

bool has_root_module_decl(const Program& program, const std::string& name) {
    for (const auto& module : program.modules) {
        if (module.module_name.empty() && module.name == name) return true;
    }
    return false;
}

bool has_root_module_import(const Program& program, const std::string& name) {
    for (const auto& import : program.module_imports) {
        if (import.module_name.empty() && import.name == name) return true;
    }
    return false;
}

template <typename T>
void move_append(std::vector<T>& dst, std::vector<T>& src) {
    dst.insert(dst.end(), std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()));
    src.clear();
}

void append_program(Program& dst, Program&& src) {
    move_append(dst.uses, src.uses);
    move_append(dst.module_imports, src.module_imports);
    move_append(dst.modules, src.modules);
    move_append(dst.functions, src.functions);
    move_append(dst.structs, src.structs);
    move_append(dst.enums, src.enums);
    move_append(dst.traits, src.traits);
    move_append(dst.impls, src.impls);
}

ParsedModuleFile parse_file_in_module(const std::string& path,
                                      const std::vector<std::string>& module_path,
                                      const std::set<std::string>& cfg_features,
                                      const ModuleCache* input_cache) {
    std::string source;
    if (input_cache) {
        const ModuleCacheSource* cached = find_module_cache_source(*input_cache, path);
        if (!cached) throw CompileError("module cache is missing source '" + path + "'");
        source = cached->source;
    } else {
        source = read_file(path);
    }
    std::string content_hash = module_metadata_source_hash(source);
    std::vector<Token> tokens = lex_source(source);
    return ParsedModuleFile{
        parse_tokens_in_module(std::move(tokens), module_path, cfg_features),
        std::move(content_hash),
        std::move(source),
    };
}

class ModuleLoader {
public:
    ModuleLoader(ModuleLoadOptions options)
        : options_(std::move(options)) {}

    ModuleLoadResult load(const std::string& input) {
        metadata_.module_search_paths = options_.module_search_paths;
        metadata_.cfg_features = options_.cfg_features;
        metadata_.implicit_std = options_.implicit_std;
        ParsedModuleFile root = parse_file_in_module(input, {}, options_.cfg_features, options_.input_cache);
        Program program = std::move(root.program);
        collect_source(input, root, {}, program, true);
        if (options_.implicit_std) load_standard_module(program);
        resolve_imports(program, dirname(input));
        ModuleCache cache;
        cache.metadata = metadata_;
        cache.sources = std::move(cache_sources_);
        cache.ast_summaries = std::move(cache_ast_summaries_);
        return ModuleLoadResult{std::move(program), std::move(metadata_), std::move(cache)};
    }

private:
    ModuleLoadOptions options_;
    ModuleMetadata metadata_;
    std::vector<ModuleCacheSource> cache_sources_;
    std::vector<ModuleCacheAstSummary> cache_ast_summaries_;
    std::map<std::string, std::string> loaded_modules_;
    std::set<std::string> loading_modules_;

    void collect_source(const std::string& path,
                        const ParsedModuleFile& parsed,
                        const std::vector<std::string>& module_path,
                        const Program& program,
                        bool is_root) {
        collect_module_metadata_source(metadata_, path, parsed.content_hash, module_path, program, is_root);
        cache_sources_.push_back(ModuleCacheSource{
            join_qualified_path(module_path),
            path,
            parsed.content_hash,
            parsed.source,
            is_root,
        });
        cache_ast_summaries_.push_back(make_module_cache_ast_summary(
            path,
            parsed.content_hash,
            module_path,
            program,
            is_root
        ));
        if (options_.input_cache) {
            const ModuleCacheAstSummary* cached = find_module_cache_ast_summary(*options_.input_cache, path);
            if (!cached) throw CompileError("module cache is missing AST summary for source '" + path + "'");
            require_matching_module_cache_ast_summary(*cached, cache_ast_summaries_.back());
        }
    }

    void load_standard_module(Program& program) {
        const std::string name = "std";
        if (has_root_module_decl(program, name) || has_root_module_import(program, name)) return;
        std::optional<std::string> path = find_standard_header_file();
        if (!path) return;

        ModuleDecl decl;
        decl.name = name;
        decl.module_name = "";
        decl.is_public = true;
        decl.loc = SourceLocation{1, 1};
        program.modules.push_back(std::move(decl));

        std::vector<std::string> module_path{name};
        ParsedModuleFile standard_file = parse_file_in_module(*path, module_path, options_.cfg_features, options_.input_cache);
        Program standard = std::move(standard_file.program);
        collect_source(*path, standard_file, module_path, standard, false);
        loaded_modules_.emplace(name, *path);
        append_program(program, std::move(standard));
    }

    void resolve_imports(Program& program, const std::string& base_dir) {
        std::vector<ModuleImport> imports = std::move(program.module_imports);
        program.module_imports.clear();

        for (const auto& import : imports) {
            if (loading_modules_.count(import.name)) {
                throw CompileError(where(import.loc) + ": cyclic module import '" + import.name + "'");
            }

            std::string source_path;
            if (options_.input_cache) {
                const ModuleMetadataImport* cached = find_module_cache_import(*options_.input_cache, import);
                if (!cached) {
                    throw CompileError(where(import.loc) + ": module cache is missing validated import '" +
                                       import.name + "' in module '" +
                                       (import.module_name.empty() ? "<root>" : import.module_name) + "'");
                }
                source_path = cached->source_path;
            } else {
                source_path = find_module_file(import, base_dir, options_.module_search_paths).path;
            }
            add_module_metadata_import(metadata_, import, source_path);
            auto loaded = loaded_modules_.find(import.name);
            if (loaded != loaded_modules_.end()) {
                if (loaded->second != source_path) {
                    throw CompileError(where(import.loc) + ": module '" + import.name +
                                       "' was already loaded from '" + loaded->second +
                                       "', not '" + source_path + "'");
                }
                continue;
            }

            loading_modules_.insert(import.name);
            std::vector<std::string> module_path = split_qualified_path(import.name);
            ParsedModuleFile child_file = parse_file_in_module(source_path, module_path, options_.cfg_features, options_.input_cache);
            Program child = std::move(child_file.program);
            collect_source(source_path, child_file, module_path, child, false);
            resolve_imports(child, dirname(source_path));
            loading_modules_.erase(import.name);
            loaded_modules_.emplace(import.name, source_path);
            append_program(program, std::move(child));
        }
    }
};

} // namespace

Program parse_file_with_modules(const std::string& input,
                                const std::vector<std::string>& module_search_paths) {
    ModuleLoadOptions options;
    options.module_search_paths = module_search_paths;
    ModuleLoader loader(std::move(options));
    return loader.load(input).program;
}

Program parse_file_with_modules(const std::string& input,
                                const std::vector<std::string>& module_search_paths,
                                std::set<std::string> cfg_features) {
    ModuleLoadOptions options;
    options.module_search_paths = module_search_paths;
    options.cfg_features = std::move(cfg_features);
    ModuleLoader loader(std::move(options));
    return loader.load(input).program;
}

ModuleLoadResult parse_file_with_module_metadata(const std::string& input,
                                                 const std::vector<std::string>& module_search_paths,
                                                 std::set<std::string> cfg_features) {
    ModuleLoadOptions options;
    options.module_search_paths = module_search_paths;
    options.cfg_features = std::move(cfg_features);
    ModuleLoader loader(std::move(options));
    return loader.load(input);
}

ModuleLoadResult parse_file_with_module_metadata(const std::string& input,
                                                 ModuleLoadOptions options) {
    ModuleLoader loader(std::move(options));
    return loader.load(input);
}

} // namespace ari
