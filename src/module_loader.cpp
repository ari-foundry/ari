#include "module_loader.hpp"

#include "common.hpp"
#include "lexer.hpp"
#include "module_path.hpp"
#include "parser.hpp"

#include <fstream>
#include <iterator>
#include <map>
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

Program parse_file_in_module(const std::string& path,
                             const std::vector<std::string>& module_path,
                             const std::set<std::string>& cfg_features) {
    std::string source = read_file(path);
    std::vector<Token> tokens = lex_source(std::move(source));
    return parse_tokens_in_module(std::move(tokens), module_path, cfg_features);
}

class ModuleLoader {
public:
    ModuleLoader(std::vector<std::string> module_search_paths, std::set<std::string> cfg_features)
        : module_search_paths_(std::move(module_search_paths)),
          cfg_features_(std::move(cfg_features)) {}

    Program load(const std::string& input) {
        Program program = parse_file_in_module(input, {}, cfg_features_);
        resolve_imports(program, dirname(input));
        return program;
    }

private:
    std::vector<std::string> module_search_paths_;
    std::set<std::string> cfg_features_;
    std::map<std::string, std::string> loaded_modules_;
    std::set<std::string> loading_modules_;

    void resolve_imports(Program& program, const std::string& base_dir) {
        std::vector<ModuleImport> imports = std::move(program.module_imports);
        program.module_imports.clear();

        for (const auto& import : imports) {
            if (loading_modules_.count(import.name)) {
                throw CompileError(where(import.loc) + ": cyclic module import '" + import.name + "'");
            }

            ModuleFileSearch found = find_module_file(import, base_dir, module_search_paths_);
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
            Program child = parse_file_in_module(found.path, split_qualified_path(import.name), cfg_features_);
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
    ModuleLoader loader(module_search_paths, {});
    return loader.load(input);
}

Program parse_file_with_modules(const std::string& input,
                                const std::vector<std::string>& module_search_paths,
                                std::set<std::string> cfg_features) {
    ModuleLoader loader(module_search_paths, std::move(cfg_features));
    return loader.load(input);
}

} // namespace ari
