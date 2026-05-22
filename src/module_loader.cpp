#include "module_loader.hpp"

#include "common.hpp"
#include "lexer.hpp"
#include "module_ast_summary.hpp"
#include "module_cache.hpp"
#include "module_ir_summary.hpp"
#include "module_metadata.hpp"
#include "module_path.hpp"
#include "parser.hpp"

#include <cctype>
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
    std::vector<ModuleCacheIrFunctionSummary> cached_ir_functions;
};

struct ImplSpecializationOrigin {
    std::string receiver_key;
    std::string trait_key;
    std::string method_name;
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

bool starts_with(const std::string& text, const std::string& prefix) {
    return text.size() >= prefix.size() &&
           text.compare(0, prefix.size(), prefix) == 0;
}

std::string basename_of_name(const std::string& name) {
    std::size_t split = name.rfind("::");
    if (split == std::string::npos) return name;
    return name.substr(split + 2);
}

std::string mangle_text_key(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (unsigned char c : text) {
        if (std::isalnum(c)) out.push_back(static_cast<char>(c));
        else out.push_back('_');
    }
    return out;
}

std::string module_qualified_name(const std::string& module_name,
                                  const std::string& name) {
    if (name.find("::") != std::string::npos) return name;
    if (module_name.empty()) return name;
    return module_name + "::" + name;
}

bool split_impl_specialization_origin(const std::string& origin,
                                      ImplSpecializationOrigin& parsed) {
    if (!starts_with(origin, "impl::")) return false;
    std::size_t receiver_start = 6;
    std::size_t trait_split = origin.find("::", receiver_start);
    if (trait_split == std::string::npos || trait_split == receiver_start) return false;
    std::size_t method_split = origin.find("::", trait_split + 2);
    if (method_split == std::string::npos || method_split == trait_split + 2) return false;
    if (origin.find("::", method_split + 2) != std::string::npos) return false;

    parsed.receiver_key = origin.substr(receiver_start, trait_split - receiver_start);
    parsed.trait_key = origin.substr(trait_split + 2, method_split - trait_split - 2);
    parsed.method_name = origin.substr(method_split + 2);
    return !parsed.receiver_key.empty() &&
           !parsed.trait_key.empty() &&
           !parsed.method_name.empty();
}

std::set<std::string> generic_param_name_set(const std::vector<GenericParam>& generics) {
    std::set<std::string> names;
    for (const auto& generic : generics) names.insert(generic.name);
    return names;
}

void add_generic_param_names(std::set<std::string>& names,
                             const std::vector<GenericParam>& generics) {
    for (const auto& generic : generics) names.insert(generic.name);
}

std::string impl_origin_method_key(const std::string& trait_key,
                                   const std::string& method_name) {
    return trait_key + "::" + method_name;
}

std::string inherent_impl_origin_method_key(const std::string& method_name) {
    return impl_origin_method_key("inherent", method_name);
}

void require_known_specialization_args(const ModuleCacheIrFunctionSummary& fn,
                                       const std::set<std::string>& allowed_names,
                                       const std::string& path) {
    std::set<std::string> seen;
    for (const auto& arg : fn.specialization_args) {
        if (!seen.insert(arg.name).second) {
            throw CompileError("module cache IR summary for '" + path +
                               "' has specialization '" + fn.name +
                               "' with duplicate generic argument '" +
                               arg.name + "'");
        }
        if (!allowed_names.count(arg.name)) {
            throw CompileError("module cache IR summary for '" + path +
                               "' has specialization '" + fn.name +
                               "' with unknown generic argument '" +
                               arg.name + "'");
        }
    }
}

ModuleFileSearch find_module_file(const ModuleImport& import,
                                  const std::string& base_dir,
                                  const std::vector<std::string>& module_search_paths) {
    ModuleFileSearch result;
    add_module_candidates(base_dir, import.local_name, result.searched);
    if (!import.module_name.empty()) {
        add_module_candidates(
            path_join(base_dir, qualified_basename(import.module_name)),
            import.local_name,
            result.searched);
    }
    for (const auto& search_path : module_search_paths) {
        if (!search_path.empty()) add_module_candidates(search_path, import.local_name, result.searched);
    }

    for (const auto& candidate : result.searched) {
        if (file_exists(candidate)) {
            result.path = candidate;
            return result;
        }
    }

    throw CompileError(import.loc,
                       "cannot find module file for '" + import.name +
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
    move_append(dst.item_macros, src.item_macros);
    move_append(dst.constants, src.constants);
    move_append(dst.type_aliases, src.type_aliases);
    move_append(dst.functions, src.functions);
    move_append(dst.structs, src.structs);
    move_append(dst.enums, src.enums);
    move_append(dst.traits, src.traits);
    move_append(dst.impls, src.impls);
}

bool is_cache_backed_executable_function(const FunctionDecl& fn) {
    return !fn.meta && !fn.is_extern && fn.has_body && fn.generics.empty();
}

void require_ir_summary_covers_ast_summary_functions(const Program& declarations,
                                                     const std::vector<ModuleCacheIrFunctionSummary>& ir_functions,
                                                     const std::string& path) {
    std::set<std::string> lowered_function_names;
    for (const auto& fn : ir_functions) lowered_function_names.insert(fn.name);

    for (const auto& fn : declarations.functions) {
        if (!is_cache_backed_executable_function(fn)) continue;
        if (lowered_function_names.count(fn.name)) continue;
        throw CompileError("module cache IR summary for '" + path +
                           "' is missing lowered function '" + fn.name + "'");
    }
}

void require_ir_summary_specializations_match_ast_summary(
    const Program& declarations,
    const std::vector<ModuleCacheIrFunctionSummary>& ir_functions,
    const std::string& path) {
    std::map<std::string, std::set<std::string>> generic_function_params;
    for (const auto& fn : declarations.functions) {
        if (!fn.generics.empty()) {
            generic_function_params.emplace(fn.name, generic_param_name_set(fn.generics));
        }
    }
    std::map<std::string, std::set<std::string>> impl_specialization_method_params;
    std::map<std::string, std::set<std::string>> impl_specialization_origin_params;
    std::set<std::string> locally_declared_trait_method_keys;
    std::set<std::string> locally_declared_trait_method_names;
    for (const auto& impl : declarations.impls) {
        for (const auto& method : impl.methods) {
            std::set<std::string>& names =
                impl_specialization_method_params[basename_of_name(method.name)];
            add_generic_param_names(names, impl.generics);
            add_generic_param_names(names, method.generics);

            if (impl.has_trait) {
                std::set<std::string> trait_keys;
                trait_keys.insert(mangle_text_key(impl.trait_type.name));
                trait_keys.insert(mangle_text_key(module_qualified_name(impl.module_name, impl.trait_type.name)));
                for (const auto& trait_key : trait_keys) {
                    std::set<std::string>& origin_names =
                        impl_specialization_origin_params[impl_origin_method_key(
                            trait_key,
                            basename_of_name(method.name))];
                    add_generic_param_names(origin_names, impl.generics);
                    add_generic_param_names(origin_names, method.generics);
                }
            } else {
                std::set<std::string>& origin_names =
                    impl_specialization_origin_params[inherent_impl_origin_method_key(
                        basename_of_name(method.name))];
                add_generic_param_names(origin_names, impl.generics);
                add_generic_param_names(origin_names, method.generics);
            }
        }
    }
    for (const auto& trait : declarations.traits) {
        for (const auto& method : trait.methods) {
            std::string method_name = basename_of_name(method.name);
            std::set<std::string>& names =
                impl_specialization_method_params[method_name];
            add_generic_param_names(names, trait.generics);
            add_generic_param_names(names, method.generics);

            std::string key = impl_origin_method_key(mangle_text_key(trait.name), method_name);
            locally_declared_trait_method_keys.insert(key);
            locally_declared_trait_method_names.insert(method_name);
            std::set<std::string>& origin_names = impl_specialization_origin_params[key];
            add_generic_param_names(origin_names, trait.generics);
            add_generic_param_names(origin_names, method.generics);
        }
    }

    for (const auto& fn : ir_functions) {
        if (fn.specialization_kind.empty()) continue;
        if (fn.specialization_kind == "generic-function") {
            auto origin = generic_function_params.find(fn.specialization_origin);
            if (origin == generic_function_params.end()) {
                throw CompileError("module cache IR summary for '" + path +
                                   "' has generic specialization '" + fn.name +
                                   "' for unknown generic function '" +
                                   fn.specialization_origin + "'");
            }
            require_known_specialization_args(fn, origin->second, path);
            continue;
        }
        if (fn.specialization_kind == "impl-method") {
            ImplSpecializationOrigin origin;
            bool has_method_origin = split_impl_specialization_origin(fn.specialization_origin, origin);
            auto method = has_method_origin
                ? impl_specialization_method_params.find(origin.method_name)
                : impl_specialization_method_params.end();
            if (!has_method_origin || method == impl_specialization_method_params.end()) {
                throw CompileError("module cache IR summary for '" + path +
                                   "' has impl-method specialization '" + fn.name +
                                   "' for unknown impl method origin '" +
                                   fn.specialization_origin + "'");
            }
            auto origin_method = impl_specialization_origin_params.find(
                impl_origin_method_key(origin.trait_key, origin.method_name));
            if (origin_method == impl_specialization_origin_params.end() &&
                locally_declared_trait_method_names.count(origin.method_name) != 0 &&
                locally_declared_trait_method_keys.count(impl_origin_method_key(origin.trait_key, origin.method_name)) == 0) {
                throw CompileError("module cache IR summary for '" + path +
                                   "' has impl-method specialization '" + fn.name +
                                   "' for unknown impl method origin '" +
                                   fn.specialization_origin + "'");
            }
            require_known_specialization_args(
                fn,
                origin_method == impl_specialization_origin_params.end()
                    ? method->second
                    : origin_method->second,
                path);
            continue;
        }
        throw CompileError("module cache IR summary for '" + path +
                           "' has unknown specialization kind '" +
                           fn.specialization_kind + "' for lowered function '" +
                           fn.name + "'");
    }
}

void require_cached_impl_call_targets_resolve_expr(
    const ModuleCacheIrExprSummaryPtr& expr,
    const std::set<std::string>& lowered_function_names,
    const std::string& function_name,
    const std::string& path);

void require_cached_impl_call_targets_resolve_stmts(
    const std::vector<ModuleCacheIrStmtSummaryPtr>& statements,
    const std::set<std::string>& lowered_function_names,
    const std::string& function_name,
    const std::string& path);

void require_cached_impl_call_targets_resolve_exprs(
    const std::vector<ModuleCacheIrExprSummaryPtr>& expressions,
    const std::set<std::string>& lowered_function_names,
    const std::string& function_name,
    const std::string& path) {
    for (const auto& expr : expressions) {
        require_cached_impl_call_targets_resolve_expr(expr, lowered_function_names, function_name, path);
    }
}

void require_cached_impl_call_targets_resolve_expr(
    const ModuleCacheIrExprSummaryPtr& expr,
    const std::set<std::string>& lowered_function_names,
    const std::string& function_name,
    const std::string& path) {
    if (!expr) return;
    if ((expr->kind == "call" || expr->kind == "function-ref") &&
        starts_with(expr->name, "impl::") &&
        !lowered_function_names.count(expr->name)) {
        throw CompileError("module cache IR summary for '" + path +
                           "' has lowered body '" + function_name +
                           "' calling unknown cached impl function '" +
                           expr->name + "'");
    }

    require_cached_impl_call_targets_resolve_expr(expr->operand, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_expr(expr->left, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_expr(expr->right, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_expr(expr->payload, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_exprs(expr->args, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_expr(expr->if_condition, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_stmts(expr->if_then_body, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_expr(expr->if_then_value, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_stmts(expr->if_else_body, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_expr(expr->if_else_value, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_stmts(expr->block_body, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_expr(expr->block_value, lowered_function_names, function_name, path);
    require_cached_impl_call_targets_resolve_expr(expr->match_value, lowered_function_names, function_name, path);
    for (const auto& arm : expr->match_arms) {
        require_cached_impl_call_targets_resolve_stmts(arm.body, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_expr(arm.value, lowered_function_names, function_name, path);
    }
    require_cached_impl_call_targets_resolve_stmts(expr->try_residual_cleanup, lowered_function_names, function_name, path);
}

void require_cached_impl_call_targets_resolve_stmts(
    const std::vector<ModuleCacheIrStmtSummaryPtr>& statements,
    const std::set<std::string>& lowered_function_names,
    const std::string& function_name,
    const std::string& path) {
    for (const auto& statement : statements) {
        if (!statement) continue;
        require_cached_impl_call_targets_resolve_expr(statement->binding.init, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_expr(statement->assign_target, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_expr(statement->assign_rhs, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_expr(statement->expr, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_expr(statement->condition, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_stmts(statement->statements, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_stmts(statement->then_body, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_stmts(statement->else_body, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_stmts(statement->loop_body, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_expr(statement->for_start, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_expr(statement->for_end, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_exprs(statement->for_values, lowered_function_names, function_name, path);
        require_cached_impl_call_targets_resolve_expr(statement->match_value, lowered_function_names, function_name, path);
        for (const auto& binding : statement->init_bindings) {
            require_cached_impl_call_targets_resolve_expr(binding.init, lowered_function_names, function_name, path);
        }
        require_cached_impl_call_targets_resolve_exprs(statement->updates, lowered_function_names, function_name, path);
        for (const auto& arm : statement->match_arms) {
            require_cached_impl_call_targets_resolve_stmts(arm.body, lowered_function_names, function_name, path);
        }
        require_cached_impl_call_targets_resolve_expr(statement->break_value, lowered_function_names, function_name, path);
    }
}

void require_ir_summary_cached_impl_calls_resolve(
    const std::vector<ModuleCacheIrFunctionSummary>& ir_functions,
    const std::set<std::string>& lowered_function_names,
    const std::string& path) {
    for (const auto& fn : ir_functions) {
        require_cached_impl_call_targets_resolve_stmts(
            fn.body.statements,
            lowered_function_names,
            fn.name,
            path);
    }
}

std::set<std::string> cached_lowered_function_names(
    const ModuleCache& cache,
    const std::vector<ModuleCacheIrFunctionSummary>& local_ir_functions,
    const std::string& path) {
    std::set<std::string> lowered_function_names;
    for (const auto& fn : local_ir_functions) lowered_function_names.insert(fn.name);
    for (const auto& summary : cache.ir_summaries) {
        for (const auto& fn : materialize_module_cache_ir_summary_functions(summary, path)) {
            lowered_function_names.insert(fn.name);
        }
    }
    return lowered_function_names;
}

void require_ir_summary_cached_impl_calls_resolve(
    const std::vector<ModuleCacheIrFunctionSummary>& ir_functions,
    const ModuleCache* input_cache,
    const std::string& path) {
    if (input_cache) {
        require_ir_summary_cached_impl_calls_resolve(
            ir_functions,
            cached_lowered_function_names(*input_cache, ir_functions, path),
            path);
        return;
    }
    std::set<std::string> lowered_function_names;
    for (const auto& fn : ir_functions) lowered_function_names.insert(fn.name);
    require_ir_summary_cached_impl_calls_resolve(ir_functions, lowered_function_names, path);
}

bool can_load_module_cache_declarations_with_ir_functions(
    const Program& declarations,
    const std::vector<ModuleCacheIrFunctionSummary>& ir_functions) {
    if (!declarations.item_macros.empty()) return false;
    for (const auto& decl : declarations.constants) {
        if (!decl.init) return false;
    }

    std::set<std::string> lowered_function_names;
    for (const auto& fn : ir_functions) lowered_function_names.insert(fn.name);

    for (const auto& fn : declarations.functions) {
        if (!fn.has_body || fn.is_extern || fn.has_body_summary) continue;
        if (!is_cache_backed_executable_function(fn)) return false;
        if (!lowered_function_names.count(fn.name)) return false;
    }
    for (const auto& trait : declarations.traits) {
        for (const auto& method : trait.methods) {
            if (method.has_body && !method.is_extern && !method.has_body_summary) return false;
        }
    }
    for (const auto& impl : declarations.impls) {
        for (const auto& method : impl.methods) {
            if (method.has_body && !method.is_extern && !method.has_body_summary) return false;
        }
    }
    return true;
}

ParsedModuleFile parse_file_in_module(const std::string& path,
                                      const std::vector<std::string>& module_path,
                                      const std::set<std::string>& cfg_features,
                                      const std::string& target_triple,
                                      const ModuleCache* input_cache,
                                      bool allow_summary_materialize) {
    std::string source;
    if (input_cache) {
        const ModuleCacheSource* cached = find_module_cache_source(*input_cache, path);
        if (!cached) throw CompileError("module cache is missing source '" + path + "'");
        const bool ast_summaries_preserve_source_spans = false;
        if (allow_summary_materialize && ast_summaries_preserve_source_spans) {
            const ModuleCacheAstSummary* summary = find_module_cache_ast_summary(*input_cache, path);
            if (summary) {
                Program declarations = materialize_module_cache_ast_summary_declarations(*summary, path);
                std::vector<ModuleCacheIrFunctionSummary> ir_functions;
                const ModuleCacheIrSummary* ir_summary =
                    find_module_cache_ir_summary(*input_cache, path);
                bool can_load_ast_summary = can_load_module_cache_ast_summary_declarations(declarations);
                if (ir_summary) {
                    ir_functions = materialize_module_cache_ir_summary_functions(*ir_summary, path);
                    if (!can_load_ast_summary) {
                        require_ir_summary_covers_ast_summary_functions(declarations, ir_functions, path);
                    }
                    require_ir_summary_specializations_match_ast_summary(declarations, ir_functions, path);
                    require_ir_summary_cached_impl_calls_resolve(ir_functions, input_cache, path);
                }
                if (can_load_ast_summary ||
                    can_load_module_cache_declarations_with_ir_functions(declarations, ir_functions)) {
                    return ParsedModuleFile{
                        std::move(declarations),
                        summary->content_hash,
                        cached->source,
                        std::move(ir_functions),
                    };
                }
            }
        }
        source = cached->source;
    } else {
        source = read_file(path);
    }
    std::string content_hash = module_metadata_source_hash(source);
    std::vector<Token> tokens = lex_source(source, path);
    return ParsedModuleFile{
        parse_tokens_in_module(std::move(tokens), module_path, cfg_features, target_triple),
        std::move(content_hash),
        std::move(source),
        {},
    };
}

class ModuleLoader {
public:
    ModuleLoader(ModuleLoadOptions options)
        : options_(std::move(options)) {}

    ModuleLoadResult load(const std::string& input) {
        metadata_.module_search_paths = options_.module_search_paths;
        metadata_.cfg_features = options_.cfg_features;
        metadata_.target_triple = options_.target_triple;
        metadata_.implicit_std = options_.implicit_std;
        ParsedModuleFile root = parse_file_in_module(input, {}, options_.cfg_features, options_.target_triple, options_.input_cache, false);
        Program program = std::move(root.program);
        collect_source(input, root, {}, program, true);
        if (options_.implicit_std) load_standard_module(program);
        resolve_imports(program, dirname(input));
        ModuleCache cache;
        cache.metadata = metadata_;
        cache.sources = std::move(cache_sources_);
        cache.ast_summaries = std::move(cache_ast_summaries_);
        return ModuleLoadResult{
            std::move(program),
            std::move(metadata_),
            std::move(cache),
            std::move(cached_ir_functions_),
        };
    }

private:
    ModuleLoadOptions options_;
    ModuleMetadata metadata_;
    std::vector<ModuleCacheSource> cache_sources_;
    std::vector<ModuleCacheAstSummary> cache_ast_summaries_;
    std::vector<ModuleCacheIrFunctionSummary> cached_ir_functions_;
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
        decl.loc = SourceLocation{};
        program.modules.push_back(std::move(decl));

        std::vector<std::string> module_path{name};
        ParsedModuleFile standard_file = parse_file_in_module(*path, module_path, options_.cfg_features, options_.target_triple, options_.input_cache, true);
        Program standard = std::move(standard_file.program);
        collect_source(*path, standard_file, module_path, standard, false);
        move_append(cached_ir_functions_, standard_file.cached_ir_functions);
        loaded_modules_.emplace(name, *path);
        resolve_imports(standard, dirname(*path));
        append_program(program, std::move(standard));
    }

    void resolve_imports(Program& program, const std::string& base_dir) {
        std::vector<ModuleImport> imports = std::move(program.module_imports);
        program.module_imports.clear();

        for (const auto& import : imports) {
            if (loading_modules_.count(import.name)) {
                throw CompileError(import.loc, "cyclic module import '" + import.name + "'");
            }

            std::string source_path;
            if (options_.input_cache) {
                const ModuleMetadataImport* cached = find_module_cache_import(*options_.input_cache, import);
                if (!cached) {
                    throw CompileError(import.loc,
                                       "module cache is missing validated import '" +
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
                    throw CompileError(import.loc,
                                       "module '" + import.name +
                                           "' was already loaded from '" + loaded->second +
                                           "', not '" + source_path + "'");
                }
                continue;
            }

            loading_modules_.insert(import.name);
            std::vector<std::string> module_path = split_qualified_path(import.name);
            ParsedModuleFile child_file = parse_file_in_module(source_path, module_path, options_.cfg_features, options_.target_triple, options_.input_cache, true);
            Program child = std::move(child_file.program);
            collect_source(source_path, child_file, module_path, child, false);
            move_append(cached_ir_functions_, child_file.cached_ir_functions);
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
