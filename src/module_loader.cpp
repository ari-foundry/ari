#include "module_loader.hpp"

#include "common.hpp"
#include "lexer.hpp"
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
                                      const std::set<std::string>& cfg_features) {
    std::string source = read_file(path);
    std::string content_hash = module_metadata_source_hash(source);
    std::vector<Token> tokens = lex_source(std::move(source));
    return ParsedModuleFile{
        parse_tokens_in_module(std::move(tokens), module_path, cfg_features),
        std::move(content_hash),
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
        ParsedModuleFile root = parse_file_in_module(input, {}, options_.cfg_features);
        Program program = std::move(root.program);
        collect_module_metadata_source(metadata_, input, std::move(root.content_hash), {}, program, true);
        if (options_.implicit_std) load_standard_module(program);
        resolve_imports(program, dirname(input));
        return ModuleLoadResult{std::move(program), std::move(metadata_)};
    }

private:
    ModuleLoadOptions options_;
    ModuleMetadata metadata_;
    std::map<std::string, std::string> loaded_modules_;
    std::set<std::string> loading_modules_;

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
        ParsedModuleFile standard_file = parse_file_in_module(*path, module_path, options_.cfg_features);
        Program standard = std::move(standard_file.program);
        collect_module_metadata_source(metadata_, *path, std::move(standard_file.content_hash), module_path, standard, false);
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

            ModuleFileSearch found = find_module_file(import, base_dir, options_.module_search_paths);
            add_module_metadata_import(metadata_, import, found.path);
            auto loaded = loaded_modules_.find(import.name);
            if (loaded != loaded_modules_.end()) {
                if (loaded->second != found.path) {
                    throw CompileError(where(import.loc) + ": module '" + import.name +
                                       "' was already loaded from '" + loaded->second +
                                       "', not '" + found.path + "'");
                }
                continue;
            }

            loading_modules_.insert(import.name);
            std::vector<std::string> module_path = split_qualified_path(import.name);
            ParsedModuleFile child_file = parse_file_in_module(found.path, module_path, options_.cfg_features);
            Program child = std::move(child_file.program);
            collect_module_metadata_source(metadata_, found.path, std::move(child_file.content_hash), module_path, child, false);
            resolve_imports(child, dirname(found.path));
            loading_modules_.erase(import.name);
            loaded_modules_.emplace(import.name, found.path);
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
