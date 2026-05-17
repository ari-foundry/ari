#include "sema.hpp"

#include "ast_builders.hpp"
#include "attribute_semantics.hpp"
#include "borrow_return_semantics.hpp"
#include "aggregate_literal_semantics.hpp"
#include "ast_clone.hpp"
#include "ari_builtin.hpp"
#include "borrow_call_semantics.hpp"
#include "borrow_lifetime.hpp"
#include "borrow_semantics.hpp"
#include "c_abi_types.hpp"
#include "cfg_eval.hpp"
#include "constant_semantics.hpp"
#include "control_flow_semantics.hpp"
#include "drop_semantics.hpp"
#include "enum_constructor_semantics.hpp"
#include "for_pattern_semantics.hpp"
#include "format_semantics.hpp"
#include "format_string_semantics.hpp"
#include "ir_builders.hpp"
#include "iterator_semantics.hpp"
#include "layout.hpp"
#include "local_state.hpp"
#include "loop_state_semantics.hpp"
#include "meta_expansion.hpp"
#include "meta_semantics.hpp"
#include "module_path.hpp"
#include "move_semantics.hpp"
#include "parser.hpp"
#include "pattern_coverage.hpp"
#include "pattern_semantics.hpp"
#include "prelude_macros.hpp"
#include "prelude_resolver.hpp"
#include "product_coverage.hpp"
#include "range_semantics.hpp"
#include "slice_semantics.hpp"
#include "std_box_semantics.hpp"
#include "std_enum_probe_semantics.hpp"
#include "std_string_semantics.hpp"
#include "std_vec_semantics.hpp"
#include "symbol_mangle.hpp"
#include "trait_semantics.hpp"
#include "try_model.hpp"
#include "type_inference.hpp"
#include "type_semantics.hpp"
#include "vector_semantics.hpp"
#include "zone_pointer_semantics.hpp"
#include "zone_return_semantics.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ari {

struct FunctionSig {
    std::vector<IrType> params;
    IrType result;
    std::optional<std::size_t> borrow_return_param_index;
    std::string borrow_return_path;
    bool borrow_return_contract_explicit = false;
    std::optional<std::size_t> zone_pointer_return_param_index;
    std::string module_name;
    std::string link_name;
    bool is_public = false;
    bool is_extern = false;
    bool is_variadic = false;
    std::string extern_abi;
    bool deprecated = false;
    std::string deprecated_message;
    SourceLocation loc;
};

static void set_borrow_return_contract(FunctionSig& sig) {
    sig.borrow_return_param_index = borrow_return_param_index(sig.params, sig.result);
}

static void set_zone_pointer_return_contract(FunctionSig& sig) {
    sig.zone_pointer_return_param_index = zone_pointer_return_param_index(sig.params, sig.result);
}

static void set_function_return_contracts(FunctionSig& sig) {
    set_borrow_return_contract(sig);
    set_zone_pointer_return_contract(sig);
}

static IrExternAbi ir_extern_abi_from_source(const std::string& abi) {
    if (abi == "ari") return IrExternAbi::AriBuiltin;
    return IrExternAbi::C;
}

struct EnumCaseInfo {
    std::string enum_name;
    IrType enum_type;
    std::string name;
    std::string module_name;
    std::uint32_t tag = 0;
    std::vector<IrType> payloads;
    std::vector<TypeRef> payload_refs;
    std::vector<std::string> generic_names;
    bool is_generic = false;
    SourceLocation loc;
};

struct EnumInfo {
    struct Case {
        std::string name;
        std::string qualified_name;
        std::uint32_t tag = 0;
        std::vector<TypeRef> payloads;
        SourceLocation loc;
    };

    std::string name;
    std::string module_name;
    IrType type;
    SourceLocation loc;
    std::vector<std::string> case_names;
    std::size_t generic_arity = 0;
    std::vector<std::string> generic_names;
    std::vector<Case> cases;
    bool is_public = false;
    bool deprecated = false;
    std::string deprecated_message;
};

struct StructInfo {
    struct Field {
        std::string name;
        TypeRef type;
        bool mutable_field = false;
        SourceLocation loc;
    };

    std::string name;
    std::string module_name;
    std::size_t generic_arity = 0;
    std::vector<std::string> generic_names;
    std::vector<Field> fields;
    bool tuple_struct = false;
    bool is_public = false;
    bool deprecated = false;
    std::string deprecated_message;
    SourceLocation loc;
};

struct TraitInfo {
    struct AssociatedType {
        std::string name;
        SourceLocation loc;
    };

    struct Method {
        std::string name;
        std::vector<GenericParam> generics;
        std::vector<TypeRef> params;
        TypeRef result;
        bool has_result = false;
        bool has_self_receiver = false;
        SourceLocation loc;
    };

    std::string name;
    std::string module_name;
    std::size_t generic_arity = 0;
    std::vector<std::string> generic_names;
    std::vector<TypeRef> supertrait_refs;
    std::map<std::string, AssociatedType> associated_types;
    std::map<std::string, Method> methods;
    bool is_public = false;
    SourceLocation loc;
};

struct UseInfo {
    std::string path;
    std::string module_name;
    bool is_public = false;
    SourceLocation loc;
};

struct ModuleInfo {
    std::string name;
    std::string module_name;
    bool is_public = false;
    SourceLocation loc;
};

struct ConstantInfo {
    std::string name;
    std::string module_name;
    bool is_public = false;
    TypeRef type_ref;
    const Expr* init = nullptr;
    SourceLocation loc;
    bool evaluating = false;
    bool evaluated = false;
    ConstantValue value;
};

struct TrackedAggregateAccess {
    std::string base_name;
    IrType base_type;
    IrType type;
    std::string path;
    IrExprPtr expr;
    bool has_final_field_mutability = false;
    bool final_field_mutable = true;
    std::string final_field_label;
    std::string final_container_name;
};

class SemanticChecker {
public:
    explicit SemanticChecker(const Program& program, SemaOptions options)
        : program_(program), options_(options), borrow_context_(local_scopes_) {}

    IrProgram check() {
        collect_module_decls();
        collect_uses();
        collect_meta_functions();
        expand_item_macro_invocations();
        expand_attribute_macros();
        expand_generated_attribute_macros();
        collect_item_macro_module_decls();
        collect_item_macro_uses();
        validate_attributes();
        validate_pattern_macro_invocations();
        collect_trait_decls();
        collect_struct_decls();
        collect_enum_layouts();
        expand_derive_impls();
        collect_constant_decls();
        validate_struct_decls();
        resolve_trait_supertraits();
        validate_trait_method_decls();
        validate_generic_constraints();
        validate_impls();
        validate_supertrait_impls();
        validate_into_iterator_result_contracts();
        collect_impl_method_signatures();
        collect_function_signatures();
        std::vector<const FunctionDecl*> test_functions = collect_test_functions();
        if (options_.require_main) require_main();
        if (options_.test_mode && test_functions.empty()) {
            throw CompileError("test mode requires at least one @test function");
        }

        IrProgram ir;
        ir.require_main = options_.require_main || options_.test_mode;
        ir.target_triple = target_.triple;
        collect_ir_extern_functions(ir);
        collect_ir_c_enums(ir);
        collect_ir_c_records(ir);
        for_each_function_decl([&](const FunctionDecl& fn) {
            if (is_executable_function(fn) &&
                !options_.cached_ir_function_names.count(fn.name) &&
                !(options_.test_mode && fn.name == "main")) {
                ir.functions.push_back(check_function(fn));
            }
        });
        if (options_.test_mode) {
            ir.functions.push_back(make_test_main(test_functions));
        }
        std::size_t impl_method_index = 0;
        std::size_t specialization_index = 0;
        while (impl_method_index < impl_methods_to_lower_.size() ||
               specialization_index < pending_specializations_.size()) {
            while (impl_method_index < impl_methods_to_lower_.size()) {
                const ImplMethodInfo& item = impl_methods_to_lower_[impl_method_index++];
                ir.functions.push_back(check_function_as(*item.fn, item.lowered_name, item.substitutions));
            }
            while (specialization_index < pending_specializations_.size()) {
                const PendingSpecialization& item = pending_specializations_[specialization_index++];
                ir.functions.push_back(check_function_as(*item.fn, item.name, item.substitutions));
            }
        }
        ir.trait_object_vtables = trait_object_vtables_;
        ir.warnings = warnings_;
        return ir;
    }

private:
    enum class Flow {
        Continues,
        Stops,
        Returns
    };

    struct CheckedStatements {
        std::vector<IrStmtPtr> statements;
        Flow flow = Flow::Continues;
    };

    struct CheckedStatement {
        IrStmtPtr statement;
        Flow flow = Flow::Continues;
    };

    struct CheckedExprBlock {
        std::vector<IrStmtPtr> statements;
        IrExprPtr value;
        StateSnapshot state;
        std::optional<BorrowResultSource> borrow_source;
        bool diverges = false;
    };

    struct LoopInfo {
        std::vector<std::string> names;
        std::vector<IrType> types;
        std::string label;
        bool is_loop = true;
        bool supports_values = false;
        bool supports_break_values = false;
        std::size_t scope_depth = 0;
        bool has_break_result_type = false;
        IrType break_result_type;
        std::vector<StateSnapshot> break_state_snapshots;
        std::vector<StateSnapshot> continue_state_snapshots;
        std::optional<BorrowResultSource> break_borrow_source;
        std::vector<std::string> exit_cleanup_owner_names;
    };

    struct LoopBodyRecheck {
        const std::vector<StmtPtr>* statements = nullptr;
        bool scoped = false;
        LocalScopeStack::NameState name_state;
    };

    struct PendingSpecialization {
        const FunctionDecl* fn = nullptr;
        std::string name;
        std::map<std::string, IrType> substitutions;
    };

    struct GenericTraitBound {
        std::string generic_name;
        std::string trait_name;
        std::vector<IrType> trait_args;
        SourceLocation loc;
    };

    struct ImplMethodInfo {
        const FunctionDecl* fn = nullptr;
        std::string lowered_name;
        std::string trait_name;
        std::vector<IrType> trait_args;
        IrType receiver_type;
        std::vector<std::string> generic_names;
        std::vector<std::string> method_generic_names;
        std::vector<GenericTraitBound> generic_bounds;
        std::vector<GenericTraitBound> method_generic_bounds;
        FunctionSig sig;
        std::map<std::string, IrType> substitutions;
        std::string module_name;
        bool is_public = false;
        SourceLocation loc;
    };

    struct GenericTraitImplInfo {
        std::string trait_name;
        std::vector<IrType> trait_args;
        IrType self_type;
        std::vector<std::string> generic_names;
        std::vector<GenericTraitBound> generic_bounds;
        std::string module_name;
        SourceLocation loc;
    };

    struct BorrowReturnPathHintScan {
        std::optional<std::string> path;
        bool unknown = false;
    };

    struct BorrowReturnOperandPathHint {
        std::string path;
        IrType type;
    };

    struct TraitImplCoherenceInfo {
        std::string trait_name;
        std::vector<IrType> trait_args;
        IrType self_type;
        std::vector<std::string> generic_names;
        SourceLocation loc;
    };

    struct TraitObjectMethodEntry {
        const TraitInfo* trait = nullptr;
        std::vector<IrType> trait_args;
        const TraitInfo::Method* method = nullptr;
    };

    struct TraitObjectConversion {
        std::string vtable_name;
        std::uint64_t vtable_offset = 0;
    };

    const Program& program_;
    SemaOptions options_;
    TargetInfo target_ = resolve_target_info(options_.target_triple);
    std::map<std::string, FunctionSig> functions_;
    std::map<std::string, const FunctionDecl*> generic_functions_;
    std::map<std::string, MetaFunctionInfo> meta_functions_;
    std::map<std::string, ConstantInfo> constants_;
    std::map<std::string, StructInfo> structs_;
    std::map<std::string, EnumInfo> enums_;
    std::map<std::string, EnumCaseInfo> enum_cases_;
    std::map<std::string, TraitInfo> traits_;
    std::map<std::string, std::vector<GenericTraitBound>> trait_supertraits_;
    std::map<std::string, ModuleInfo> modules_;
    std::map<std::string, std::map<std::string, UseInfo>> uses_;
    std::set<std::string> impl_keys_;
    LocalScopeStack local_scopes_;
    BorrowContext borrow_context_;
    std::vector<LoopInfo> loops_;
    std::map<std::string, IrType> current_type_substitutions_;
    std::vector<GenericTraitBound> current_generic_bounds_;
    std::vector<IrFunction> specialized_functions_;
    std::vector<PendingSpecialization> pending_specializations_;
    std::set<std::string> queued_specializations_;
    std::map<std::string, std::vector<ImplMethodInfo>> method_impls_;
    std::map<std::string, std::vector<ImplMethodInfo>> associated_impls_;
    std::vector<ImplMethodInfo> generic_method_impls_;
    std::vector<ImplMethodInfo> generic_associated_impls_;
    std::vector<GenericTraitImplInfo> generic_trait_impls_;
    std::vector<TraitImplCoherenceInfo> trait_impl_coherence_;
    std::map<std::string, ImplMethodInfo> drop_impls_;
    std::vector<ImplMethodInfo> impl_methods_to_lower_;
    std::set<std::string> queued_impl_methods_;
    std::vector<UseDecl> item_macro_uses_;
    std::vector<ModuleDecl> item_macro_modules_;
    std::vector<ConstDecl> item_macro_constants_;
    std::vector<FunctionDecl> item_macro_functions_;
    std::vector<StructDecl> item_macro_structs_;
    std::vector<EnumDecl> item_macro_enums_;
    std::vector<TraitDecl> item_macro_traits_;
    std::vector<ImplDecl> item_macro_impls_;
    std::vector<ImplDecl> derived_impls_;
    std::set<const FunctionDecl*> attribute_rewritten_functions_;
    std::set<const StructDecl*> attribute_rewritten_structs_;
    std::set<const EnumDecl*> attribute_rewritten_enums_;
    std::set<const TraitDecl*> attribute_rewritten_traits_;
    std::set<const ImplDecl*> attribute_rewritten_impls_;
    std::map<const Pattern*, Pattern> pattern_macro_expansions_;
    std::vector<IrTraitObjectVTable> trait_object_vtables_;
    std::map<std::string, std::string> trait_object_vtable_names_;
    std::map<std::string, std::string> exported_symbols_;
    std::map<std::string, std::string> emitted_function_symbols_;
    std::vector<std::string> constant_eval_stack_;
    std::vector<std::string> warnings_;
    IrType current_return_;
    std::string current_module_name_;
    std::optional<std::size_t> current_borrow_return_param_index_;
    std::string current_borrow_return_param_name_;
    std::optional<std::string> current_borrow_return_path_;
    std::optional<std::size_t> current_zone_pointer_return_param_index_;
    std::string current_zone_pointer_return_source_;
    bool allow_zone_temp_init_ = false;
    int hidden_local_counter_ = 0;

    static bool is_executable_function(const FunctionDecl& fn) {
        return !fn.meta && !fn.is_extern && fn.has_body && fn.generics.empty();
    }

    static std::string unqualified_name(const std::string& name) {
        std::size_t split = name.rfind("::");
        if (split == std::string::npos) return name;
        return name.substr(split + 2);
    }

    static bool is_planned_prelude_macro_name(const std::string& name) {
        return is_prelude_macro_name(unqualified_name(name));
    }

    static bool is_qualified_name(const std::string& name) {
        return is_qualified_path(name);
    }

    static bool is_c_symbol_name(const std::string& name) {
        if (name.empty()) return false;
        unsigned char first = static_cast<unsigned char>(name[0]);
        if (!(std::isalpha(first) || name[0] == '_')) return false;
        for (char c : name) {
            unsigned char ch = static_cast<unsigned char>(c);
            if (!(std::isalnum(ch) || c == '_')) return false;
        }
        return true;
    }

    static std::string qualify_in_module(const std::string& module_name, const std::string& name) {
        if (module_name.empty() || is_qualified_name(name)) return name;
        return module_name + "::" + name;
    }

    static std::string module_of_qualified_name(const std::string& name) {
        return qualified_parent(name);
    }

    static std::string basename_of_qualified_name(const std::string& name) {
        return qualified_basename(name);
    }

    static std::vector<std::string> split_qualified_name(const std::string& name) {
        return split_qualified_path(name);
    }

    static std::string join_qualified_parts(
        const std::vector<std::string>& parts,
        std::size_t begin,
        std::size_t end
    ) {
        return join_qualified_path(parts, begin, end);
    }

    const UseInfo* find_use_in_module(const std::string& module_name, const std::string& alias) const {
        auto scoped = uses_.find(module_name);
        if (scoped != uses_.end()) {
            auto found = scoped->second.find(alias);
            if (found != scoped->second.end()) return &found->second;
        }
        return nullptr;
    }

    const UseInfo* find_use(const std::string& alias) const {
        return find_use_in_module(current_module_name_, alias);
    }

    bool can_access_use(const UseInfo& use) const {
        return use.module_name == current_module_name_ || use.is_public;
    }

    static bool is_same_or_descendant_module(const std::string& module_name,
                                             const std::string& ancestor) {
        if (module_name == ancestor) return true;
        if (ancestor.empty()) return true;
        return module_name.size() > ancestor.size() &&
               module_name.compare(0, ancestor.size(), ancestor) == 0 &&
               module_name[ancestor.size()] == ':' &&
               module_name[ancestor.size() + 1] == ':';
    }

    bool can_access_module(const ModuleInfo& module) const {
        if (module.is_public) return true;
        return is_same_or_descendant_module(current_module_name_, module.module_name);
    }

    void require_module_path_access(SourceLocation loc, const std::string& module_name) const {
        if (module_name.empty()) return;
        std::vector<std::string> parts = split_qualified_name(module_name);
        for (std::size_t i = 1; i <= parts.size(); ++i) {
            std::string prefix = join_qualified_parts(parts, 0, i);
            auto found = modules_.find(prefix);
            if (found == modules_.end()) continue;
            if (!can_access_module(found->second)) {
                fail(loc, "module '" + prefix + "' is not public");
            }
        }
    }

    static std::string resolve_relative_name(SourceLocation loc,
                                             const std::string& module_name,
                                             const std::string& name) {
        return resolve_relative_path(loc, module_name, name);
    }

    std::string resolve_current_relative_name(const std::string& name) const {
        try {
            return resolve_relative_name(SourceLocation{}, current_module_name_, name);
        } catch (const CompileError&) {
            return name;
        }
    }

    std::string resolve_one_use_path(const std::string& name) const {
        std::string relative = resolve_current_relative_name(name);
        if (const UseInfo* direct = find_use(relative)) return direct->path;
        if (!is_qualified_name(relative)) {
            return relative;
        }

        std::vector<std::string> parts = split_qualified_name(relative);
        for (std::size_t split = parts.size() - 1; split >= 1; --split) {
            std::string module_name = join_qualified_parts(parts, 0, split);
            const std::string& alias = parts[split];
            std::string candidate_module = join_qualified_parts(parts, 0, split + 1);
            if (modules_.count(candidate_module)) {
                if (split == 1) break;
                continue;
            }
            if (const UseInfo* use = find_use_in_module(module_name, alias)) {
                if (can_access_use(*use)) {
                    std::string rest = join_qualified_parts(parts, split + 1, parts.size());
                    if (!rest.empty() && !modules_.count(use->path)) {
                        if (split == 1) break;
                        continue;
                    }
                    return rest.empty() ? use->path : use->path + "::" + rest;
                }
            }
            if (split == 1) break;
        }

        const UseInfo* prefix = find_use(parts[0]);
        if (prefix) {
            std::string rest = join_qualified_parts(parts, 1, parts.size());
            if (!rest.empty() && !modules_.count(prefix->path)) {
                std::string std_child_module = "std::" + parts[0];
                std::string std_child_prefix = std_child_module + "::";
                if (prefix->path.rfind(std_child_prefix, 0) == 0 && modules_.count(std_child_module)) {
                    return std_child_module + "::" + rest;
                }
                return relative;
            }
            return rest.empty() ? prefix->path : prefix->path + "::" + rest;
        }

        return relative;
    }

    std::string resolve_use_path(const std::string& name) const {
        std::string resolved = name;
        for (int i = 0; i < 16; ++i) {
            std::string next = resolve_one_use_path(resolved);
            if (next == resolved) return resolved;
            resolved = std::move(next);
        }
        return resolved;
    }

    std::string import_or_qualified_name(const std::string& name) const {
        return resolve_use_path(name);
    }

    std::string resolve_enum_type_name(const std::string& name) const {
        std::string resolved = import_or_qualified_name(name);
        if (is_qualified_name(resolved) || current_module_name_.empty()) return resolved;
        std::string scoped = current_module_name_ + "::" + resolved;
        if (enums_.count(scoped)) return scoped;
        return resolved;
    }

    std::string resolve_struct_type_name(const std::string& name) const {
        std::string resolved = import_or_qualified_name(name);
        if (is_qualified_name(resolved) || current_module_name_.empty()) return resolved;
        std::string scoped = current_module_name_ + "::" + resolved;
        if (structs_.count(scoped)) return scoped;
        return resolved;
    }

    std::string resolve_function_name(const std::string& name) const {
        std::string resolved = import_or_qualified_name(name);
        if (is_qualified_name(resolved) || current_module_name_.empty()) return resolved;
        std::string scoped = current_module_name_ + "::" + resolved;
        if (functions_.count(scoped)) return scoped;
        return resolved;
    }

    std::string resolve_generic_function_name(const std::string& name) const {
        std::string resolved = import_or_qualified_name(name);
        if (is_qualified_name(resolved) || current_module_name_.empty()) return resolved;
        std::string scoped = current_module_name_ + "::" + resolved;
        if (generic_functions_.count(scoped)) return scoped;
        return resolved;
    }

    std::string resolve_constant_name(const std::string& name) const {
        std::string resolved = import_or_qualified_name(name);
        if (is_qualified_name(resolved) || current_module_name_.empty()) return resolved;
        std::string scoped = current_module_name_ + "::" + resolved;
        if (constants_.count(scoped)) return scoped;
        return resolved;
    }

    std::string resolve_meta_function_name(const std::string& name) const {
        std::string resolved = import_or_qualified_name(name);
        if (is_qualified_name(resolved) || current_module_name_.empty()) return resolved;
        std::string scoped = current_module_name_ + "::" + resolved;
        if (meta_functions_.count(scoped)) return scoped;
        return resolved;
    }

    const MetaFunctionInfo& require_meta_invocation(SourceLocation loc,
                                                    MetaInvocationSite site,
                                                    const std::string& name) const {
        std::string meta_name = resolve_meta_function_name(name);
        auto found = meta_functions_.find(meta_name);
        if (found == meta_functions_.end()) {
            fail(loc, unknown_meta_invocation_message(site, name));
        }
        if (!meta_transform_allowed_at_site(site, found->second.transform_kind)) {
            fail(loc, meta_invocation_domain_message(site, name, meta_name, found->second.transform_kind));
        }
        if (found->second.ast_return_kind == MetaAstReturnKind::Expression &&
            site != MetaInvocationSite::ExpressionMacro) {
            fail(loc,
                 "non-identity ast meta function '" + meta_name +
                     "' can currently be used only at expression macro sites");
        }
        if (found->second.ast_return_kind == MetaAstReturnKind::ItemDeclarations &&
            site != MetaInvocationSite::ItemMacro &&
            site != MetaInvocationSite::Attribute) {
            fail(loc,
                 "declaration-returning ast meta function '" + meta_name +
                     "' can currently be used only at item macro or attribute sites");
        }
        if (found->second.ast_return_kind == MetaAstReturnKind::Pattern &&
            site != MetaInvocationSite::PatternMacro) {
            fail(loc,
                 "pattern-returning ast meta function '" + meta_name +
                     "' can currently be used only at pattern macro sites");
        }
        return found->second;
    }

    std::string resolve_enum_case_name(const std::string& name) const {
        std::string resolved = import_or_qualified_name(name);
        if (is_qualified_name(resolved) || current_module_name_.empty()) return resolved;
        std::string scoped = current_module_name_ + "::" + resolved;
        if (enum_cases_.count(scoped)) return scoped;
        return resolved;
    }

    std::string resolve_trait_name(const std::string& name) const {
        std::string resolved = import_or_qualified_name(name);
        if (is_qualified_name(resolved) || current_module_name_.empty()) return resolved;
        std::string scoped = current_module_name_ + "::" + resolved;
        if (traits_.count(scoped)) return scoped;
        return resolved;
    }

    bool try_resolve_associated_receiver_type(SourceLocation loc, const std::string& name, IrType& out) {
        std::string struct_name = resolve_struct_type_name(name);
        if (structs_.count(struct_name)) {
            TypeRef type;
            type.name = name;
            type.loc = loc;
            out = resolve_executable_type(type);
            return true;
        }

        std::string enum_name = resolve_enum_type_name(name);
        if (enums_.count(enum_name)) {
            TypeRef type;
            type.name = name;
            type.loc = loc;
            out = resolve_executable_type(type);
            return true;
        }

        if (!is_qualified_name(name)) {
            std::string base = unqualified_name(name);
            if (base == "i8" || base == "i16" || base == "i32" || base == "i64" ||
                base == "u8" || base == "u16" || base == "u32" || base == "u64" ||
                base == "bool" || base == "string") {
                TypeRef type;
                type.name = name;
                type.loc = loc;
                out = resolve_executable_type(type);
                return true;
            }
        }

        return false;
    }

    bool can_access(const std::string& module_name, bool is_public) const {
        return module_name.empty() || module_name == current_module_name_ || is_public;
    }

    void require_enum_case_access(SourceLocation loc, const EnumCaseInfo& info) const {
        auto enum_found = enums_.find(info.enum_name);
        if (enum_found == enums_.end()) fail(loc, "unknown enum '" + info.enum_name + "'");
        require_module_path_access(loc, enum_found->second.module_name);
        if (!can_access(enum_found->second.module_name, enum_found->second.is_public)) {
            fail(loc, "enum case '" + info.name + "' is not public");
        }
    }

    void require_struct_access(SourceLocation loc, const StructInfo& info) const {
        require_module_path_access(loc, info.module_name);
        if (!can_access(info.module_name, info.is_public)) {
            fail(loc, "struct '" + info.name + "' is not public");
        }
    }

    void require_function_access(SourceLocation loc, const FunctionSig& sig, const std::string& name) const {
        require_module_path_access(loc, sig.module_name);
        if (!can_access(sig.module_name, sig.is_public)) {
            fail(loc, "function '" + name + "' is not public");
        }
    }

    void require_constant_access(SourceLocation loc, const ConstantInfo& info, const std::string& name) const {
        require_module_path_access(loc, info.module_name);
        if (!can_access(info.module_name, info.is_public)) {
            fail(loc, "constant '" + name + "' is not public");
        }
    }

    void require_trait_access(SourceLocation loc, const TraitInfo& info) const {
        require_module_path_access(loc, info.module_name);
        if (!can_access(info.module_name, info.is_public)) {
            fail(loc, "trait '" + info.name + "' is not public");
        }
    }

    void require_impl_method_access(SourceLocation loc, const ImplMethodInfo& info, const std::string& method_name) const {
        require_module_path_access(loc, info.module_name);
        if (!can_access(info.module_name, info.is_public)) {
            fail(loc, "method '" + method_name + "' for type " + type_name(info.receiver_type) + " is not public");
        }
    }

    void require_function_decl_access(SourceLocation loc, const FunctionDecl& fn, const std::string& name) const {
        require_module_path_access(loc, fn.module_name);
        if (!can_access(fn.module_name, fn.is_public)) {
            fail(loc, "function '" + name + "' is not public");
        }
    }

    template <typename Visitor>
    void for_each_function_decl(Visitor&& visitor) const {
        for (const auto& fn : program_.functions) {
            if (!attribute_rewritten_functions_.count(&fn)) visitor(fn);
        }
        for (const auto& fn : item_macro_functions_) visitor(fn);
    }

    template <typename Visitor>
    void for_each_constant_decl(Visitor&& visitor) const {
        for (const auto& decl : program_.constants) visitor(decl);
        for (const auto& decl : item_macro_constants_) visitor(decl);
    }

    template <typename Visitor>
    void for_each_struct_decl(Visitor&& visitor) const {
        for (const auto& decl : program_.structs) {
            if (!attribute_rewritten_structs_.count(&decl)) visitor(decl);
        }
        for (const auto& decl : item_macro_structs_) visitor(decl);
    }

    template <typename Visitor>
    void for_each_enum_decl(Visitor&& visitor) const {
        for (const auto& decl : program_.enums) {
            if (!attribute_rewritten_enums_.count(&decl)) visitor(decl);
        }
        for (const auto& decl : item_macro_enums_) visitor(decl);
    }

    template <typename Visitor>
    void for_each_trait_decl(Visitor&& visitor) const {
        for (const auto& decl : program_.traits) {
            if (!attribute_rewritten_traits_.count(&decl)) visitor(decl);
        }
        for (const auto& decl : item_macro_traits_) visitor(decl);
    }

    template <typename Visitor>
    void for_each_impl_decl(Visitor&& visitor) const {
        for (const auto& decl : program_.impls) {
            if (!attribute_rewritten_impls_.count(&decl)) visitor(decl);
        }
        for (const auto& decl : item_macro_impls_) visitor(decl);
        for (const auto& decl : derived_impls_) visitor(decl);
    }

    template <typename Visitor>
    void for_each_module_decl(Visitor&& visitor) const {
        for (const auto& decl : program_.modules) visitor(decl);
        for (const auto& decl : item_macro_modules_) visitor(decl);
    }

    void collect_module_decl(const ModuleDecl& decl) {
        ModuleInfo info;
        info.name = decl.name;
        info.module_name = decl.module_name;
        info.is_public = decl.is_public;
        info.loc = decl.loc;
        auto inserted = modules_.emplace(decl.name, std::move(info));
        if (!inserted.second) fail(decl.loc, "duplicate module '" + decl.name + "'");
    }

    void collect_module_decls() {
        for (const auto& decl : program_.modules) collect_module_decl(decl);
    }

    void collect_item_macro_module_decls() {
        for (const auto& decl : item_macro_modules_) collect_module_decl(decl);
    }

    void collect_uses() {
        for (const auto& use : program_.uses) {
            if (use.is_glob) collect_glob_use(use);
            else add_use_info(use.module_name, use.alias, use.path, use.is_public, use.loc);
        }
        collect_implicit_std_prelude_uses();
    }

    void collect_item_macro_uses() {
        for (const auto& use : item_macro_uses_) {
            if (use.is_glob) collect_glob_use(use);
            else add_use_info(use.module_name, use.alias, use.path, use.is_public, use.loc);
        }
        collect_implicit_std_prelude_uses();
    }

    void add_use_info(
        const std::string& module_name,
        const std::string& alias,
        const std::string& path,
        bool is_public,
        SourceLocation loc
    ) {
        auto& scope = uses_[module_name];
        std::string resolved_path = resolve_relative_name(loc, module_name, path);
        auto inserted = scope.emplace(alias, UseInfo{resolved_path, module_name, is_public, loc});
        if (!inserted.second) fail(loc, "duplicate use alias '" + alias + "'");
    }

    static bool is_std_module_name(const std::string& module_name) {
        return module_name == "std" ||
               (module_name.size() > 5 && module_name.compare(0, 5, "std::") == 0);
    }

    static bool is_std_descendant_module_name(const std::string& module_name) {
        return module_name.size() > 5 && module_name.compare(0, 5, "std::") == 0;
    }

    bool has_source_std_surface() const {
        bool found_slice = false;
        bool found_option = false;
        for_each_struct_decl([&](const StructDecl& decl) {
            if (decl.name == "std::Slice") found_slice = true;
        });
        for_each_enum_decl([&](const EnumDecl& decl) {
            if (decl.name == "std::Option") found_option = true;
        });
        return found_slice && found_option;
    }

    bool prelude_specials_available() const {
        return has_source_std_surface();
    }

    bool source_std_generic_function_available(const std::string& name) const {
        auto found = generic_functions_.find(resolve_generic_function_name(name));
        if (found != generic_functions_.end() && is_std_module_name(found->second->module_name)) {
            return true;
        }
        std::string resolved = resolve_use_path(name);
        std::string resolved_base = basename_of_qualified_name(resolved);
        bool resolved_is_special = is_source_declared_prelude_special_name(resolved);
        bool found_source_std_generic = false;
        for_each_function_decl([&](const FunctionDecl& fn) {
            if (fn.generics.empty() || !is_std_module_name(fn.module_name)) return;
            std::string qualified = qualify_in_module(fn.module_name, fn.name);
            if (qualified == resolved && is_source_declared_prelude_special_name(qualified)) {
                found_source_std_generic = true;
                return;
            }
            if (resolved_is_special &&
                basename_of_qualified_name(qualified) == resolved_base &&
                is_source_declared_prelude_special_name(qualified)) {
                found_source_std_generic = true;
            }
        });
        return found_source_std_generic;
    }

    std::vector<std::pair<std::string, std::string>> implicit_std_prelude_items() const {
        std::vector<std::pair<std::string, std::string>> items;
        auto add = [&items](const std::string& alias, const std::string& path) {
            if (!alias.empty() && !path.empty()) items.emplace_back(alias, path);
        };
        auto add_unqualified_type_alias = [&add](const std::string& alias, const std::string& path) {
            std::size_t planned_arity = 0;
            if (planned_prelude_type_arity(alias, planned_arity)) return;
            add(alias, path);
        };

        for (const auto& decl : program_.constants) {
            if (decl.module_name == "std" && decl.is_public) {
                add(basename_of_qualified_name(decl.name), decl.name);
            }
        }
        for_each_function_decl([&](const FunctionDecl& decl) {
            if (decl.module_name == "std" && decl.is_public) {
                add(basename_of_qualified_name(decl.name), decl.name);
            }
        });
        for_each_struct_decl([&](const StructDecl& decl) {
            if (decl.module_name == "std" && decl.is_public) {
                add(basename_of_qualified_name(decl.name), decl.name);
            }
        });
        for_each_enum_decl([&](const EnumDecl& decl) {
            if (decl.module_name != "std" || !decl.is_public) return;
            add(basename_of_qualified_name(decl.name), decl.name);
            for (const auto& item : decl.cases) {
                add(item.name, qualify_in_module(decl.module_name, item.name));
            }
        });
        for_each_trait_decl([&](const TraitDecl& decl) {
            if (decl.module_name == "std" && decl.is_public) {
                add(basename_of_qualified_name(decl.name), decl.name);
            }
        });

        auto root_uses = uses_.find("std");
        if (root_uses != uses_.end()) {
            for (const auto& item : root_uses->second) {
                if (item.second.is_public) add(item.first, item.second.path);
            }
        }

        for_each_module_decl([&](const ModuleDecl& decl) {
            if (is_std_descendant_module_name(decl.name) && decl.is_public) {
                add(basename_of_qualified_name(decl.name), decl.name);
            }
        });
        for (const auto& decl : program_.constants) {
            if (is_std_descendant_module_name(decl.module_name) && decl.is_public) {
                std::string alias = basename_of_qualified_name(decl.name);
                add(alias, decl.name);
                add(basename_of_qualified_name(decl.module_name) + "::" + alias, decl.name);
            }
        }
        for_each_function_decl([&](const FunctionDecl& decl) {
            if (is_std_descendant_module_name(decl.module_name) && decl.is_public) {
                std::string alias = basename_of_qualified_name(decl.name);
                add(alias, decl.name);
                add(basename_of_qualified_name(decl.module_name) + "::" + alias, decl.name);
            }
        });
        for_each_struct_decl([&](const StructDecl& decl) {
            if (is_std_descendant_module_name(decl.module_name) && decl.is_public) {
                std::string alias = basename_of_qualified_name(decl.name);
                add_unqualified_type_alias(alias, decl.name);
                add(basename_of_qualified_name(decl.module_name) + "::" + alias, decl.name);
            }
        });
        for_each_enum_decl([&](const EnumDecl& decl) {
            if (!is_std_descendant_module_name(decl.module_name) || !decl.is_public) return;
            std::string enum_alias = basename_of_qualified_name(decl.name);
            std::string module_alias = basename_of_qualified_name(decl.module_name);
            add_unqualified_type_alias(enum_alias, decl.name);
            add(module_alias + "::" + enum_alias, decl.name);
            for (const auto& item : decl.cases) {
                std::string case_name = qualify_in_module(decl.module_name, item.name);
                add(item.name, case_name);
                add(module_alias + "::" + item.name, case_name);
            }
        });
        for_each_trait_decl([&](const TraitDecl& decl) {
            if (is_std_descendant_module_name(decl.module_name) && decl.is_public) {
                std::string alias = basename_of_qualified_name(decl.name);
                add(alias, decl.name);
                add(basename_of_qualified_name(decl.module_name) + "::" + alias, decl.name);
            }
        });

        return items;
    }

    bool module_declares_alias(const std::string& module_name, const std::string& alias) const {
        bool has_module_alias = false;
        for_each_module_decl([&](const ModuleDecl& decl) {
            if (decl.module_name == module_name && basename_of_qualified_name(decl.name) == alias) has_module_alias = true;
        });
        if (has_module_alias) return true;
        bool has_constant_alias = false;
        for_each_constant_decl([&](const ConstDecl& decl) {
            if (decl.module_name == module_name && basename_of_qualified_name(decl.name) == alias) has_constant_alias = true;
        });
        if (has_constant_alias) return true;
        bool has_function_alias = false;
        for_each_function_decl([&](const FunctionDecl& decl) {
            if (decl.module_name == module_name && basename_of_qualified_name(decl.name) == alias) has_function_alias = true;
        });
        if (has_function_alias) return true;
        bool has_struct_alias = false;
        for_each_struct_decl([&](const StructDecl& decl) {
            if (decl.module_name == module_name && basename_of_qualified_name(decl.name) == alias) has_struct_alias = true;
        });
        if (has_struct_alias) return true;
        bool has_enum_alias = false;
        for_each_enum_decl([&](const EnumDecl& decl) {
            if (decl.module_name != module_name) return;
            if (basename_of_qualified_name(decl.name) == alias) {
                has_enum_alias = true;
                return;
            }
            for (const auto& item : decl.cases) {
                if (item.name == alias) has_enum_alias = true;
            }
        });
        if (has_enum_alias) return true;
        bool has_trait_alias = false;
        for_each_trait_decl([&](const TraitDecl& decl) {
            if (decl.module_name == module_name && basename_of_qualified_name(decl.name) == alias) has_trait_alias = true;
        });
        if (has_trait_alias) return true;
        return false;
    }

    bool unqualified_decl_shadows_prelude_name(const std::string& name, const std::string& resolved) const {
        return !is_qualified_name(name) &&
               resolved == name &&
               module_declares_alias(current_module_name_, name);
    }

    static PreludeMacroKind prelude_macro_kind_for_resolved_name(const std::string& name) {
        std::string base = basename_of_qualified_name(name);
        PreludeMacroKind kind = prelude_macro_kind(base);
        if (kind == PreludeMacroKind::None) return PreludeMacroKind::None;
        if (kind == PreludeMacroKind::Vec && name == "std::vec::Vec") return kind;
        if (name == base || name == "std::" + base) return kind;
        return PreludeMacroKind::None;
    }

    void add_implicit_use_info(const std::string& module_name,
                               const std::string& alias,
                               const std::string& path) {
        auto& scope = uses_[module_name];
        if (scope.count(alias) || module_declares_alias(module_name, alias)) return;
        scope.emplace(alias, UseInfo{path, module_name, false, SourceLocation{1, 1}});
    }

    void collect_implicit_std_prelude_uses() {
        if (!options_.implicit_std || !has_source_std_surface()) return;

        std::set<std::string> module_names;
        module_names.insert("");
        for_each_module_decl([&](const ModuleDecl& decl) {
            module_names.insert(decl.name);
        });
        for (const auto& decl : program_.constants) module_names.insert(decl.module_name);
        for_each_function_decl([&](const FunctionDecl& decl) {
            module_names.insert(decl.module_name);
        });
        for_each_struct_decl([&](const StructDecl& decl) {
            module_names.insert(decl.module_name);
        });
        for_each_enum_decl([&](const EnumDecl& decl) {
            module_names.insert(decl.module_name);
        });
        for_each_trait_decl([&](const TraitDecl& decl) {
            module_names.insert(decl.module_name);
        });
        for_each_impl_decl([&](const ImplDecl& decl) {
            module_names.insert(decl.module_name);
        });

        std::vector<std::pair<std::string, std::string>> prelude_items = implicit_std_prelude_items();
        for (const auto& module_name : module_names) {
            if (is_std_module_name(module_name)) continue;
            for (const auto& item : prelude_items) {
                add_implicit_use_info(module_name, item.first, item.second);
            }
        }
    }

    bool can_glob_import_item(const UseDecl& use, const std::string& item_module, bool item_public) const {
        return item_module == use.path && (item_public || item_module == use.module_name);
    }

    void collect_glob_use(const UseDecl& use) {
        std::string relative_path = resolve_relative_name(use.loc, use.module_name, use.path);
        std::string previous_module = current_module_name_;
        current_module_name_ = use.module_name;
        std::string module_path = resolve_use_path(relative_path);
        current_module_name_ = previous_module;
        require_module_path_access(use.loc, module_path);
        UseDecl expanded = use;
        expanded.path = module_path;

        for_each_function_decl([&](const FunctionDecl& fn) {
            if (can_glob_import_item(expanded, fn.module_name, fn.is_public)) {
                add_use_info(use.module_name, basename_of_qualified_name(fn.name), fn.name, use.is_public, use.loc);
            }
        });
        for_each_constant_decl([&](const ConstDecl& decl) {
            if (can_glob_import_item(expanded, decl.module_name, decl.is_public)) {
                add_use_info(use.module_name, basename_of_qualified_name(decl.name), decl.name, use.is_public, use.loc);
            }
        });
        for_each_struct_decl([&](const StructDecl& decl) {
            if (can_glob_import_item(expanded, decl.module_name, decl.is_public)) {
                add_use_info(use.module_name, basename_of_qualified_name(decl.name), decl.name, use.is_public, use.loc);
            }
        });
        for_each_enum_decl([&](const EnumDecl& decl) {
            if (!can_glob_import_item(expanded, decl.module_name, decl.is_public)) return;
            add_use_info(use.module_name, basename_of_qualified_name(decl.name), decl.name, use.is_public, use.loc);
            for (const auto& item : decl.cases) {
                std::string case_name = qualify_in_module(decl.module_name, item.name);
                add_use_info(use.module_name, item.name, case_name, use.is_public, use.loc);
            }
        });
        for_each_trait_decl([&](const TraitDecl& decl) {
            if (can_glob_import_item(expanded, decl.module_name, decl.is_public)) {
                add_use_info(use.module_name, basename_of_qualified_name(decl.name), decl.name, use.is_public, use.loc);
            }
        });
    }

    static void require_unique_generic_params(
        const std::vector<GenericParam>& generics,
        const std::string& owner_kind,
        const std::string& owner_name
    ) {
        std::set<std::string> names;
        for (const auto& generic : generics) {
            if (!names.insert(generic.name).second) {
                fail(generic.loc, "duplicate generic parameter '" + generic.name + "' in " + owner_kind + " '" + owner_name + "'");
            }
        }
    }

    void collect_meta_functions() {
        for (const auto& fn : program_.functions) {
            if (!fn.meta) continue;
            MetaTransformKind transform_kind = validate_meta_function_signature(fn);
            auto inserted = meta_functions_.emplace(
                fn.name,
                MetaFunctionInfo{
                    fn.name,
                    fn.module_name,
                    transform_kind,
                    fn.loc,
                    fn.params[0].name,
                    meta_function_ast_return_kind(fn),
                    meta_function_token_return(fn),
                    meta_function_ast_return(fn),
                    meta_function_type_return(fn),
                }
            );
            if (!inserted.second) fail(fn.loc, "duplicate meta function '" + fn.name + "'");
        }
    }

    static bool is_builtin_attribute(const std::string& name) {
        return name == "derive" ||
               name == "deprecated" ||
               name == "export" ||
               name == "no_mangle" ||
               name == "borrow_return" ||
               name == "repr" ||
               name == "test" ||
               name == "cfg";
    }

    static void require_attribute_args(const Attribute& attr) {
        if (!attr.has_args || attr.args.empty()) {
            fail(attr.loc, "attribute '@" + attr.name + "' expects arguments");
        }
    }

    static void reject_attribute_args(const Attribute& attr) {
        if (attr.has_args) {
            fail(attr.loc, "attribute '@" + attr.name + "' does not take arguments");
        }
    }

    void validate_builtin_attribute(const Attribute& attr, const std::string& target_kind) const {
        if (attr.name == "derive") {
            if (target_kind != "struct" && target_kind != "enum") {
                fail(attr.loc, "attribute '@derive' is only supported on structs and enums");
            }
            require_attribute_args(attr);
            return;
        }
        if (attr.name == "repr") {
            if (target_kind != "struct" && target_kind != "enum") {
                fail(attr.loc, "attribute '@repr' is only supported on structs and enums");
            }
            require_attribute_args(attr);
            if (!attribute_has_single_identifier_argument(attr, "C")) {
                fail(attr.loc, "attribute '@repr' currently supports only C layout");
            }
            return;
        }
        if (attr.name == "test") {
            if (target_kind != "function") {
                fail(attr.loc, "attribute '@test' is only supported on functions");
            }
            reject_attribute_args(attr);
            return;
        }
        if (attr.name == "export") {
            if (target_kind != "function") {
                fail(attr.loc, "attribute '@export' is only supported on functions");
            }
            if (attr.has_args &&
                (attr.args.size() != 1 || attr.args[0].kind != TokenKind::String)) {
                fail(attr.loc, "attribute '@export' expects no arguments or one string symbol name");
            }
            return;
        }
        if (attr.name == "no_mangle") {
            if (target_kind != "function") {
                fail(attr.loc, "attribute '@no_mangle' is only supported on functions");
            }
            reject_attribute_args(attr);
            return;
        }
        if (attr.name == "borrow_return") {
            if (target_kind != "function") {
                fail(attr.loc, "attribute '@borrow_return' is only supported on functions");
            }
            (void)explicit_borrow_return_contract({attr});
            return;
        }
        if (attr.name == "cfg") {
            (void)cfg_attribute_enabled(attr, options_.cfg_features, options_.target_triple);
            return;
        }
        if (attr.name == "deprecated") {
            if (attr.has_args &&
                (attr.args.size() != 1 || attr.args[0].kind != TokenKind::String)) {
                fail(attr.loc, "attribute '@deprecated' expects no arguments or one string message");
            }
            return;
        }
    }

    static const Attribute* deprecated_attribute(const std::vector<Attribute>& attributes) {
        return find_attribute(attributes, "deprecated");
    }

    static std::string deprecated_message(const std::vector<Attribute>& attributes) {
        const Attribute* attr = deprecated_attribute(attributes);
        if (!attr || !attr->has_args || attr->args.empty()) return "";
        return attr->args[0].text;
    }

    std::string exported_link_name(const FunctionDecl& fn) const {
        const Attribute* export_attr = find_attribute(fn.attributes, "export");
        const Attribute* no_mangle_attr = find_attribute(fn.attributes, "no_mangle");
        if (export_attr && no_mangle_attr) {
            fail(export_attr->loc, "attributes '@export' and '@no_mangle' cannot be combined");
        }
        if (!export_attr && !no_mangle_attr) return "";
        if (fn.is_extern) {
            SourceLocation loc = export_attr ? export_attr->loc : no_mangle_attr->loc;
            fail(loc, "attribute '@" + std::string(export_attr ? "export" : "no_mangle") + "' cannot be used on extern functions");
        }
        if (fn.meta) {
            SourceLocation loc = export_attr ? export_attr->loc : no_mangle_attr->loc;
            fail(loc, "attribute '@" + std::string(export_attr ? "export" : "no_mangle") + "' cannot be used on meta functions");
        }
        if (!fn.generics.empty()) {
            SourceLocation loc = export_attr ? export_attr->loc : no_mangle_attr->loc;
            fail(loc, "exporting generic function definitions is planned after generic export ABI is defined");
        }

        std::string symbol = basename_of_qualified_name(fn.name);
        SourceLocation loc = fn.loc;
        if (export_attr) {
            loc = export_attr->loc;
            if (export_attr->has_args) symbol = export_attr->args[0].text;
        } else {
            loc = no_mangle_attr->loc;
        }
        if (!is_c_symbol_name(symbol)) {
            fail(loc, "invalid exported symbol '" + symbol + "'");
        }
        if (options_.require_main && symbol == "main") {
            fail(loc, "exported symbol 'main' conflicts with the generated C entry point");
        }
        return symbol;
    }

    void register_emitted_function_symbol(SourceLocation loc,
                                          const std::string& symbol,
                                          const std::string& function_name) {
        auto inserted = emitted_function_symbols_.emplace(symbol, function_name);
        if (inserted.second) return;
        fail(loc,
             "emitted symbol '" + symbol + "' for function '" + function_name +
                 "' conflicts with function '" + inserted.first->second + "'");
    }

    void warn_deprecated_use(SourceLocation loc,
                             const std::string& kind,
                             const std::string& name,
                             const std::string& message) {
        std::string warning = where(loc) + ": warning: use of deprecated " + kind + " '" + name + "'";
        if (!message.empty()) warning += ": " + message;
        warnings_.push_back(std::move(warning));
    }

    void warn_scalar_match_shadow(SourceLocation loc) {
        warnings_.push_back(where(loc) +
                            ": warning: scalar match pattern is fully shadowed by earlier match arms");
    }

    void warn_aggregate_match_shadow(SourceLocation loc) {
        warnings_.push_back(where(loc) +
                            ": warning: aggregate match pattern is fully shadowed by earlier match arms");
    }

    void validate_attribute_list(
        const std::vector<Attribute>& attributes,
        const std::string& target_kind,
        const std::string& module_name
    ) {
        std::string previous_module = current_module_name_;
        current_module_name_ = module_name;
        for (const auto& attr : attributes) {
            if (is_builtin_attribute(attr.name)) {
                validate_builtin_attribute(attr, target_kind);
                continue;
            }
            (void)require_meta_invocation(attr.loc, MetaInvocationSite::Attribute, attr.name);
        }
        current_module_name_ = previous_module;
    }

    void validate_attributes() {
        for_each_function_decl([&](const FunctionDecl& fn) {
            validate_attribute_list(fn.attributes, "function", fn.module_name);
        });
        for_each_struct_decl([&](const StructDecl& decl) {
            validate_attribute_list(decl.attributes, "struct", decl.module_name);
            validate_repr_c_struct(decl);
        });
        for_each_enum_decl([&](const EnumDecl& decl) {
            validate_attribute_list(decl.attributes, "enum", decl.module_name);
            validate_repr_c_enum(decl);
        });
        for_each_trait_decl([&](const TraitDecl& decl) {
            validate_attribute_list(decl.attributes, "trait", decl.module_name);
        });
        for_each_impl_decl([&](const ImplDecl& decl) {
            validate_attribute_list(decl.attributes, "impl", decl.module_name);
        });
    }

    void expand_derive_impls() {
        for_each_struct_decl([&](const StructDecl& decl) {
            std::vector<ImplDecl> impls = expand_derive_impls_for_struct(decl);
            derived_impls_.insert(
                derived_impls_.end(),
                std::make_move_iterator(impls.begin()),
                std::make_move_iterator(impls.end()));
        });
        for_each_enum_decl([&](const EnumDecl& decl) {
            std::vector<ImplDecl> impls = expand_derive_impls_for_enum(decl);
            derived_impls_.insert(
                derived_impls_.end(),
                std::make_move_iterator(impls.begin()),
                std::make_move_iterator(impls.end()));
        });
    }

    void append_item_macro_expansion(ItemMacroExpansion expansion) {
        for (auto& use : expansion.uses) item_macro_uses_.push_back(std::move(use));
        for (auto& decl : expansion.modules) item_macro_modules_.push_back(std::move(decl));
        for (auto& constant : expansion.constants) item_macro_constants_.push_back(std::move(constant));
        for (auto& fn : expansion.functions) item_macro_functions_.push_back(std::move(fn));
        for (auto& decl : expansion.structs) item_macro_structs_.push_back(std::move(decl));
        for (auto& decl : expansion.enums) item_macro_enums_.push_back(std::move(decl));
        for (auto& decl : expansion.traits) item_macro_traits_.push_back(std::move(decl));
        for (auto& decl : expansion.impls) item_macro_impls_.push_back(std::move(decl));
    }

    void expand_item_macro_invocations() {
        for (const auto& invocation : program_.item_macros) {
            std::string previous_module = current_module_name_;
            current_module_name_ = invocation.module_name;
            ItemMacroExpansion expansion;
            try {
                const MetaFunctionInfo& meta =
                    require_meta_invocation(invocation.loc, MetaInvocationSite::ItemMacro, invocation.name);
                if (meta.ast_return_kind == MetaAstReturnKind::ItemDeclarations) {
                    expansion = expand_item_macro_decl_constructor(invocation, meta.parameter_name, *meta.ast_return);
                } else if (meta.token_return) {
                    expansion = expand_item_macro_token_return(invocation, meta.parameter_name, *meta.token_return);
                } else {
                    expansion = expand_item_macro_items(invocation);
                }
            } catch (...) {
                current_module_name_ = previous_module;
                throw;
            }
            current_module_name_ = previous_module;
            append_item_macro_expansion(std::move(expansion));
        }
    }

    static bool attribute_meta_rewrites_declaration(const MetaFunctionInfo& meta) {
        return meta.token_return || meta.ast_return_kind == MetaAstReturnKind::ItemDeclarations;
    }

    ItemMacroExpansion expand_attribute_macro_output(const Attribute& attr,
                                                     const MetaFunctionInfo& meta,
                                                     const std::vector<Token>& declaration_tokens,
                                                     const std::string& module_name,
                                                     SourceLocation loc) {
        if (declaration_tokens.empty()) {
            fail(attr.loc, "attribute macro declaration input is unavailable for this declaration");
        }
        if (meta.ast_return_kind == MetaAstReturnKind::ItemDeclarations) {
            return expand_attribute_macro_decl_constructor(
                declaration_tokens, module_name, loc, meta.parameter_name, *meta.ast_return);
        }
        if (meta.token_return) {
            return expand_attribute_macro_token_return(
                declaration_tokens, attr, module_name, loc, meta.parameter_name, *meta.token_return);
        }
        fail(attr.loc, "internal error: attribute meta function does not rewrite declarations");
    }

    template <typename MarkRewritten>
    void expand_attribute_macros_for_declaration(const std::vector<Attribute>& attributes,
                                                 const std::vector<Token>& declaration_tokens,
                                                 const std::string& module_name,
                                                 SourceLocation loc,
                                                 const std::string& target_kind,
                                                 MarkRewritten mark_rewritten) {
        const Attribute* rewriting_attr = nullptr;
        const MetaFunctionInfo* rewriting_meta = nullptr;
        if (find_rewriting_attribute_macro(
                attributes,
                module_name,
                target_kind,
                rewriting_attr,
                rewriting_meta)) {
            ItemMacroExpansion expansion = expand_attribute_macro_output(
                *rewriting_attr,
                *rewriting_meta,
                declaration_tokens,
                module_name,
                loc);
            append_item_macro_expansion(std::move(expansion));
            mark_rewritten();
        }
    }

    bool find_rewriting_attribute_macro(const std::vector<Attribute>& attributes,
                                        const std::string& module_name,
                                        const std::string& target_kind,
                                        const Attribute*& rewriting_attr,
                                        const MetaFunctionInfo*& rewriting_meta) {
        bool rewritten = false;
        for (const auto& attr : attributes) {
            if (is_builtin_attribute(attr.name)) continue;

            std::string previous_module = current_module_name_;
            current_module_name_ = module_name;
            try {
                const MetaFunctionInfo& meta =
                    require_meta_invocation(attr.loc, MetaInvocationSite::Attribute, attr.name);
                if (!attribute_meta_rewrites_declaration(meta)) {
                    current_module_name_ = previous_module;
                    continue;
                }
                if (rewritten) {
                    fail(attr.loc,
                         target_kind + " declarations can currently use only one rewriting attribute macro");
                }
                rewriting_attr = &attr;
                rewriting_meta = &meta;
                rewritten = true;
            } catch (...) {
                current_module_name_ = previous_module;
                throw;
            }
            current_module_name_ = previous_module;
        }
        return rewritten;
    }

    template <typename Decl>
    bool expand_generated_attribute_macros_in(std::vector<Decl>& declarations,
                                              const std::string& target_kind) {
        for (std::size_t i = 0; i < declarations.size(); ++i) {
            const Attribute* rewriting_attr = nullptr;
            const MetaFunctionInfo* rewriting_meta = nullptr;
            if (!find_rewriting_attribute_macro(
                    declarations[i].attributes,
                    declarations[i].module_name,
                    target_kind,
                    rewriting_attr,
                    rewriting_meta)) {
                continue;
            }
            ItemMacroExpansion expansion = expand_attribute_macro_output(
                *rewriting_attr,
                *rewriting_meta,
                declarations[i].source_tokens,
                declarations[i].module_name,
                declarations[i].loc);
            using Difference = typename std::vector<Decl>::difference_type;
            declarations.erase(declarations.begin() + static_cast<Difference>(i));
            append_item_macro_expansion(std::move(expansion));
            return true;
        }
        return false;
    }

    bool expand_generated_attribute_macros_once() {
        if (expand_generated_attribute_macros_in(item_macro_functions_, "function")) return true;
        if (expand_generated_attribute_macros_in(item_macro_structs_, "struct")) return true;
        if (expand_generated_attribute_macros_in(item_macro_enums_, "enum")) return true;
        if (expand_generated_attribute_macros_in(item_macro_traits_, "trait")) return true;
        if (expand_generated_attribute_macros_in(item_macro_impls_, "impl")) return true;
        return false;
    }

    void expand_generated_attribute_macros() {
        constexpr std::size_t max_generated_attribute_expansions = 1024;
        for (std::size_t expansions = 0; expand_generated_attribute_macros_once(); ++expansions) {
            if (expansions >= max_generated_attribute_expansions) {
                fail(SourceLocation{},
                     "attribute macro expansion exceeded the generated-attribute recursion limit");
            }
        }
    }

    void reject_rewriting_attribute_macros_on_meta_function(const FunctionDecl& fn) {
        for (const auto& attr : fn.attributes) {
            if (is_builtin_attribute(attr.name)) continue;

            std::string previous_module = current_module_name_;
            current_module_name_ = fn.module_name;
            try {
                const MetaFunctionInfo& meta =
                    require_meta_invocation(attr.loc, MetaInvocationSite::Attribute, attr.name);
                if (attribute_meta_rewrites_declaration(meta)) {
                    fail(attr.loc, "attribute macros cannot rewrite meta functions yet");
                }
            } catch (...) {
                current_module_name_ = previous_module;
                throw;
            }
            current_module_name_ = previous_module;
        }
    }

    void expand_attribute_macros() {
        for (const auto& fn : program_.functions) {
            if (fn.meta) {
                reject_rewriting_attribute_macros_on_meta_function(fn);
                continue;
            }
            expand_attribute_macros_for_declaration(
                fn.attributes,
                fn.source_tokens,
                fn.module_name,
                fn.loc,
                "function",
                [&]() { attribute_rewritten_functions_.insert(&fn); });
        }
        for (const auto& decl : program_.structs) {
            expand_attribute_macros_for_declaration(
                decl.attributes,
                decl.source_tokens,
                decl.module_name,
                decl.loc,
                "struct",
                [&]() { attribute_rewritten_structs_.insert(&decl); });
        }
        for (const auto& decl : program_.enums) {
            expand_attribute_macros_for_declaration(
                decl.attributes,
                decl.source_tokens,
                decl.module_name,
                decl.loc,
                "enum",
                [&]() { attribute_rewritten_enums_.insert(&decl); });
        }
        for (const auto& decl : program_.traits) {
            expand_attribute_macros_for_declaration(
                decl.attributes,
                decl.source_tokens,
                decl.module_name,
                decl.loc,
                "trait",
                [&]() { attribute_rewritten_traits_.insert(&decl); });
        }
        for (const auto& decl : program_.impls) {
            expand_attribute_macros_for_declaration(
                decl.attributes,
                decl.source_tokens,
                decl.module_name,
                decl.loc,
                "impl",
                [&]() { attribute_rewritten_impls_.insert(&decl); });
        }
    }

    const Pattern& expanded_pattern(const Pattern& pattern) const {
        auto found = pattern_macro_expansions_.find(&pattern);
        if (found == pattern_macro_expansions_.end()) return pattern;
        return found->second;
    }

    static bool pattern_tree_has_macro_invocation(const Pattern& pattern) {
        if (pattern.is_macro_invocation) return true;
        if (pattern.payload_pattern && pattern_tree_has_macro_invocation(*pattern.payload_pattern)) return true;
        if (pattern.alias_pattern && pattern_tree_has_macro_invocation(*pattern.alias_pattern)) return true;
        for (const auto& alternative : pattern.alternatives) {
            if (pattern_tree_has_macro_invocation(alternative)) return true;
        }
        for (const auto& element : pattern.elements) {
            if (pattern_tree_has_macro_invocation(element)) return true;
        }
        return false;
    }

    Pattern expand_pattern_macro_tree(const Pattern& pattern) {
        if (pattern.is_macro_invocation) {
            const MetaFunctionInfo& meta =
                require_meta_invocation(pattern.loc, MetaInvocationSite::PatternMacro, pattern.case_name);
            Pattern expanded;
            if (meta.ast_return_kind == MetaAstReturnKind::Pattern) {
                expanded = expand_pattern_macro_constructor(pattern, meta.parameter_name, *meta.ast_return);
            } else if (meta.token_return) {
                expanded = expand_pattern_macro_token_return(pattern, meta.parameter_name, *meta.token_return);
            } else {
                expanded = expand_pattern_macro_invocation(pattern);
            }
            return expand_pattern_macro_tree(expanded);
        }

        Pattern expanded = clone_pattern(pattern);
        if (pattern.payload_pattern) {
            expanded.payload_pattern = std::make_unique<Pattern>(expand_pattern_macro_tree(*pattern.payload_pattern));
        }
        if (pattern.alias_pattern) {
            expanded.alias_pattern = std::make_unique<Pattern>(expand_pattern_macro_tree(*pattern.alias_pattern));
        }
        expanded.alternatives.clear();
        expanded.alternatives.reserve(pattern.alternatives.size());
        for (const auto& alternative : pattern.alternatives) {
            expanded.alternatives.push_back(expand_pattern_macro_tree(alternative));
        }
        expanded.elements.clear();
        expanded.elements.reserve(pattern.elements.size());
        for (const auto& element : pattern.elements) {
            expanded.elements.push_back(expand_pattern_macro_tree(element));
        }
        return expanded;
    }

    void validate_pattern_macro(const Pattern& pattern) {
        if (!pattern_tree_has_macro_invocation(pattern)) return;
        pattern_macro_expansions_[&pattern] = expand_pattern_macro_tree(pattern);
    }

    void validate_expr_pattern_macros(const ExprPtr& expr) {
        if (!expr) return;
        validate_expr_pattern_macros(expr_operand(*expr));
        validate_expr_pattern_macros(expr_left(*expr));
        validate_expr_pattern_macros(expr_right(*expr));
        for (const auto& arg : expr->args) validate_expr_pattern_macros(arg);
        if (expr->if_payload) {
            validate_expr_pattern_macros(expr->if_payload->condition);
            if (expr->if_payload->condition_pattern) validate_pattern_macro(*expr->if_payload->condition_pattern);
            validate_stmt_list_pattern_macros(expr->if_payload->then_body);
            validate_expr_pattern_macros(expr->if_payload->then_value);
            validate_stmt_list_pattern_macros(expr->if_payload->else_body);
            validate_expr_pattern_macros(expr->if_payload->else_value);
        }
        if (expr->block_payload) {
            validate_stmt_list_pattern_macros(expr->block_payload->body);
            validate_expr_pattern_macros(expr->block_payload->value);
        }
        if (expr->match_payload) {
            validate_expr_pattern_macros(expr->match_payload->value);
            for (const auto& arm : expr->match_payload->arms) {
                validate_pattern_macro(arm.pattern);
                validate_expr_pattern_macros(arm.value);
            }
        }
    }

    void validate_binding_pattern_macros(const Binding& binding) {
        if (binding.has_pattern) validate_pattern_macro(binding.pattern);
        validate_expr_pattern_macros(binding.init);
    }

    void validate_stmt_pattern_macros(const Stmt& stmt) {
        validate_binding_pattern_macros(stmt.binding);
        validate_expr_pattern_macros(stmt.expr);
        validate_expr_pattern_macros(stmt.condition);
        validate_expr_pattern_macros(stmt.for_iterable);
        validate_expr_pattern_macros(stmt.match_value);
        if (stmt.has_condition_pattern && stmt.condition_pattern) validate_pattern_macro(*stmt.condition_pattern);
        if (stmt.for_pattern) validate_pattern_macro(*stmt.for_pattern);
        for (const auto& binding : stmt.init_bindings) validate_binding_pattern_macros(binding);
        for (const auto& update : stmt.updates) validate_expr_pattern_macros(update);
        if (stmt.assign_payload) {
            validate_expr_pattern_macros(stmt.assign_payload->target);
            validate_expr_pattern_macros(stmt.assign_payload->rhs);
        }
        if (stmt.body_payload) {
            validate_stmt_list_pattern_macros(stmt.body_payload->statements);
            validate_stmt_list_pattern_macros(stmt.body_payload->then_body);
            validate_stmt_list_pattern_macros(stmt.body_payload->else_body);
            validate_stmt_list_pattern_macros(stmt.body_payload->loop_body);
        }
        if (stmt.match_arms) {
            for (const auto& arm : *stmt.match_arms) {
                validate_pattern_macro(arm.pattern);
                validate_stmt_list_pattern_macros(arm.body);
            }
        }
        if (stmt.break_payload) validate_expr_pattern_macros(stmt.break_payload->value);
    }

    void validate_stmt_list_pattern_macros(const std::vector<StmtPtr>& statements) {
        for (const auto& stmt : statements) {
            if (stmt) validate_stmt_pattern_macros(*stmt);
        }
    }

    void validate_function_pattern_macros(const FunctionDecl& fn) {
        std::string previous_module = current_module_name_;
        current_module_name_ = fn.module_name;
        for (const auto& param : fn.params) {
            if (param.has_pattern) validate_pattern_macro(param.pattern);
        }
        validate_stmt_list_pattern_macros(fn.body);
        current_module_name_ = previous_module;
    }

    void validate_pattern_macro_invocations() {
        for_each_function_decl([&](const FunctionDecl& fn) {
            validate_function_pattern_macros(fn);
        });
        for_each_trait_decl([&](const TraitDecl& trait) {
            for (const auto& method : trait.methods) validate_function_pattern_macros(method);
        });
        for_each_impl_decl([&](const ImplDecl& impl) {
            for (const auto& method : impl.methods) validate_function_pattern_macros(method);
        });
    }

    void validate_repr_c_struct(const StructDecl& decl) const {
        const Attribute* repr = find_attribute(decl.attributes, "repr");
        if (!repr) return;
        for (const auto& field : decl.fields) {
            if (field.type.qualifier != TypeQualifier::Value &&
                field.type.qualifier != TypeQualifier::Ref &&
                field.type.qualifier != TypeQualifier::MutRef &&
                field.type.qualifier != TypeQualifier::Ptr) {
                fail(field.loc,
                     "attribute '@repr(C)' fields cannot use own; expose ownership through an explicit ptr/ref ABI");
            }
        }
    }

    void validate_repr_c_enum(const EnumDecl& decl) const {
        const Attribute* repr = find_attribute(decl.attributes, "repr");
        if (!repr) return;
        for (const auto& item : decl.cases) {
            if (!item.payloads.empty()) {
                fail(item.loc, "attribute '@repr(C)' currently supports only fieldless enums");
            }
        }
    }

    void collect_constant_decls() {
        for_each_constant_decl([&](const ConstDecl& decl) {
            ConstantInfo info;
            info.name = decl.name;
            info.module_name = decl.module_name;
            info.is_public = decl.is_public;
            info.type_ref = decl.type;
            info.init = decl.init.get();
            info.loc = decl.loc;
            auto inserted = constants_.emplace(decl.name, std::move(info));
            if (!inserted.second) fail(decl.loc, "duplicate constant '" + decl.name + "'");
        });
    }

    void collect_struct_decls() {
        for_each_struct_decl([&](const StructDecl& decl) {
            StructInfo info;
            info.name = decl.name;
            info.module_name = decl.module_name;
            info.generic_arity = decl.generics.size();
            info.tuple_struct = decl.tuple_struct;
            info.is_public = decl.is_public;
            info.deprecated = deprecated_attribute(decl.attributes) != nullptr;
            info.deprecated_message = deprecated_message(decl.attributes);
            info.loc = decl.loc;

            std::set<std::string> generic_names;
            for (const auto& generic : decl.generics) {
                if (!generic_names.insert(generic.name).second) {
                    fail(decl.loc, "duplicate generic parameter '" + generic.name + "' in struct '" + decl.name + "'");
                }
                info.generic_names.push_back(generic.name);
            }

            std::set<std::string> field_names;
            for (const auto& field : decl.fields) {
                if (!field_names.insert(field.name).second) {
                    fail(field.loc, "duplicate field '" + field.name + "' in struct '" + decl.name + "'");
                }
                info.fields.push_back(StructInfo::Field{field.name, field.type, field.mutable_field, field.loc});
            }

            auto inserted = structs_.emplace(decl.name, std::move(info));
            if (!inserted.second) fail(decl.loc, "duplicate struct '" + decl.name + "'");
        });
    }

    void validate_struct_decls() {
        for_each_struct_decl([&](const StructDecl& decl) {
            auto found = structs_.find(decl.name);
            if (found == structs_.end()) return;

            std::map<std::string, IrType> substitutions;
            for (const auto& generic : decl.generics) {
                IrType placeholder;
                placeholder.qualifier = TypeQualifier::Value;
                placeholder.primitive = IrPrimitiveKind::Unknown;
                placeholder.name = generic.name;
                placeholder.loc = decl.loc;
                substitutions.emplace(generic.name, placeholder);
            }

            std::string previous_module = current_module_name_;
            std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
            current_module_name_ = decl.module_name;
            current_type_substitutions_ = std::move(substitutions);
            for (const auto& field : decl.fields) {
                IrType field_type = resolve_executable_type(field.type);
                require_root_vector_runtime_abi(field.loc, field_type, "a struct field");
            }
            current_type_substitutions_ = std::move(previous_substitutions);
            current_module_name_ = previous_module;
        });
    }

    void collect_trait_decls() {
        for_each_trait_decl([&](const TraitDecl& decl) {
            require_unique_generic_params(decl.generics, "trait", decl.name);
            TraitInfo info;
            info.name = decl.name;
            info.module_name = decl.module_name;
            info.generic_arity = decl.generics.size();
            for (const auto& generic : decl.generics) info.generic_names.push_back(generic.name);
            info.is_public = decl.is_public;
            info.loc = decl.loc;
            info.supertrait_refs = decl.supertraits;

            std::set<std::string> trait_generic_names;
            for (const auto& generic : decl.generics) trait_generic_names.insert(generic.name);

            for (const auto& associated_type : decl.associated_types) {
                TraitInfo::AssociatedType trait_type;
                trait_type.name = associated_type.name;
                trait_type.loc = associated_type.loc;
                auto inserted = info.associated_types.emplace(trait_type.name, std::move(trait_type));
                if (!inserted.second) {
                    fail(associated_type.loc,
                         "duplicate associated type '" + associated_type.name + "' in trait '" + decl.name + "'");
                }
            }

            for (const auto& method : decl.methods) {
                std::string method_name = basename_of_qualified_name(method.name);
                require_unique_generic_params(method.generics, "trait method", method_name);
                if (info.associated_types.count(method_name)) {
                    fail(method.loc,
                         "method '" + method_name + "' conflicts with an associated type in trait '" + decl.name + "'");
                }
                for (const auto& generic : method.generics) {
                    if (trait_generic_names.count(generic.name)) {
                        fail(generic.loc,
                             "trait method generic parameter '" + generic.name +
                                 "' conflicts with a trait generic parameter");
                    }
                }
                if (method.is_variadic) fail(method.variadic_loc, "variadic parameters are only supported on extern \"C\" functions");
                TraitInfo::Method trait_method;
                trait_method.name = method_name;
                trait_method.generics = method.generics;
                trait_method.loc = method.loc;
                trait_method.has_result = method.has_return_type;
                trait_method.has_self_receiver = function_params_have_self_receiver(method.params);
                if (method.has_return_type) trait_method.result = method.return_type;
                for (const auto& param : method.params) {
                    if (param.has_pattern) {
                        fail(param.pattern.loc, "trait method parameters cannot use patterns");
                    }
                    trait_method.params.push_back(param.type);
                }
                auto method_inserted = info.methods.emplace(trait_method.name, std::move(trait_method));
                if (!method_inserted.second) {
                    fail(method.loc, "duplicate method '" + method_name + "' in trait '" + decl.name + "'");
                }
            }

            auto inserted = traits_.emplace(decl.name, std::move(info));
            if (!inserted.second) fail(decl.loc, "duplicate trait '" + decl.name + "'");
        });
    }

    GenericTraitBound resolve_supertrait_bound(const TypeRef& constraint) {
        if (constraint.qualifier != TypeQualifier::Value) {
            fail(constraint.loc, "supertraits cannot use ownership qualifiers");
        }

        std::string trait_name = resolve_trait_name(constraint.name);
        auto found = traits_.find(trait_name);
        if (found == traits_.end()) {
            fail(constraint.loc, "unknown supertrait '" + constraint.name + "'");
        }
        const TraitInfo& trait = found->second;
        require_trait_access(constraint.loc, trait);
        if (constraint.args.size() != trait.generic_arity) {
            fail(constraint.loc,
                 "trait '" + trait.name + "' expects " + std::to_string(trait.generic_arity) +
                     " type argument" + (trait.generic_arity == 1 ? "" : "s"));
        }

        GenericTraitBound bound;
        bound.generic_name = "Self";
        bound.trait_name = trait.name;
        bound.loc = constraint.loc;
        bound.trait_args.reserve(constraint.args.size());
        for (const auto& arg : constraint.args) {
            bound.trait_args.push_back(resolve_executable_type(arg));
        }
        return bound;
    }

    static std::map<std::string, IrType> trait_generic_placeholder_substitutions(const TraitInfo& trait) {
        std::map<std::string, IrType> substitutions;
        for (const auto& generic_name : trait.generic_names) {
            IrType placeholder;
            placeholder.qualifier = TypeQualifier::Value;
            placeholder.primitive = IrPrimitiveKind::Unknown;
            placeholder.name = generic_name;
            placeholder.loc = trait.loc;
            substitutions.emplace(generic_name, placeholder);
        }
        return substitutions;
    }

    void detect_supertrait_cycle_from(
        const std::string& current,
        std::vector<std::string>& stack,
        std::set<std::string>& visiting,
        std::set<std::string>& visited
    ) const {
        if (visited.count(current)) return;
        if (!visiting.insert(current).second) {
            fail(traits_.at(current).loc, "supertrait cycle involving trait '" + current + "'");
        }
        stack.push_back(current);

        auto supers = trait_supertraits_.find(current);
        if (supers != trait_supertraits_.end()) {
            for (const auto& supertrait : supers->second) {
                if (std::find(stack.begin(), stack.end(), supertrait.trait_name) != stack.end()) {
                    fail(supertrait.loc, "trait '" + current + "' cannot require itself through a supertrait cycle");
                }
                detect_supertrait_cycle_from(supertrait.trait_name, stack, visiting, visited);
            }
        }

        stack.pop_back();
        visiting.erase(current);
        visited.insert(current);
    }

    void resolve_trait_supertraits() {
        for (const auto& item : traits_) {
            const TraitInfo& trait = item.second;
            if (trait.supertrait_refs.empty()) continue;

            std::string previous_module = current_module_name_;
            std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
            current_module_name_ = trait.module_name;
            current_type_substitutions_ = trait_generic_placeholder_substitutions(trait);

            std::vector<GenericTraitBound> bounds;
            bounds.reserve(trait.supertrait_refs.size());
            for (const auto& supertrait : trait.supertrait_refs) {
                bounds.push_back(resolve_supertrait_bound(supertrait));
            }
            trait_supertraits_[trait.name] = std::move(bounds);

            current_type_substitutions_ = std::move(previous_substitutions);
            current_module_name_ = previous_module;
        }

        std::set<std::string> visiting;
        std::set<std::string> visited;
        std::vector<std::string> stack;
        for (const auto& item : traits_) {
            detect_supertrait_cycle_from(item.first, stack, visiting, visited);
        }
    }

    void validate_trait_method_decls() {
        for_each_trait_decl([&](const TraitDecl& decl) {
            std::map<std::string, IrType> trait_substitutions =
                generic_placeholder_substitutions(decl.generics);
            IrType self_placeholder;
            self_placeholder.qualifier = TypeQualifier::Value;
            self_placeholder.primitive = IrPrimitiveKind::Unknown;
            self_placeholder.name = "Self";
            self_placeholder.loc = decl.loc;
            trait_substitutions.emplace("Self", self_placeholder);

            std::string previous_module = current_module_name_;
            std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
            current_module_name_ = decl.module_name;

            for (const auto& method : decl.methods) {
                std::map<std::string, IrType> method_substitutions = trait_substitutions;
                for (const auto& item : generic_placeholder_substitutions(method.generics)) {
                    method_substitutions.emplace(item.first, item.second);
                }
                current_type_substitutions_ = std::move(method_substitutions);

                for (const auto& param : method.params) {
                    IrType param_type = resolve_executable_type(param.type);
                    bool vec_view = false;
                    (void)function_parameter_abi_type(
                        param.type.loc,
                        param_type,
                        "a trait method parameter",
                        vec_view);
                }
                if (method.has_return_type) {
                    IrType result_type = resolve_executable_type(method.return_type);
                    require_root_vector_runtime_abi(method.return_type.loc, result_type, "a trait method return type");
                }
            }

            current_type_substitutions_ = std::move(previous_substitutions);
            current_module_name_ = previous_module;
        });
    }

    static bool same_type_list(const std::vector<IrType>& left, const std::vector<IrType>& right) {
        if (left.size() != right.size()) return false;
        for (std::size_t i = 0; i < left.size(); ++i) {
            if (!same_type(left[i], right[i])) return false;
        }
        return true;
    }

    static IrType value_qualified_type(IrType type) {
        type.qualifier = TypeQualifier::Value;
        return type;
    }

    static bool is_receiver_borrow_type(const IrType& type) {
        return type.qualifier == TypeQualifier::Ref || type.qualifier == TypeQualifier::MutRef;
    }

    static bool same_receiver_base_type(const IrType& left, const IrType& right) {
        return same_type(value_qualified_type(left), value_qualified_type(right));
    }

    static bool can_weaken_mut_receiver_to_shared(const IrType& expected, const IrType& actual) {
        return expected.qualifier == TypeQualifier::Ref &&
               actual.qualifier == TypeQualifier::MutRef &&
               same_receiver_base_type(expected, actual);
    }

    static void weaken_mut_receiver_to_shared_if_needed(IrExpr& receiver, const IrType& expected) {
        if (!can_weaken_mut_receiver_to_shared(expected, receiver.type)) return;
        receiver.type = expected;
        receiver.mutable_borrow = false;
    }

    static bool borrowed_receiver_matches_value(const IrType& expected,
                                                const IrType& receiver_type,
                                                bool& mutable_borrow) {
        if (!is_receiver_borrow_type(expected)) return false;
        if (!same_receiver_base_type(expected, receiver_type)) return false;
        mutable_borrow = expected.qualifier == TypeQualifier::MutRef;
        return true;
    }

    static bool has_name(const std::vector<std::string>& names, const std::string& name) {
        return std::find(names.begin(), names.end(), name) != names.end();
    }

    static bool is_impl_coherence_generic_var(const IrType& type, const std::vector<std::string>& names) {
        return type.primitive == IrPrimitiveKind::Unknown && type.args.empty() && has_name(names, type.name);
    }

    static bool contains_impl_coherence_generic_var(
        const IrType& type,
        const std::vector<std::string>& left_names,
        const std::vector<std::string>& right_names
    ) {
        if (is_impl_coherence_generic_var(type, left_names) ||
            is_impl_coherence_generic_var(type, right_names)) {
            return true;
        }
        for (const auto& arg : type.args) {
            if (contains_impl_coherence_generic_var(arg, left_names, right_names)) return true;
        }
        for (const auto& field_type : type.field_types) {
            if (contains_impl_coherence_generic_var(field_type, left_names, right_names)) return true;
        }
        return false;
    }

    static bool bind_impl_coherence_generic_var(
        const std::string& key,
        const IrType& concrete,
        const std::vector<std::string>& left_names,
        const std::vector<std::string>& right_names,
        std::map<std::string, IrType>& bindings
    ) {
        if (contains_impl_coherence_generic_var(concrete, left_names, right_names)) return true;
        auto found = bindings.find(key);
        if (found == bindings.end()) {
            bindings.emplace(key, concrete);
            return true;
        }
        return same_type(found->second, concrete);
    }

    static bool trait_impl_type_patterns_overlap(
        const IrType& left,
        const std::vector<std::string>& left_names,
        const IrType& right,
        const std::vector<std::string>& right_names,
        std::map<std::string, IrType>& bindings
    ) {
        if (is_impl_coherence_generic_var(left, left_names)) {
            return bind_impl_coherence_generic_var("left:" + left.name, right, left_names, right_names, bindings);
        }
        if (is_impl_coherence_generic_var(right, right_names)) {
            return bind_impl_coherence_generic_var("right:" + right.name, left, left_names, right_names, bindings);
        }

        if (left.qualifier != right.qualifier ||
            left.primitive != right.primitive ||
            left.name != right.name ||
            left.array_size != right.array_size ||
            left.args.size() != right.args.size()) {
            return false;
        }

        for (std::size_t i = 0; i < left.args.size(); ++i) {
            if (!trait_impl_type_patterns_overlap(left.args[i], left_names, right.args[i], right_names, bindings)) {
                return false;
            }
        }
        return true;
    }

    static bool trait_impl_patterns_overlap(const TraitImplCoherenceInfo& left, const TraitImplCoherenceInfo& right) {
        if (left.trait_name != right.trait_name) return false;
        if (left.trait_args.size() != right.trait_args.size()) return false;

        std::map<std::string, IrType> bindings;
        for (std::size_t i = 0; i < left.trait_args.size(); ++i) {
            if (!trait_impl_type_patterns_overlap(
                    left.trait_args[i], left.generic_names, right.trait_args[i], right.generic_names, bindings)) {
                return false;
            }
        }
        return trait_impl_type_patterns_overlap(left.self_type, left.generic_names, right.self_type, right.generic_names, bindings);
    }

    static std::string trait_impl_coherence_kind(const TraitImplCoherenceInfo& impl) {
        return impl.generic_names.empty() ? "impl" : "generic impl";
    }

    std::map<std::string, IrType> generic_placeholder_substitutions(const std::vector<GenericParam>& generics) const {
        std::map<std::string, IrType> substitutions;
        for (const auto& generic : generics) {
            IrType placeholder;
            placeholder.qualifier = TypeQualifier::Value;
            placeholder.primitive = IrPrimitiveKind::Unknown;
            placeholder.name = generic.name;
            placeholder.loc = generic.loc;
            substitutions.emplace(generic.name, placeholder);
        }
        return substitutions;
    }

    GenericTraitBound resolve_generic_trait_bound(const GenericParam& generic) {
        const TypeRef& constraint = generic.constraint;
        if (constraint.qualifier != TypeQualifier::Value) {
            fail(constraint.loc, "trait bounds cannot use ownership qualifiers");
        }

        std::string trait_name = resolve_trait_name(constraint.name);
        auto found = traits_.find(trait_name);
        if (found == traits_.end()) {
            fail(constraint.loc, "unknown trait bound '" + constraint.name + "'");
        }
        const TraitInfo& trait = found->second;
        require_trait_access(constraint.loc, trait);
        if (constraint.args.size() != trait.generic_arity) {
            fail(constraint.loc,
                 "trait '" + trait.name + "' expects " + std::to_string(trait.generic_arity) +
                     " type argument" + (trait.generic_arity == 1 ? "" : "s"));
        }

        GenericTraitBound bound;
        bound.generic_name = generic.name;
        bound.trait_name = trait.name;
        bound.loc = constraint.loc;
        bound.trait_args.reserve(constraint.args.size());
        for (const auto& arg : constraint.args) {
            bound.trait_args.push_back(resolve_executable_type(arg));
        }
        return bound;
    }

    std::vector<GenericTraitBound> resolve_generic_trait_bounds(const std::vector<GenericParam>& generics) {
        std::vector<GenericTraitBound> bounds;
        for (const auto& generic : generics) {
            if (generic.has_constraint) bounds.push_back(resolve_generic_trait_bound(generic));
        }
        return bounds;
    }

    GenericTraitBound substitute_trait_bound(
        const GenericTraitBound& bound,
        const std::map<std::string, IrType>& substitutions
    ) const {
        GenericTraitBound resolved = bound;
        for (auto& arg : resolved.trait_args) {
            arg = substitute_inferred_type(arg, substitutions);
        }
        return resolved;
    }

    std::map<std::string, IrType> trait_application_substitutions(
        const TraitInfo& trait,
        const std::vector<IrType>& trait_args
    ) const {
        std::map<std::string, IrType> substitutions;
        for (std::size_t i = 0; i < trait.generic_names.size() && i < trait_args.size(); ++i) {
            substitutions.emplace(trait.generic_names[i], trait_args[i]);
        }
        return substitutions;
    }

    std::vector<GenericTraitBound> instantiated_supertrait_bounds(
        const TraitInfo& trait,
        const std::vector<IrType>& trait_args
    ) const {
        std::vector<GenericTraitBound> bounds;
        auto found = trait_supertraits_.find(trait.name);
        if (found == trait_supertraits_.end()) return bounds;

        std::map<std::string, IrType> substitutions = trait_application_substitutions(trait, trait_args);
        bounds.reserve(found->second.size());
        for (const auto& bound : found->second) {
            bounds.push_back(substitute_trait_bound(bound, substitutions));
        }
        return bounds;
    }

    void collect_associated_projection_targets(
        SourceLocation loc,
        const TraitInfo& trait,
        const std::vector<IrType>& trait_args,
        const std::string& associated_name,
        std::vector<GenericTraitBound>& targets,
        std::set<std::string>& visiting
    ) const {
        std::string key =
            trait_application_display(trait.name, trait_args) + "::" + associated_name;
        if (!visiting.insert(key).second) return;

        if (trait.associated_types.count(associated_name)) {
            GenericTraitBound target;
            target.trait_name = trait.name;
            target.trait_args = trait_args;
            target.loc = loc;
            targets.push_back(std::move(target));
            visiting.erase(key);
            return;
        }

        for (const auto& supertrait : instantiated_supertrait_bounds(trait, trait_args)) {
            auto super_found = traits_.find(supertrait.trait_name);
            if (super_found == traits_.end()) continue;
            collect_associated_projection_targets(
                loc,
                super_found->second,
                supertrait.trait_args,
                associated_name,
                targets,
                visiting);
        }

        visiting.erase(key);
    }

    std::vector<GenericTraitBound> associated_projection_targets(
        SourceLocation loc,
        const TraitInfo& trait,
        const std::vector<IrType>& trait_args,
        const std::string& associated_name
    ) const {
        std::vector<GenericTraitBound> targets;
        std::set<std::string> visiting;
        collect_associated_projection_targets(
            loc,
            trait,
            trait_args,
            associated_name,
            targets,
            visiting);

        std::vector<GenericTraitBound> unique_targets;
        std::set<std::string> seen;
        for (auto& target : targets) {
            std::string key = trait_application_display(target.trait_name, target.trait_args);
            if (!seen.insert(key).second) continue;
            unique_targets.push_back(std::move(target));
        }
        return unique_targets;
    }

    bool trait_application_implies_trait(
        const std::string& source_trait_name,
        const std::vector<IrType>& source_trait_args,
        const std::string& target_trait_name,
        const std::vector<IrType>& target_trait_args,
        std::set<std::string>& visiting
    ) const {
        if (source_trait_name == target_trait_name &&
            same_type_list(source_trait_args, target_trait_args)) {
            return true;
        }

        std::string key =
            trait_application_display(source_trait_name, source_trait_args) +
            "=>" +
            trait_application_display(target_trait_name, target_trait_args);
        if (!visiting.insert(key).second) return false;

        auto trait_found = traits_.find(source_trait_name);
        if (trait_found == traits_.end()) {
            visiting.erase(key);
            return false;
        }

        for (const auto& supertrait : instantiated_supertrait_bounds(trait_found->second, source_trait_args)) {
            if (trait_application_implies_trait(
                    supertrait.trait_name,
                    supertrait.trait_args,
                    target_trait_name,
                    target_trait_args,
                    visiting)) {
                visiting.erase(key);
                return true;
            }
        }

        visiting.erase(key);
        return false;
    }

    bool trait_application_implies_trait(
        const std::string& source_trait_name,
        const std::vector<IrType>& source_trait_args,
        const std::string& target_trait_name,
        const std::vector<IrType>& target_trait_args
    ) const {
        std::set<std::string> visiting;
        return trait_application_implies_trait(
            source_trait_name,
            source_trait_args,
            target_trait_name,
            target_trait_args,
            visiting);
    }

    bool trait_application_has_method(
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const std::string& method_name,
        std::set<std::string>& visiting
    ) const {
        auto trait_found = traits_.find(trait_name);
        if (trait_found == traits_.end()) return false;
        if (trait_found->second.methods.count(method_name)) return true;

        std::string key = trait_application_display(trait_name, trait_args) + "::" + method_name;
        if (!visiting.insert(key).second) return false;
        for (const auto& supertrait : instantiated_supertrait_bounds(trait_found->second, trait_args)) {
            if (trait_application_has_method(
                    supertrait.trait_name,
                    supertrait.trait_args,
                    method_name,
                    visiting)) {
                visiting.erase(key);
                return true;
            }
        }
        visiting.erase(key);
        return false;
    }

    bool trait_application_has_method(
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const std::string& method_name
    ) const {
        std::set<std::string> visiting;
        return trait_application_has_method(trait_name, trait_args, method_name, visiting);
    }

    const TraitInfo::Method* find_trait_application_method(
        SourceLocation loc,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const std::string& method_name,
        std::set<std::string>& visiting
    ) const {
        auto trait_found = traits_.find(trait_name);
        if (trait_found == traits_.end()) return nullptr;
        auto method_found = trait_found->second.methods.find(method_name);
        if (method_found != trait_found->second.methods.end()) return &method_found->second;

        std::string key = trait_application_display(trait_name, trait_args) + "::" + method_name;
        if (!visiting.insert(key).second) return nullptr;

        const TraitInfo::Method* selected = nullptr;
        for (const auto& supertrait : instantiated_supertrait_bounds(trait_found->second, trait_args)) {
            const TraitInfo::Method* candidate = find_trait_application_method(
                loc,
                supertrait.trait_name,
                supertrait.trait_args,
                method_name,
                visiting);
            if (!candidate) continue;
            if (selected) continue;
            selected = candidate;
        }

        visiting.erase(key);
        return selected;
    }

    const TraitInfo::Method* find_trait_application_method(
        SourceLocation loc,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const std::string& method_name
    ) const {
        std::set<std::string> visiting;
        return find_trait_application_method(loc, trait_name, trait_args, method_name, visiting);
    }

    void collect_trait_object_methods_from_trait(
        const TraitInfo& trait,
        const std::vector<IrType>& trait_args,
        std::vector<TraitObjectMethodEntry>& methods,
        std::set<std::string>& seen
    ) const {
        for (const auto& item : trait.methods) {
            std::string key =
                trait_method_display(trait.name, trait_args, item.second.name);
            if (!seen.insert(key).second) continue;
            methods.push_back(TraitObjectMethodEntry{&trait, trait_args, &item.second});
        }

        for (const auto& supertrait : instantiated_supertrait_bounds(trait, trait_args)) {
            auto super_found = traits_.find(supertrait.trait_name);
            if (super_found == traits_.end()) continue;
            collect_trait_object_methods_from_trait(
                super_found->second,
                supertrait.trait_args,
                methods,
                seen);
        }
    }

    std::vector<TraitObjectMethodEntry> collect_trait_object_methods(
        const TraitInfo& trait,
        const std::vector<IrType>& trait_args
    ) const {
        std::vector<TraitObjectMethodEntry> methods;
        std::set<std::string> seen;
        collect_trait_object_methods_from_trait(trait, trait_args, methods, seen);
        return methods;
    }

    const TraitObjectMethodEntry* find_trait_object_method_entry(
        SourceLocation loc,
        const std::vector<TraitObjectMethodEntry>& methods,
        const std::string& method_name,
        const IrType& object_type
    ) const {
        const TraitObjectMethodEntry* selected = nullptr;
        for (const auto& entry : methods) {
            if (!entry.method || entry.method->name != method_name) continue;
            if (selected) {
                fail(loc,
                     "trait object method '" + method_name + "' for " +
                         type_name(object_type) + " is ambiguous across supertraits");
            }
            selected = &entry;
        }
        return selected;
    }

    static bool same_trait_object_method_entry(
        const TraitObjectMethodEntry& left,
        const TraitObjectMethodEntry& right
    ) {
        if (!left.trait || !right.trait || !left.method || !right.method) return false;
        return left.trait->name == right.trait->name &&
               same_type_list(left.trait_args, right.trait_args) &&
               left.method->name == right.method->name;
    }

    std::uint64_t trait_object_upcast_vtable_offset(
        SourceLocation loc,
        const IrType& source,
        const IrType& target
    ) const {
        auto source_trait_found = traits_.find(source.name);
        auto target_trait_found = traits_.find(target.name);
        if (source_trait_found == traits_.end() || target_trait_found == traits_.end()) {
            fail(loc, "unknown trait in dyn upcast from " + type_name(source) + " to " + type_name(target));
        }

        std::vector<TraitObjectMethodEntry> source_methods =
            collect_trait_object_methods(source_trait_found->second, source.args);
        std::vector<TraitObjectMethodEntry> target_methods =
            collect_trait_object_methods(target_trait_found->second, target.args);
        if (target_methods.empty()) return 0;

        for (const auto& entry : target_methods) {
            require_trait_object_method_object_safe(loc, *entry.method);
        }

        if (target_methods.size() <= source_methods.size()) {
            for (std::size_t base = 0; base + target_methods.size() <= source_methods.size(); ++base) {
                bool matches = true;
                for (std::size_t i = 0; i < target_methods.size(); ++i) {
                    if (!same_trait_object_method_entry(source_methods[base + i], target_methods[i])) {
                        matches = false;
                        break;
                    }
                }
                if (matches) return static_cast<std::uint64_t>(base);
            }
        }

        fail(loc, "trait object upcast from " + type_name(source) + " to " +
                      type_name(target) + " cannot locate a compatible vtable layout");
    }

    bool try_resolve_trait_qualified_call_target(
        SourceLocation loc,
        const std::string& receiver_name,
        const std::vector<TypeRef>& type_args,
        std::string& trait_name,
        std::vector<IrType>& trait_args
    ) {
        std::string resolved = resolve_trait_name(receiver_name);
        auto trait_found = traits_.find(resolved);
        if (trait_found == traits_.end()) return false;

        const TraitInfo& trait = trait_found->second;
        require_trait_access(loc, trait);
        if (type_args.size() != trait.generic_arity) {
            fail(loc,
                 "trait '" + trait.name + "' expects " +
                     std::to_string(trait.generic_arity) +
                     " type argument" + (trait.generic_arity == 1 ? "" : "s"));
        }

        trait_name = trait.name;
        trait_args.clear();
        trait_args.reserve(type_args.size());
        for (const auto& type_arg : type_args) {
            trait_args.push_back(resolve_executable_type(type_arg));
        }
        return true;
    }

    GenericTraitBound resolve_generic_trait_bound_with_context(
        const GenericParam& generic,
        const std::string& module_name,
        std::map<std::string, IrType> substitutions
    ) {
        std::string previous_module = current_module_name_;
        std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
        current_module_name_ = module_name;
        current_type_substitutions_ = std::move(substitutions);
        GenericTraitBound bound = resolve_generic_trait_bound(generic);
        current_type_substitutions_ = std::move(previous_substitutions);
        current_module_name_ = previous_module;
        return bound;
    }

    void validate_generic_constraints_for(
        const std::vector<GenericParam>& generics,
        const std::string& module_name,
        std::map<std::string, IrType> substitutions
    ) {
        if (generics.empty()) return;
        bool has_constraint = false;
        for (const auto& generic : generics) {
            if (generic.has_constraint) {
                has_constraint = true;
                break;
            }
        }
        if (!has_constraint) return;

        std::string previous_module = current_module_name_;
        std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
        current_module_name_ = module_name;
        current_type_substitutions_ = std::move(substitutions);
        for (const auto& generic : generics) {
            if (generic.has_constraint) (void)resolve_generic_trait_bound(generic);
        }
        current_type_substitutions_ = std::move(previous_substitutions);
        current_module_name_ = previous_module;
    }

    void validate_generic_constraints_for(
        const std::vector<GenericParam>& generics,
        const std::string& module_name
    ) {
        validate_generic_constraints_for(generics, module_name, generic_placeholder_substitutions(generics));
    }

    void validate_generic_constraints() {
        for_each_struct_decl([&](const StructDecl& decl) {
            validate_generic_constraints_for(decl.generics, decl.module_name);
        });
        for_each_enum_decl([&](const EnumDecl& decl) {
            validate_generic_constraints_for(decl.generics, decl.module_name);
        });
        for_each_trait_decl([&](const TraitDecl& decl) {
            validate_generic_constraints_for(decl.generics, decl.module_name);
            std::map<std::string, IrType> trait_substitutions = generic_placeholder_substitutions(decl.generics);
            for (const auto& method : decl.methods) {
                std::map<std::string, IrType> method_substitutions = trait_substitutions;
                for (const auto& item : generic_placeholder_substitutions(method.generics)) {
                    method_substitutions.emplace(item.first, item.second);
                }
                validate_generic_constraints_for(method.generics, decl.module_name, std::move(method_substitutions));
            }
        });
        for_each_function_decl([&](const FunctionDecl& fn) {
            validate_generic_constraints_for(fn.generics, fn.module_name);
        });
        for_each_impl_decl([&](const ImplDecl& impl) {
            validate_generic_constraints_for(impl.generics, impl.module_name);
        });
    }

    IrType resolve_impl_method_type(const TypeRef& type, const std::map<std::string, IrType>& substitutions) {
        return resolve_type_with_substitutions(type, substitutions);
    }

    static std::string no_trait_bound_display() {
        return "none";
    }

    static bool same_trait_bound(const GenericTraitBound& left, const GenericTraitBound& right) {
        return left.trait_name == right.trait_name && same_type_list(left.trait_args, right.trait_args);
    }

    void validate_trait_method_generic_bound(
        const std::string& method_name,
        std::size_t index,
        const GenericParam& expected,
        const GenericParam& actual,
        const std::map<std::string, IrType>& expected_substitutions,
        const std::map<std::string, IrType>& actual_substitutions,
        const TraitInfo& trait,
        const ImplDecl& impl
    ) {
        if (expected.has_constraint != actual.has_constraint) {
            std::string expected_display = expected.has_constraint
                ? type_ref_key(expected.constraint)
                : no_trait_bound_display();
            std::string actual_display = actual.has_constraint
                ? type_ref_key(actual.constraint)
                : no_trait_bound_display();
            fail(actual.loc,
                 "method '" + method_name + "' generic parameter " + std::to_string(index + 1) +
                     " bound mismatch: expected " + expected_display + ", got " + actual_display);
        }
        if (!expected.has_constraint) return;

        GenericTraitBound expected_bound = resolve_generic_trait_bound_with_context(
            expected,
            trait.module_name,
            expected_substitutions);
        GenericTraitBound actual_bound = resolve_generic_trait_bound_with_context(
            actual,
            impl.module_name,
            actual_substitutions);
        if (!same_trait_bound(expected_bound, actual_bound)) {
            fail(actual.constraint.loc,
                 "method '" + method_name + "' generic parameter " + std::to_string(index + 1) +
                     " bound mismatch: expected " +
                     trait_application_display(expected_bound.trait_name, expected_bound.trait_args) +
                     ", got " +
                     trait_application_display(actual_bound.trait_name, actual_bound.trait_args));
        }
    }

    static bool is_into_iterator_result_contract(const TraitInfo& trait, const std::string& method_name) {
        return ari::is_into_iterator_result_contract(trait.name, method_name);
    }

    static bool is_iterator_next_receiver_contract(const TraitInfo& trait, const std::string& method_name) {
        return ari::is_iterator_next_receiver_contract(trait.name, method_name);
    }

    static bool is_into_iterator_receiver_contract(const TraitInfo& trait, const std::string& method_name) {
        return ari::is_into_iterator_receiver_contract(trait.name, method_name);
    }

    void validate_trait_impl_associated_types(
        const ImplDecl& impl,
        const TraitInfo& trait,
        const std::map<std::string, IrType>& actual_substitutions
    ) {
        std::map<std::string, const ImplDecl::AssociatedTypeWitness*> witnesses;
        for (const auto& witness : impl.associated_type_witnesses) {
            auto inserted = witnesses.emplace(witness.name, &witness);
            if (!inserted.second) fail(witness.loc, "duplicate associated type witness '" + witness.name + "' in impl");
            if (!trait.associated_types.count(witness.name)) {
                fail(witness.loc,
                     "associated type witness '" + witness.name + "' is not declared by trait '" + trait.name + "'");
            }
            (void)resolve_impl_method_type(witness.type, actual_substitutions);
        }
        for (const auto& item : trait.associated_types) {
            if (!witnesses.count(item.first)) {
                fail(impl.trait_type.loc,
                     "impl of trait '" + trait.name + "' for " + type_ref_key(impl.for_type) +
                         " is missing associated type '" + item.first + "'");
            }
        }
    }

    void validate_trait_impl_methods(
        const ImplDecl& impl,
        const TraitInfo& trait,
        const std::map<std::string, IrType>& expected_substitutions,
        const std::map<std::string, IrType>& actual_substitutions
    ) {
        std::map<std::string, const FunctionDecl*> impl_methods;
        std::set<std::string> impl_generic_names;
        for (const auto& item : actual_substitutions) impl_generic_names.insert(item.first);
        for (const auto& method : impl.methods) {
            if (method.is_variadic) fail(method.variadic_loc, "variadic parameters are only supported on extern \"C\" functions");
            std::string name = basename_of_qualified_name(method.name);
            require_unique_generic_params(method.generics, "impl method", name);
            for (const auto& generic : method.generics) {
                if (impl_generic_names.count(generic.name)) {
                    fail(generic.loc,
                         "impl method generic parameter '" + generic.name +
                             "' conflicts with an impl generic parameter");
                }
            }
            auto inserted = impl_methods.emplace(name, &method);
            if (!inserted.second) fail(method.loc, "duplicate method '" + name + "' in impl");
            if (!trait.methods.count(name)) {
                fail(method.loc, "method '" + name + "' is not declared by trait '" + trait.name + "'");
            }
        }

        for (const auto& item : trait.methods) {
            const TraitInfo::Method& expected_method = item.second;
            auto found = impl_methods.find(item.first);
            if (found == impl_methods.end()) {
                fail(impl.trait_type.loc,
                     "impl of trait '" + trait.name + "' for " + type_ref_key(impl.for_type) +
                         " is missing method '" + item.first + "'");
            }
            const FunctionDecl& actual_method = *found->second;
            bool actual_has_self_receiver = function_params_have_self_receiver(actual_method.params);
            if (actual_has_self_receiver != expected_method.has_self_receiver) {
                fail(actual_method.loc,
                     "method '" + item.first + "' receiver kind mismatch with trait declaration");
            }
            if (actual_method.generics.size() != expected_method.generics.size()) {
                fail(actual_method.loc,
                     "method '" + item.first + "' generic parameter count mismatch: trait expects " +
                         std::to_string(expected_method.generics.size()) + ", impl has " +
                         std::to_string(actual_method.generics.size()));
            }
            std::map<std::string, IrType> expected_method_substitutions = expected_substitutions;
            std::map<std::string, IrType> actual_method_substitutions = actual_substitutions;
            for (std::size_t i = 0; i < expected_method.generics.size(); ++i) {
                IrType placeholder;
                placeholder.qualifier = TypeQualifier::Value;
                placeholder.primitive = IrPrimitiveKind::Unknown;
                placeholder.name = "$method_generic" + std::to_string(i);
                placeholder.loc = actual_method.loc;
                expected_method_substitutions[expected_method.generics[i].name] = placeholder;
                actual_method_substitutions[actual_method.generics[i].name] = placeholder;
            }
            for (std::size_t i = 0; i < expected_method.generics.size(); ++i) {
                validate_trait_method_generic_bound(
                    item.first,
                    i,
                    expected_method.generics[i],
                    actual_method.generics[i],
                    expected_method_substitutions,
                    actual_method_substitutions,
                    trait,
                    impl);
            }
            if (actual_method.params.size() != expected_method.params.size()) {
                fail(actual_method.loc,
                     "method '" + item.first + "' parameter count mismatch: trait expects " +
                         std::to_string(expected_method.params.size()) + ", impl has " +
                         std::to_string(actual_method.params.size()));
            }
            for (std::size_t i = 0; i < expected_method.params.size(); ++i) {
                IrType expected = resolve_impl_method_type(expected_method.params[i], expected_method_substitutions);
                IrType actual = resolve_impl_method_type(actual_method.params[i].type, actual_method_substitutions);
                if (!same_type(expected, actual)) {
                    if (i == 0 &&
                        (is_iterator_next_receiver_contract(trait, item.first) ||
                         is_into_iterator_receiver_contract(trait, item.first)) &&
                        iterator_receiver_compatible(expected, actual)) {
                        continue;
                    }
                    fail(actual_method.params[i].type.loc,
                         "method '" + item.first + "' parameter " + std::to_string(i + 1) +
                             " type mismatch: expected " + type_name(expected) +
                             ", got " + type_name(actual));
                }
            }

            IrType expected_result = expected_method.has_result
                ? resolve_impl_method_type(expected_method.result, expected_method_substitutions)
                : void_type(expected_method.loc);
            IrType actual_result = actual_method.has_return_type
                ? resolve_impl_method_type(actual_method.return_type, actual_method_substitutions)
                : void_type(actual_method.loc);
            if (!same_type(expected_result, actual_result)) {
                if (is_into_iterator_result_contract(trait, item.first)) {
                    if (actual_result.primitive == IrPrimitiveKind::Void) {
                        fail(actual_method.loc, "method 'into_iter' must return an iterator value");
                    }
                    continue;
                }
                fail(actual_method.loc,
                     "method '" + item.first + "' return type mismatch: expected " +
                         type_name(expected_result) + ", got " + type_name(actual_result));
            }
        }
    }

    void validate_inherent_impl_methods(const ImplDecl& impl) const {
        std::set<std::string> names;
        std::set<std::string> impl_generic_names;
        for (const auto& generic : impl.generics) impl_generic_names.insert(generic.name);
        for (const auto& witness : impl.associated_type_witnesses) {
            fail(witness.loc, "associated type witnesses are only allowed in trait impls");
        }
        for (const auto& method : impl.methods) {
            if (method.is_variadic) fail(method.variadic_loc, "variadic parameters are only supported on extern \"C\" functions");
            std::string name = basename_of_qualified_name(method.name);
            if (!names.insert(name).second) fail(method.loc, "duplicate method '" + name + "' in impl");
            require_unique_generic_params(method.generics, "impl method", name);
            for (const auto& generic : method.generics) {
                if (impl_generic_names.count(generic.name)) {
                    fail(generic.loc,
                         "impl method generic parameter '" + generic.name +
                             "' conflicts with an impl generic parameter");
                }
            }
        }
    }

    void validate_impls() {
        for_each_impl_decl([&](const ImplDecl& impl) {
            require_unique_generic_params(impl.generics, "impl", "impl");
            if (!impl.has_trait) {
                validate_inherent_impl_methods(impl);
                return;
            }
            if (impl.trait_type.qualifier != TypeQualifier::Value) {
                fail(impl.trait_type.loc, "impl trait type cannot have an ownership qualifier");
            }

            std::string previous_module = current_module_name_;
            std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
            current_module_name_ = impl.module_name;
            std::map<std::string, IrType> impl_substitutions = generic_placeholder_substitutions(impl.generics);
            current_type_substitutions_ = impl_substitutions;
            std::vector<GenericTraitBound> impl_bounds = resolve_generic_trait_bounds(impl.generics);
            std::string trait_name = resolve_trait_name(impl.trait_type.name);
            auto found = traits_.find(trait_name);
            if (found == traits_.end()) {
                current_type_substitutions_ = std::move(previous_substitutions);
                current_module_name_ = previous_module;
                fail(impl.trait_type.loc, "unknown trait '" + impl.trait_type.name + "'");
            }
            const TraitInfo& trait = found->second;
            require_trait_access(impl.trait_type.loc, trait);
            if (impl.trait_type.args.size() != trait.generic_arity) {
                current_type_substitutions_ = std::move(previous_substitutions);
                current_module_name_ = previous_module;
                fail(impl.trait_type.loc,
                     "trait '" + trait.name + "' expects " + std::to_string(trait.generic_arity) +
                         " type argument" + (trait.generic_arity == 1 ? "" : "s"));
            }

            IrType self_type = resolve_executable_type(impl.for_type);
            std::map<std::string, IrType> expected_substitutions;
            expected_substitutions.emplace("Self", self_type);
            std::map<std::string, IrType> actual_substitutions = impl_substitutions;
            actual_substitutions.emplace("Self", self_type);
            std::vector<IrType> trait_args;
            for (std::size_t i = 0; i < trait.generic_names.size(); ++i) {
                IrType trait_arg = resolve_executable_type(impl.trait_type.args[i]);
                trait_args.push_back(trait_arg);
                expected_substitutions[trait.generic_names[i]] = std::move(trait_arg);
            }
            std::string key = impl.generics.empty()
                ? trait_impl_key(trait.name, trait_args, self_type)
                : ("generic " + type_ref_key(impl.trait_type) + " for " + type_ref_key(impl.for_type));
            if (!impl_keys_.insert(key).second) {
                current_type_substitutions_ = std::move(previous_substitutions);
                current_module_name_ = previous_module;
                fail(impl.trait_type.loc, "duplicate impl of trait '" + trait.name + "' for " + type_name(self_type));
            }

            TraitImplCoherenceInfo coherence;
            coherence.trait_name = trait.name;
            coherence.trait_args = trait_args;
            coherence.self_type = self_type;
            for (const auto& generic : impl.generics) coherence.generic_names.push_back(generic.name);
            coherence.loc = impl.trait_type.loc;
            for (const auto& previous : trait_impl_coherence_) {
                if (!trait_impl_patterns_overlap(coherence, previous)) continue;
                current_type_substitutions_ = std::move(previous_substitutions);
                current_module_name_ = previous_module;
                fail(impl.trait_type.loc,
                     trait_impl_coherence_kind(coherence) + " of trait '" +
                         trait_application_display(coherence.trait_name, coherence.trait_args) +
                         "' for " + type_name(coherence.self_type) + " overlaps " +
                         trait_impl_coherence_kind(previous) + " of trait '" +
                         trait_application_display(previous.trait_name, previous.trait_args) +
                         "' for " + type_name(previous.self_type));
            }

            validate_trait_impl_associated_types(impl, trait, actual_substitutions);
            validate_trait_impl_methods(impl, trait, expected_substitutions, actual_substitutions);
            trait_impl_coherence_.push_back(std::move(coherence));
            if (!impl.generics.empty()) {
                GenericTraitImplInfo generic_impl;
                generic_impl.trait_name = trait.name;
                generic_impl.trait_args = trait_args;
                generic_impl.self_type = self_type;
                for (const auto& generic : impl.generics) generic_impl.generic_names.push_back(generic.name);
                generic_impl.generic_bounds = impl_bounds;
                generic_impl.module_name = impl.module_name;
                generic_impl.loc = impl.trait_type.loc;
                generic_trait_impls_.push_back(std::move(generic_impl));
            }
            current_type_substitutions_ = std::move(previous_substitutions);
            current_module_name_ = previous_module;
        });
    }

    void validate_supertrait_impls() {
        for_each_impl_decl([&](const ImplDecl& impl) {
            if (!impl.has_trait) return;

            std::string previous_module = current_module_name_;
            std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
            current_module_name_ = impl.module_name;
            std::map<std::string, IrType> substitutions = generic_placeholder_substitutions(impl.generics);
            current_type_substitutions_ = substitutions;

            std::string trait_name = resolve_trait_name(impl.trait_type.name);
            auto trait_found = traits_.find(trait_name);
            if (trait_found == traits_.end()) {
                current_type_substitutions_ = std::move(previous_substitutions);
                current_module_name_ = previous_module;
                return;
            }
            const TraitInfo& trait = trait_found->second;
            IrType self_type = resolve_executable_type(impl.for_type);
            std::vector<IrType> trait_args;
            trait_args.reserve(trait.generic_names.size());
            for (std::size_t i = 0; i < trait.generic_names.size(); ++i) {
                trait_args.push_back(resolve_executable_type(impl.trait_type.args[i]));
            }

            for (const auto& supertrait : instantiated_supertrait_bounds(trait, trait_args)) {
                if (!type_implements_trait(supertrait.trait_name, supertrait.trait_args, self_type)) {
                    current_type_substitutions_ = std::move(previous_substitutions);
                    current_module_name_ = previous_module;
                    fail(impl.trait_type.loc,
                         "impl of trait '" + trait.name + "' for " + type_name(self_type) +
                             " requires supertrait '" +
                             trait_application_display(supertrait.trait_name, supertrait.trait_args) +
                             "'");
                }
            }

            current_type_substitutions_ = std::move(previous_substitutions);
            current_module_name_ = previous_module;
        });
    }

    const FunctionDecl& require_impl_method_decl(const ImplDecl& impl, const std::string& name) const {
        for (const auto& method : impl.methods) {
            if (basename_of_qualified_name(method.name) == name) return method;
        }
        throw CompileError("internal error: missing validated impl method '" + name + "'");
    }

    void validate_into_iterator_result_contracts() {
        for_each_impl_decl([&](const ImplDecl& impl) {
            if (!impl.has_trait) return;

            std::string previous_module = current_module_name_;
            std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
            current_module_name_ = impl.module_name;
            std::map<std::string, IrType> substitutions = generic_placeholder_substitutions(impl.generics);
            current_type_substitutions_ = substitutions;
            IrType self_type = resolve_executable_type(impl.for_type);
            substitutions.emplace("Self", self_type);
            current_type_substitutions_ = substitutions;

            std::string trait_name = resolve_trait_name(impl.trait_type.name);
            if (!is_std_into_iterator_trait_name(trait_name)) {
                current_type_substitutions_ = std::move(previous_substitutions);
                current_module_name_ = previous_module;
                return;
            }
            IrType item_type = resolve_executable_type(impl.trait_type.args[0]);
            item_type.qualifier = TypeQualifier::Value;
            const FunctionDecl& method = require_impl_method_decl(impl, "into_iter");
            IrType result_type = method.has_return_type
                ? resolve_type_with_substitutions(method.return_type, substitutions)
                : void_type(method.loc);
            if (result_type.primitive == IrPrimitiveKind::Void) {
                fail(method.loc, "method 'into_iter' must return an iterator value");
            }

            ForIteratorTraitMatch iterator_match;
            if (!try_find_for_iterator_trait_kind(result_type, ForIteratorTraitKind::Iterator, iterator_match)) {
                fail(method.has_return_type ? method.return_type.loc : method.loc,
                     "method 'into_iter' for " + for_iterator_trait_display(trait_name, item_type) +
                         " must return a type that implements Iterator[" + type_name(item_type) +
                         "], got " + type_name(result_type));
            }
            if (!same_type(iterator_match.item_type, item_type)) {
                fail(method.has_return_type ? method.return_type.loc : method.loc,
                     "method 'into_iter' for " + for_iterator_trait_display(trait_name, item_type) +
                         " returns Iterator[" + type_name(iterator_match.item_type) +
                         "] instead of Iterator[" + type_name(item_type) + "]");
            }

            current_type_substitutions_ = std::move(previous_substitutions);
            current_module_name_ = previous_module;
        });
    }

    std::optional<BorrowReturnOperandPathHint> borrow_return_operand_path_hint(
        const Expr& expr,
        const std::string& param_name,
        const IrType& type
    ) {
        if (expr.kind == ExprKind::Name) {
            if (expr.name != param_name) return std::nullopt;
            return BorrowReturnOperandPathHint{"", type};
        }

        if (expr.kind == ExprKind::FieldAccess) {
            std::optional<BorrowReturnOperandPathHint> base =
                borrow_return_operand_path_hint(*expr_operand(expr), param_name, type);
            if (!base || base->type.primitive != IrPrimitiveKind::Struct) return std::nullopt;
            std::size_t index = struct_field_index(expr.loc, base->type, expr.name);
            return BorrowReturnOperandPathHint{
                local_owned_field_path(base->path, index),
                base->type.field_types[index]
            };
        }

        if (expr.kind == ExprKind::TupleIndex) {
            std::optional<BorrowReturnOperandPathHint> base =
                borrow_return_operand_path_hint(*expr_operand(expr), param_name, type);
            if (!base ||
                (base->type.primitive != IrPrimitiveKind::Tuple &&
                 base->type.primitive != IrPrimitiveKind::Struct)) {
                return std::nullopt;
            }
            const std::vector<IrType>& fields = aggregate_field_types(base->type);
            if (expr.tuple_index >= fields.size()) return std::nullopt;
            return BorrowReturnOperandPathHint{
                local_owned_field_path(base->path, static_cast<std::size_t>(expr.tuple_index)),
                fields[static_cast<std::size_t>(expr.tuple_index)]
            };
        }

        if (expr.kind == ExprKind::Index) {
            std::optional<BorrowReturnOperandPathHint> base =
                borrow_return_operand_path_hint(*expr_operand(expr), param_name, type);
            if (!base ||
                (base->type.primitive != IrPrimitiveKind::Array &&
                 base->type.primitive != IrPrimitiveKind::Vector) ||
                base->type.args.size() != 1) {
                return std::nullopt;
            }
            const ExprPtr& index = expr_right(expr);
            if (!index || index->kind != ExprKind::Integer || index->int_negative) return std::nullopt;
            if (index->int_value >= base->type.array_size) return std::nullopt;
            return BorrowReturnOperandPathHint{
                local_owned_field_path(base->path, static_cast<std::size_t>(index->int_value)),
                base->type.args[0]
            };
        }

        return std::nullopt;
    }

    std::optional<std::string> borrow_return_expr_path_hint(const Expr& expr,
                                                            const std::string& param_name,
                                                            const IrType& param_type) {
        if (expr.kind == ExprKind::Name) {
            if (expr.name == param_name) return "";
            return std::nullopt;
        }
        if (expr.kind == ExprKind::Borrow && expr_operand(expr)) {
            std::optional<BorrowReturnOperandPathHint> hint =
                borrow_return_operand_path_hint(*expr_operand(expr), param_name, param_type);
            if (hint) return hint->path;
            return std::nullopt;
        }
        if (expr.kind == ExprKind::If) {
            const ExprPtr& then_value = expr_if_then_value(expr);
            const ExprPtr& else_value = expr_if_else_value(expr);
            if (!then_value || !else_value) return std::nullopt;
            std::optional<std::string> then_path =
                borrow_return_expr_path_hint(*then_value, param_name, param_type);
            std::optional<std::string> else_path =
                borrow_return_expr_path_hint(*else_value, param_name, param_type);
            if (then_path && else_path && *then_path == *else_path) return then_path;
            return std::nullopt;
        }
        if (expr.kind == ExprKind::Match) {
            std::optional<std::string> path;
            bool saw_arm = false;
            for (const auto& arm : expr_match_arms(expr)) {
                if (!arm.value) return std::nullopt;
                std::optional<std::string> arm_path =
                    borrow_return_expr_path_hint(*arm.value, param_name, param_type);
                if (!arm_path) return std::nullopt;
                if (!path) path = *arm_path;
                else if (*path != *arm_path) return std::nullopt;
                saw_arm = true;
            }
            if (saw_arm) return path;
            return std::nullopt;
        }
        if (expr.kind == ExprKind::Block && expr_block_body(expr).empty() && expr_block_value(expr)) {
            return borrow_return_expr_path_hint(*expr_block_value(expr), param_name, param_type);
        }
        return std::nullopt;
    }

    void merge_borrow_return_path_hint(BorrowReturnPathHintScan& scan,
                                       std::optional<std::string> path) {
        if (!path) {
            scan.unknown = true;
            return;
        }
        if (!scan.path) {
            scan.path = std::move(path);
            return;
        }
        if (*scan.path != *path) scan.unknown = true;
    }

    void collect_borrow_return_path_hints(const std::vector<StmtPtr>& statements,
                                          const std::string& param_name,
                                          const IrType& param_type,
                                          BorrowReturnPathHintScan& scan) {
        for (const auto& stmt : statements) {
            switch (stmt->kind) {
                case StmtKind::Return:
                    if (stmt->expr) {
                        merge_borrow_return_path_hint(
                            scan,
                            borrow_return_expr_path_hint(*stmt->expr, param_name, param_type));
                    }
                    break;
                case StmtKind::Block:
                    collect_borrow_return_path_hints(stmt_statements(*stmt), param_name, param_type, scan);
                    break;
                case StmtKind::If:
                    collect_borrow_return_path_hints(stmt_then_body(*stmt), param_name, param_type, scan);
                    collect_borrow_return_path_hints(stmt_else_body(*stmt), param_name, param_type, scan);
                    break;
                case StmtKind::While:
                case StmtKind::WhileLet:
                case StmtKind::For:
                    collect_borrow_return_path_hints(stmt_loop_body(*stmt), param_name, param_type, scan);
                    break;
                case StmtKind::InitWhile:
                    collect_borrow_return_path_hints(stmt_loop_body(*stmt), param_name, param_type, scan);
                    break;
                case StmtKind::Match:
                    for (const auto& arm : stmt_match_arms(*stmt)) {
                        collect_borrow_return_path_hints(arm.body, param_name, param_type, scan);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    std::string resolve_explicit_borrow_return_path(
        const ExplicitBorrowReturnContract& contract,
        const IrType& source_type
    ) {
        std::string path;
        IrType current = value_qualified_type(source_type);
        for (const auto& component : contract.path) {
            if (component.kind == BorrowReturnPathComponent::Kind::Field) {
                if (current.primitive != IrPrimitiveKind::Struct) {
                    fail(component.loc,
                         "attribute '@borrow_return' field path requires a struct source before '" +
                             component.text + "'");
                }
                std::size_t index = struct_field_index(component.loc, current, component.text);
                path = local_owned_field_path(path, index);
                current = current.field_types[index];
                continue;
            }

            if (current.primitive == IrPrimitiveKind::Tuple ||
                current.primitive == IrPrimitiveKind::Struct ||
                current.primitive == IrPrimitiveKind::Array) {
                const std::vector<IrType>& fields = aggregate_field_types(current);
                if (component.index >= fields.size()) {
                    fail(component.loc,
                         "attribute '@borrow_return' index " +
                             std::to_string(component.index) +
                             " is out of range for source path");
                }
                path = local_owned_field_path(path, static_cast<std::size_t>(component.index));
                current = fields[static_cast<std::size_t>(component.index)];
                continue;
            }

            if (current.primitive == IrPrimitiveKind::Vector && current.args.size() == 1) {
                path = local_owned_field_path(path, static_cast<std::size_t>(component.index));
                current = current.args[0];
                continue;
            }

            fail(component.loc,
                 "attribute '@borrow_return' indexed path requires an aggregate source");
        }
        return path;
    }

    void apply_explicit_borrow_return_contract(FunctionSig& sig, const FunctionDecl& fn) {
        std::optional<ExplicitBorrowReturnContract> contract =
            explicit_borrow_return_contract(fn.attributes);
        if (!contract) return;
        if (!is_borrow_type(sig.result)) {
            fail(contract->loc, "attribute '@borrow_return' requires a borrow return type");
        }

        std::optional<std::size_t> source_index;
        for (std::size_t i = 0; i < fn.params.size(); ++i) {
            if (fn.params[i].name != contract->param_name) continue;
            source_index = i;
            break;
        }
        if (!source_index || *source_index >= sig.params.size()) {
            fail(contract->loc,
                 "attribute '@borrow_return' references unknown parameter '" +
                     contract->param_name + "'");
        }
        if (!is_borrow_type(sig.params[*source_index])) {
            fail(contract->loc,
                 "attribute '@borrow_return' source parameter '" +
                     contract->param_name + "' must have ref or ref mut type");
        }

        sig.borrow_return_param_index = source_index;
        sig.borrow_return_path =
            resolve_explicit_borrow_return_path(*contract, sig.params[*source_index]);
        sig.borrow_return_contract_explicit = true;
    }

    void set_function_borrow_return_path_hint(FunctionSig& sig, const FunctionDecl& fn) {
        if (sig.borrow_return_contract_explicit) return;
        if (!sig.borrow_return_param_index || !fn.has_body) return;
        if (*sig.borrow_return_param_index >= fn.params.size() ||
            *sig.borrow_return_param_index >= sig.params.size()) {
            return;
        }
        const Param& param = fn.params[*sig.borrow_return_param_index];
        if (param.name.empty()) return;
        BorrowReturnPathHintScan scan;
        collect_borrow_return_path_hints(
            fn.body,
            param.name,
            sig.params[*sig.borrow_return_param_index],
            scan);
        if (!scan.unknown && scan.path) sig.borrow_return_path = *scan.path;
    }

    void collect_impl_method_signatures() {
        for_each_impl_decl([&](const ImplDecl& impl) {
            std::string previous_module = current_module_name_;
            current_module_name_ = impl.module_name;

            std::map<std::string, IrType> substitutions = generic_placeholder_substitutions(impl.generics);
            std::map<std::string, IrType> outer_previous_substitutions = std::move(current_type_substitutions_);
            current_type_substitutions_ = substitutions;
            IrType self_type = resolve_executable_type(impl.for_type);
            require_root_vector_runtime_abi(impl.for_type.loc, self_type, "an impl receiver");
            substitutions.emplace("Self", self_type);
            current_type_substitutions_ = substitutions;
            std::vector<GenericTraitBound> impl_bounds = resolve_generic_trait_bounds(impl.generics);

            std::string trait_name;
            std::vector<IrType> trait_args;
            if (impl.has_trait) {
                trait_name = resolve_trait_name(impl.trait_type.name);
                auto trait_found = traits_.find(trait_name);
                if (trait_found == traits_.end()) {
                    current_module_name_ = previous_module;
                    fail(impl.trait_type.loc, "unknown trait '" + impl.trait_type.name + "'");
                }
                const TraitInfo& trait = trait_found->second;
                for (std::size_t i = 0; i < trait.generic_names.size(); ++i) {
                    IrType trait_arg = resolve_executable_type(impl.trait_type.args[i]);
                    trait_args.push_back(trait_arg);
                    substitutions.emplace(trait.generic_names[i], std::move(trait_arg));
                }
            }
            current_type_substitutions_ = std::move(outer_previous_substitutions);

            std::set<std::string> local_method_names;
            for (const auto& method : impl.methods) {
                if (!method.has_body) {
                    current_module_name_ = previous_module;
                    fail(method.loc, "impl method '" + basename_of_qualified_name(method.name) + "' must have a body");
                }
                std::string method_name = basename_of_qualified_name(method.name);
                if (!local_method_names.insert(method_name).second) {
                    current_module_name_ = previous_module;
                    fail(method.loc, "duplicate method '" + method_name + "' in impl");
                }

                std::map<std::string, IrType> method_substitutions = substitutions;
                for (const auto& item : generic_placeholder_substitutions(method.generics)) {
                    method_substitutions.emplace(item.first, item.second);
                }

                std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
                current_type_substitutions_ = method_substitutions;
                std::vector<GenericTraitBound> method_bounds = resolve_generic_trait_bounds(method.generics);

                FunctionSig sig;
                sig.loc = method.loc;
                sig.module_name = method.module_name;
                sig.is_public = false;
                for (const auto& param : method.params) {
                    IrType param_type = resolve_executable_type(param.type);
                    bool vec_view = false;
                    sig.params.push_back(function_parameter_abi_type(
                        param.type.loc,
                        param_type,
                        "an impl method parameter",
                        vec_view));
                }
                sig.result = method.has_return_type ? resolve_executable_type(method.return_type) : void_type(method.loc);
                if (method.has_return_type) {
                    require_root_vector_runtime_abi(method.return_type.loc, sig.result, "an impl method return type");
                }
                set_function_return_contracts(sig);
                apply_explicit_borrow_return_contract(sig, method);
                set_function_borrow_return_path_hint(sig, method);

                current_type_substitutions_ = std::move(previous_substitutions);

                ImplMethodInfo info;
                info.fn = &method;
                info.trait_name = trait_name;
                info.trait_args = trait_args;
                info.receiver_type = self_type;
                for (const auto& generic : impl.generics) info.generic_names.push_back(generic.name);
                for (const auto& generic : method.generics) info.method_generic_names.push_back(generic.name);
                info.generic_bounds = impl_bounds;
                info.method_generic_bounds = method_bounds;
                info.sig = sig;
                info.substitutions = substitutions;
                info.module_name = impl.module_name;
                info.is_public = impl.is_public || method.is_public;
                info.loc = method.loc;
                info.lowered_name = impl_method_lowered_name(self_type, trait_name, method_name);
                if ((trait_name == "Drop" || trait_name == "std::Drop") && method_name == "drop") {
                    drop_impls_[drop_impl_key(self_type)] = info;
                }

                bool has_self_receiver = function_params_have_self_receiver(method.params);

                if (!impl.generics.empty() || !method.generics.empty()) {
                    if (!has_self_receiver) {
                        generic_associated_impls_.push_back(std::move(info));
                    } else {
                        generic_method_impls_.push_back(std::move(info));
                    }
                } else {
                    auto inserted = functions_.emplace(info.lowered_name, info.sig);
                    if (!inserted.second) {
                        current_module_name_ = previous_module;
                        fail(method.loc, "duplicate lowered impl method '" + method_name + "' for " + type_name(self_type));
                    }

                    if (has_self_receiver) {
                        method_impls_[method_lookup_key(self_type, method_name)].push_back(std::move(info));
                    } else {
                        associated_impls_[method_lookup_key(self_type, method_name)].push_back(std::move(info));
                    }
                }
            }

            current_module_name_ = previous_module;
        });
    }

    std::map<std::string, IrType> enum_type_arg_substitutions(
        SourceLocation loc,
        const EnumInfo& info,
        const std::vector<IrType>& type_args
    ) const {
        if (type_args.size() != info.generic_arity) {
            fail(loc,
                 "enum '" + info.name + "' expects " + std::to_string(info.generic_arity) +
                     " type argument" + (info.generic_arity == 1 ? "" : "s"));
        }

        std::map<std::string, IrType> substitutions;
        for (std::size_t i = 0; i < type_args.size(); ++i) {
            substitutions.emplace(info.generic_names[i], type_args[i]);
        }
        return substitutions;
    }

    std::vector<IrType> resolve_enum_payload_refs(
        const EnumInfo& info,
        const std::vector<TypeRef>& payload_refs,
        const std::map<std::string, IrType>& substitutions
    ) {
        std::map<std::string, IrType> previous_substitutions = current_type_substitutions_;
        std::string previous_module = current_module_name_;

        for (const auto& item : substitutions) {
            current_type_substitutions_[item.first] = item.second;
        }
        current_module_name_ = info.module_name;

        std::vector<IrType> payloads;
        payloads.reserve(payload_refs.size());
        for (const auto& payload : payload_refs) {
            payloads.push_back(resolve_executable_type(payload));
        }

        current_module_name_ = previous_module;
        current_type_substitutions_ = std::move(previous_substitutions);
        return payloads;
    }

    void note_enum_payload_layout_requirement(
        SourceLocation loc,
        const IrType& payload_type,
        std::size_t payload_count,
        bool& aggregate_layout
    ) const {
        bool unresolved_generic_payload =
            payload_type.qualifier == TypeQualifier::Value &&
            payload_type.primitive == IrPrimitiveKind::Unknown &&
            payload_type.args.empty();
        bool payload_needs_aggregate =
            !is_legacy_enum_payload_type(payload_type) || payload_count > 1;
        if (payload_needs_aggregate) aggregate_layout = true;
        if (payload_needs_aggregate &&
            !unresolved_generic_payload &&
            !is_aggregate_enum_payload_type(payload_type)) {
            fail(loc,
                 "enum aggregate payloads currently support integer, bool, pointer-shaped, one-word enum, or homogeneous nested aggregate enum values, got " +
                     type_name(payload_type));
        }
        if (!payload_needs_aggregate && !is_legacy_enum_payload_type(payload_type)) {
            fail(loc, "enum payload must fit the 32-bit tagged-union payload slot, got " + type_name(payload_type));
        }
    }

    IrType resolve_enum_type_application(
        SourceLocation loc,
        const EnumInfo& info,
        const std::vector<IrType>& type_args
    ) {
        std::map<std::string, IrType> substitutions =
            enum_type_arg_substitutions(loc, info, type_args);

        IrType type;
        type.qualifier = TypeQualifier::Value;
        type.primitive = IrPrimitiveKind::Enum;
        type.name = info.name;
        type.loc = loc;
        type.args = type_args;

        bool aggregate_layout = false;
        std::size_t max_payloads = 0;
        std::vector<IrType> payload_slot_types;
        std::vector<bool> payload_slot_set;
        for (const auto& item : info.cases) {
            if (item.payloads.size() > 1) aggregate_layout = true;
            max_payloads = std::max(max_payloads, item.payloads.size());
            if (payload_slot_types.size() < item.payloads.size()) {
                payload_slot_types.resize(item.payloads.size());
                payload_slot_set.resize(item.payloads.size(), false);
            }
            std::vector<IrType> payloads = resolve_enum_payload_refs(info, item.payloads, substitutions);
            for (std::size_t i = 0; i < payloads.size(); ++i) {
                note_enum_payload_layout_requirement(item.payloads[i].loc, payloads[i], item.payloads.size(), aggregate_layout);
                IrType slot_type = enum_payload_slot_storage_type(item.payloads[i].loc, payloads[i]);
                if (!payload_slot_set[i]) {
                    payload_slot_types[i] = std::move(slot_type);
                    payload_slot_set[i] = true;
                } else if (!same_type(payload_slot_types[i], slot_type)) {
                    fail(item.payloads[i].loc,
                         "enum aggregate payload slot " + std::to_string(i) +
                             " mixes storage types " + type_name(payload_slot_types[i]) +
                             " and " + type_name(slot_type) +
                             "; mixed nested aggregate enum payload slots need an explicit ABI rule");
                }
            }
        }

        if (aggregate_layout) {
            type.field_types.push_back(enum_tag_storage_type(loc));
            type.field_names.push_back("$tag");
            type.field_mutable.push_back(false);
            for (std::size_t i = 0; i < max_payloads; ++i) {
                type.field_types.push_back(payload_slot_set[i] ? payload_slot_types[i] : enum_payload_storage_type(loc));
                type.field_names.push_back("$payload" + std::to_string(i));
                type.field_mutable.push_back(false);
            }
        }
        return type;
    }

    EnumCaseInfo specialize_enum_case_info(
        SourceLocation loc,
        const EnumCaseInfo& info,
        const std::vector<IrType>& type_args
    ) {
        auto enum_found = enums_.find(info.enum_name);
        if (enum_found == enums_.end()) fail(loc, "unknown enum '" + info.enum_name + "'");
        const EnumInfo& enum_info = enum_found->second;
        std::map<std::string, IrType> substitutions =
            enum_type_arg_substitutions(loc, enum_info, type_args);

        EnumCaseInfo specialized = info;
        specialized.enum_type = resolve_enum_type_application(loc, enum_info, type_args);
        specialized.payloads = resolve_enum_payload_refs(enum_info, info.payload_refs, substitutions);
        specialized.generic_names.clear();
        specialized.is_generic = false;
        return specialized;
    }

    static IrType generic_enum_payload_placeholder(const TypeRef& payload) {
        IrType type;
        type.qualifier = TypeQualifier::Value;
        type.primitive = IrPrimitiveKind::Unknown;
        type.name = type_ref_key(payload);
        type.loc = payload.loc;
        return type;
    }

    void collect_enum_layouts() {
        for_each_enum_decl([&](const EnumDecl& decl) {
            require_unique_generic_params(decl.generics, "enum", decl.name);
            if (structs_.count(decl.name)) {
                fail(decl.loc, "enum '" + decl.name + "' conflicts with struct of the same name");
            }
            if (decl.cases.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
                fail(decl.loc, "enum '" + decl.name + "' has too many cases");
            }

            IrType type;
            type.qualifier = TypeQualifier::Value;
            type.primitive = IrPrimitiveKind::Enum;
            type.name = decl.name;
            type.loc = decl.loc;

            EnumInfo info;
            info.name = decl.name;
            info.module_name = decl.module_name;
            info.type = type;
            info.loc = decl.loc;
            info.generic_arity = decl.generics.size();
            info.is_public = decl.is_public;
            info.deprecated = deprecated_attribute(decl.attributes) != nullptr;
            info.deprecated_message = deprecated_message(decl.attributes);
            for (const auto& generic : decl.generics) info.generic_names.push_back(generic.name);

            for (std::size_t i = 0; i < decl.cases.size(); ++i) {
                const EnumCase& item = decl.cases[i];
                EnumInfo::Case case_info;
                case_info.name = item.name;
                case_info.qualified_name = qualify_in_module(decl.module_name, item.name);
                case_info.tag = static_cast<std::uint32_t>(i);
                case_info.payloads = item.payloads;
                case_info.loc = item.loc;
                info.case_names.push_back(item.name);
                info.cases.push_back(std::move(case_info));
            }

            auto inserted = enums_.emplace(decl.name, std::move(info));
            if (!inserted.second) fail(decl.loc, "duplicate enum '" + decl.name + "'");
        });

        for (auto& [enum_name, enum_info] : enums_) {
            if (enum_info.generic_arity == 0) {
                enum_info.type = resolve_enum_type_application(enum_info.loc, enum_info, {});
            }

            for (const auto& item : enum_info.cases) {
                EnumCaseInfo info;
                info.enum_name = enum_name;
                info.enum_type = enum_info.type;
                info.name = item.qualified_name;
                info.module_name = enum_info.module_name;
                info.tag = item.tag;
                info.payload_refs = item.payloads;
                info.generic_names = enum_info.generic_names;
                info.is_generic = enum_info.generic_arity != 0;
                info.loc = item.loc;

                if (info.is_generic) {
                    for (const auto& payload : item.payloads) {
                        info.payloads.push_back(generic_enum_payload_placeholder(payload));
                    }
                } else {
                    info = specialize_enum_case_info(item.loc, info, {});
                }

                std::string case_key = info.name;
                auto inserted = enum_cases_.emplace(case_key, std::move(info));
                if (!inserted.second) {
                    fail(item.loc, "duplicate enum case constructor '" + case_key + "'");
                }
            }
        }
    }

    void collect_function_signatures() {
        for_each_function_decl([&](const FunctionDecl& fn) {
            require_unique_generic_params(fn.generics, fn.meta ? "meta function" : "function", fn.name);
            if (fn.params.size() > std::numeric_limits<std::uint16_t>::max()) {
                fail(fn.loc, "functions support up to 65535 parameters");
            }
            if (fn.is_variadic && !fn.is_extern) {
                fail(fn.variadic_loc, "variadic parameters are only supported on extern \"C\" functions");
            }
            if (fn.is_extern) {
                collect_extern_function_signature(fn);
                return;
            }
            (void)exported_link_name(fn);
            if (!fn.generics.empty()) {
                auto inserted = generic_functions_.emplace(fn.name, &fn);
                if (!inserted.second) fail(fn.loc, "duplicate generic function '" + fn.name + "'");
                return;
            }
            if (!is_executable_function(fn)) return;
            if (enum_cases_.count(fn.name)) fail(fn.loc, "function '" + fn.name + "' conflicts with enum case constructor");
            if (is_format_print_name(fn.name)) fail(fn.loc, "function '" + fn.name + "' conflicts with prelude print builtin");
            if (functions_.count(fn.name)) fail(fn.loc, "duplicate executable function '" + fn.name + "'");

            FunctionSig sig;
            sig.loc = fn.loc;
            sig.module_name = fn.module_name;
            sig.is_public = fn.is_public;
            sig.deprecated = deprecated_attribute(fn.attributes) != nullptr;
            sig.deprecated_message = deprecated_message(fn.attributes);
            sig.link_name = exported_link_name(fn);
            if (!sig.link_name.empty()) {
                auto exported = exported_symbols_.emplace(sig.link_name, fn.name);
                if (!exported.second) {
                    fail(fn.loc,
                         "exported symbol '" + sig.link_name + "' is already used by function '" +
                             exported.first->second + "'");
                }
            }
            register_emitted_function_symbol(fn.loc,
                                             sig.link_name.empty() ? mangle_function_name(fn.name) : sig.link_name,
                                             fn.name);
            std::string previous_module = current_module_name_;
            current_module_name_ = fn.module_name;
            for (const auto& param : fn.params) {
                IrType param_type = resolve_executable_type(param.type);
                bool vec_view = false;
                sig.params.push_back(function_parameter_abi_type(
                    param.type.loc,
                    param_type,
                    "a function parameter",
                    vec_view));
            }
            sig.result = fn.has_return_type ? resolve_executable_type(fn.return_type) : void_type(fn.loc);
            if (fn.has_return_type) {
                require_root_vector_runtime_abi(fn.return_type.loc, sig.result, "a function return type");
            }
            set_function_return_contracts(sig);
            apply_explicit_borrow_return_contract(sig, fn);
            set_function_borrow_return_path_hint(sig, fn);
            current_module_name_ = previous_module;

            auto inserted = functions_.emplace(fn.name, std::move(sig));
            if (!inserted.second) fail(fn.loc, "duplicate executable function '" + fn.name + "'");
        });
    }

    void require_function_signature_root_vector_runtime_abi(const FunctionDecl& fn,
                                                           const std::vector<IrType>& param_types,
                                                           const IrType& result) const {
        for (std::size_t i = 0; i < param_types.size(); ++i) {
            require_root_vector_runtime_abi(fn.params[i].type.loc, param_types[i], "a function parameter");
        }
        if (fn.has_return_type) {
            require_root_vector_runtime_abi(fn.return_type.loc, result, "a function return type");
        }
    }

    static bool is_unsized_vector_storage_type(const IrType& type) {
        return is_vector_storage_type(type) && type.array_size == 0;
    }

    IrType function_parameter_abi_type(SourceLocation loc,
                                       const IrType& source,
                                       const std::string& context,
                                       bool& vec_view) const {
        vec_view = false;
        if (is_unsized_vector_storage_type(source)) {
            if (source.args.size() != 1) {
                fail(loc, "Vec parameter ABI requires exactly one element type");
            }
            require_slice_element_materializable(loc, source.args[0], "Vec parameter ABI");
            vec_view = true;
            return make_prelude_slice_type(loc, source.args[0]);
        }
        require_root_vector_runtime_abi(loc, source, context);
        return source;
    }

    void collect_extern_function_signature(const FunctionDecl& fn) {
        if (fn.params.size() > std::numeric_limits<std::uint16_t>::max()) {
            fail(fn.loc, "functions support up to 65535 parameters");
        }
        bool is_c_abi = fn.extern_abi == "C";
        bool is_ari_abi = fn.extern_abi == "ari";
        if (fn.meta) fail(fn.loc, "extern functions cannot be meta functions");
        if (!fn.generics.empty()) {
            if (is_ari_abi) {
                fail(fn.loc, "extern \"ari\" builtin declarations cannot be generic");
            }
            fail(fn.loc, "extern C declarations cannot be generic; declare concrete C symbols or wrap generic foreign APIs in C");
        }
        if (fn.has_body) fail(fn.loc, "extern functions cannot have a body");
        if (fn.is_variadic && !is_c_abi) {
            fail(fn.variadic_loc, "variadic parameters are only supported on extern \"C\" functions");
        }
        if (fn.is_variadic && fn.params.empty()) {
            fail(fn.variadic_loc, "C variadic extern functions require at least one fixed parameter");
        }
        for (const auto& param : fn.params) {
            if (param.has_pattern) {
                fail(param.pattern.loc, "extern function parameters cannot use patterns");
            }
        }
        if (enum_cases_.count(fn.name)) fail(fn.loc, "extern function '" + fn.name + "' conflicts with enum case constructor");
        if (is_format_print_name(fn.name)) fail(fn.loc, "extern function '" + fn.name + "' conflicts with prelude print builtin");
        if (functions_.count(fn.name)) fail(fn.loc, "duplicate function '" + fn.name + "'");
        (void)exported_link_name(fn);
        if (is_ari_abi) {
            if (fn.extern_link_name.empty()) {
                fail(fn.loc, "extern \"ari\" builtin declarations require an explicit builtin symbol");
            }
            if (!is_ari_builtin_symbol(fn.extern_link_name)) {
                fail(fn.loc, "unknown Ari builtin symbol '" + fn.extern_link_name + "'");
            }
        } else if (!fn.extern_link_name.empty() && !is_c_symbol_name(fn.extern_link_name)) {
            fail(fn.loc, "invalid external link symbol '" + fn.extern_link_name + "'");
        }

        FunctionSig sig;
        sig.loc = fn.loc;
        sig.module_name = fn.module_name;
        sig.is_public = fn.is_public;
        sig.is_extern = true;
        sig.is_variadic = fn.is_variadic;
        sig.extern_abi = fn.extern_abi;
        sig.link_name = fn.extern_link_name;
        sig.deprecated = deprecated_attribute(fn.attributes) != nullptr;
        sig.deprecated_message = deprecated_message(fn.attributes);
        std::string previous_module = current_module_name_;
        current_module_name_ = fn.module_name;
        for (const auto& param : fn.params) {
            IrType param_type = resolve_executable_type(param.type);
            require_root_vector_runtime_abi(param.type.loc, param_type, "an extern function parameter");
            if (param_type.qualifier == TypeQualifier::Value && param_type.primitive == IrPrimitiveKind::Void) {
                fail(param.type.loc, "extern parameter cannot have void type; use ptr c_void for void*");
            }
            sig.params.push_back(param_type);
        }
        sig.result = fn.has_return_type ? resolve_executable_type(fn.return_type) : void_type(fn.loc);
        if (fn.has_return_type) {
            require_root_vector_runtime_abi(fn.return_type.loc, sig.result, "an extern function return type");
        }
        set_function_return_contracts(sig);
        apply_explicit_borrow_return_contract(sig, fn);
        current_module_name_ = previous_module;

        auto inserted = functions_.emplace(fn.name, std::move(sig));
        if (!inserted.second) fail(fn.loc, "duplicate function '" + fn.name + "'");
    }

    void collect_ir_extern_functions(IrProgram& ir) const {
        for (const auto& item : functions_) {
            const FunctionSig& sig = item.second;
            if (!sig.is_extern) continue;
            IrExternFunction fn;
            fn.name = item.first;
            fn.link_name = sig.link_name;
            fn.abi = ir_extern_abi_from_source(sig.extern_abi);
            fn.return_type = sig.result;
            fn.is_variadic = sig.is_variadic;
            fn.loc = sig.loc;
            fn.params.reserve(sig.params.size());
            for (std::size_t i = 0; i < sig.params.size(); ++i) {
                fn.params.push_back(IrParam{"arg" + std::to_string(i), sig.params[i]});
            }
            ir.extern_functions.push_back(std::move(fn));
        }
    }

    void collect_ir_c_records(IrProgram& ir) {
        for_each_struct_decl([&](const StructDecl& decl) {
            if (!decl.is_public) return;
            if (!find_attribute(decl.attributes, "repr")) return;

            IrCRecord record;
            record.name = decl.name;
            record.c_name = unqualified_name(decl.name);
            record.loc = decl.loc;
            record.opaque = !decl.generics.empty();
            if (record.opaque) {
                ir.c_records.push_back(std::move(record));
                return;
            }
            for (const auto& field : decl.fields) {
                record.fields.push_back(IrCRecordField{
                    field.name,
                    resolve_executable_type(field.type),
                    field.loc
                });
            }
            ir.c_records.push_back(std::move(record));
        });
    }

    void collect_ir_c_enums(IrProgram& ir) const {
        for_each_enum_decl([&](const EnumDecl& decl) {
            if (!decl.is_public) return;
            if (!find_attribute(decl.attributes, "repr")) return;

            IrCEnum item;
            item.name = decl.name;
            item.c_name = unqualified_name(decl.name);
            item.loc = decl.loc;
            for (std::size_t i = 0; i < decl.cases.size(); ++i) {
                const EnumCase& enum_case = decl.cases[i];
                item.cases.push_back(IrCEnumCase{
                    enum_case.name,
                    item.c_name + "_" + enum_case.name,
                    static_cast<std::uint32_t>(i),
                    enum_case.loc
                });
            }
            ir.c_enums.push_back(std::move(item));
        });
    }

    std::vector<const FunctionDecl*> collect_test_functions() const {
        std::vector<const FunctionDecl*> tests;
        for_each_function_decl([&](const FunctionDecl& fn) {
            if (!find_attribute(fn.attributes, "test")) return;
            if (fn.is_extern) fail(fn.loc, "@test functions cannot be extern");
            if (fn.meta) fail(fn.loc, "@test functions cannot be meta functions");
            if (!fn.generics.empty()) fail(fn.loc, "@test functions cannot be generic");
            if (!fn.has_body) fail(fn.loc, "@test functions must have a body");
            if (fn.name == "main") fail(fn.loc, "@test function cannot be named main");
            if (!fn.params.empty()) fail(fn.loc, "@test functions cannot take parameters");
            auto found = functions_.find(fn.name);
            if (found == functions_.end()) fail(fn.loc, "internal error: missing @test function signature for '" + fn.name + "'");
            const IrType& result = found->second.result;
            if (result.primitive != IrPrimitiveKind::Void && result.primitive != IrPrimitiveKind::I64) {
                fail(fn.loc, "@test functions must return i64 or void");
            }
            tests.push_back(&fn);
        });
        return tests;
    }

    IrFunction make_test_main(const std::vector<const FunctionDecl*>& test_functions) const {
        SourceLocation loc{1, 1};
        IrFunction fn;
        fn.name = "main";
        fn.return_type = i64_type(loc);
        fn.loc = loc;

        for (const FunctionDecl* test : test_functions) {
            auto sig = functions_.find(test->name);
            if (sig == functions_.end()) throw CompileError("internal error: missing @test function '" + test->name + "'");

            auto stmt = std::make_unique<IrStmt>();
            stmt->kind = IrStmtKind::ExprStmt;
            stmt->loc = test->loc;
            stmt->expr = make_ir_call_expr(test->loc, test->name, sig->second.result);
            fn.body.push_back(std::move(stmt));
        }

        auto ret = std::make_unique<IrStmt>();
        ret->kind = IrStmtKind::Return;
        ret->loc = loc;
        ret->expr = make_integer_zero(loc, fn.return_type);
        fn.body.push_back(std::move(ret));

        return fn;
    }

    void require_main() const {
        auto found = functions_.find("main");
        if (found == functions_.end()) throw CompileError("missing executable function 'main'");
        if (!found->second.params.empty()) fail(found->second.loc, "main cannot take parameters");
        if (found->second.result.qualifier != TypeQualifier::Value ||
            found->second.result.primitive != IrPrimitiveKind::I64) {
            fail(found->second.loc, "main must return i64 in the executable subset");
        }
    }

    static IrType primitive_type(IrPrimitiveKind primitive, std::string name, SourceLocation loc) {
        IrType type;
        type.primitive = primitive;
        type.name = std::move(name);
        type.loc = loc;
        return type;
    }

    static IrType void_type(SourceLocation loc) {
        return primitive_type(IrPrimitiveKind::Void, "void", loc);
    }

    static IrType null_pointer_type(SourceLocation loc) {
        IrType type = void_type(loc);
        type.qualifier = TypeQualifier::Ptr;
        return type;
    }

    static IrType function_pointer_type(const FunctionSig& sig, SourceLocation loc) {
        IrType type = primitive_type(IrPrimitiveKind::Function, "fn", loc);
        type.array_size = sig.params.size();
        type.args = sig.params;
        type.args.push_back(sig.result);
        return type;
    }

    static IrType function_pointer_result_type(const IrType& type) {
        if (type.primitive != IrPrimitiveKind::Function ||
            type.args.empty() ||
            type.array_size + 1 != type.args.size()) {
            throw CompileError(where(type.loc) + ": malformed function pointer type");
        }
        return type.args[static_cast<std::size_t>(type.array_size)];
    }

    static IrType integer_type(IrPrimitiveKind primitive, SourceLocation loc) {
        return primitive_type(primitive, primitive_name(primitive), loc);
    }

    static IrType i64_type(SourceLocation loc) {
        return integer_type(IrPrimitiveKind::I64, loc);
    }

    static IrType bool_type(SourceLocation loc) {
        return primitive_type(IrPrimitiveKind::Bool, "bool", loc);
    }

    static IrType array_storage_type(SourceLocation loc, const IrType& element, std::uint64_t length) {
        IrType type = primitive_type(IrPrimitiveKind::Array, "Array", loc);
        type.args.push_back(element);
        type.array_size = length;
        type.field_types.reserve(static_cast<std::size_t>(length));
        type.field_mutable.reserve(static_cast<std::size_t>(length));
        for (std::uint64_t i = 0; i < length; ++i) {
            type.field_types.push_back(element);
            type.field_mutable.push_back(false);
        }
        return type;
    }

    static void reject_type_args(const TypeRef& ast_type) {
        if (!ast_type.args.empty()) {
            fail(ast_type.loc, "unsupported executable type '" + ast_type.name + "'");
        }
    }

    IrType resolve_nullable_type(const TypeRef& ast_type) {
        if (ast_type.qualifier != TypeQualifier::Value) {
            fail(ast_type.loc, "nullable type suffix ? cannot be combined with own, ref, or ptr qualifiers");
        }

        TypeRef base = ast_type;
        base.nullable = false;
        IrType type = resolve_executable_type(base);
        if (type.qualifier != TypeQualifier::Value) {
            fail(ast_type.loc, "nullable type suffix ? expects a plain value type");
        }

        type.qualifier = TypeQualifier::Ptr;
        type.loc = ast_type.loc;
        return type;
    }

    IrType finish_type_ref_wrapper_type(const TypeRef& ast_type, IrType type) {
        if (ast_type.nullable) {
            if (ast_type.qualifier != TypeQualifier::Value) {
                fail(ast_type.loc, "nullable type suffix ? cannot be combined with own, ref, or ptr qualifiers");
            }
            if (type.qualifier != TypeQualifier::Value) {
                fail(ast_type.loc, "nullable type suffix ? expects a plain value type");
            }
            type.qualifier = TypeQualifier::Ptr;
        } else if (ast_type.qualifier != TypeQualifier::Value) {
            type.qualifier = ast_type.qualifier;
        }
        type.loc = ast_type.loc;
        return type;
    }

    IrType resolve_type_macro_invocation(const TypeRef& ast_type) {
        const MetaFunctionInfo& meta =
            require_meta_invocation(ast_type.loc, MetaInvocationSite::TypeMacro, ast_type.name);
        TypeRef expanded = meta.type_return
                               ? expand_type_macro_constructor(ast_type, meta.parameter_name, *meta.type_return)
                               : expand_type_macro_invocation(ast_type);
        IrType type = resolve_executable_type(expanded);
        return finish_type_ref_wrapper_type(ast_type, std::move(type));
    }

    IrType finish_associated_projection_type(const TypeRef& ast_type, IrType type) {
        return finish_type_ref_wrapper_type(ast_type, std::move(type));
    }

    IrType resolve_associated_type_projection(const TypeRef& ast_type) {
        std::string trait_name = resolve_trait_name(ast_type.name);
        auto trait_found = traits_.find(trait_name);
        if (trait_found == traits_.end()) {
            fail(ast_type.loc, "unknown trait '" + ast_type.name + "' in associated type projection");
        }
        const TraitInfo& trait = trait_found->second;
        require_trait_access(ast_type.loc, trait);
        if (ast_type.args.size() != trait.generic_arity) {
            fail(ast_type.loc,
                 "trait '" + trait.name + "' expects " + std::to_string(trait.generic_arity) +
                     " type argument" + (trait.generic_arity == 1 ? "" : "s"));
        }
        std::vector<IrType> trait_args;
        trait_args.reserve(ast_type.args.size());
        for (const auto& arg : ast_type.args) trait_args.push_back(resolve_executable_type(arg));

        std::vector<GenericTraitBound> projection_targets =
            associated_projection_targets(ast_type.loc, trait, trait_args, ast_type.associated_projection);
        if (projection_targets.empty()) {
            fail(ast_type.loc,
                 "trait '" + trait.name + "' has no associated type '" + ast_type.associated_projection + "'");
        }
        if (projection_targets.size() > 1) {
            fail(ast_type.loc,
                 "associated type projection '" + type_ref_key(ast_type) +
                     "' is ambiguous across supertraits");
        }
        const GenericTraitBound& projection_target = projection_targets.front();

        std::vector<IrType> candidates;
        for_each_impl_decl([&](const ImplDecl& impl) {
            if (!impl.has_trait) return;

            std::string previous_module = current_module_name_;
            std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
            current_module_name_ = impl.module_name;
            std::map<std::string, IrType> impl_substitutions = generic_placeholder_substitutions(impl.generics);
            current_type_substitutions_ = impl_substitutions;

            bool matches = false;
            std::map<std::string, IrType> inferred;
            if (resolve_trait_name(impl.trait_type.name) == projection_target.trait_name &&
                impl.trait_type.args.size() == projection_target.trait_args.size()) {
                std::vector<IrType> impl_trait_args;
                impl_trait_args.reserve(impl.trait_type.args.size());
                for (const auto& arg : impl.trait_type.args) impl_trait_args.push_back(resolve_executable_type(arg));
                if (impl.generics.empty()) {
                    matches = same_type_list(impl_trait_args, projection_target.trait_args);
                } else {
                    matches = true;
                    std::vector<std::string> generic_names;
                    for (const auto& generic : impl.generics) generic_names.push_back(generic.name);
                    for (std::size_t i = 0; i < impl_trait_args.size(); ++i) {
                        if (!infer_generic_pattern_type(
                                impl_trait_args[i],
                                projection_target.trait_args[i],
                                generic_names,
                                inferred)) {
                            matches = false;
                            break;
                        }
                    }
                    for (const auto& generic_name : generic_names) {
                        if (!inferred.count(generic_name)) {
                            matches = false;
                            break;
                        }
                    }
                }
            }

            if (matches) {
                for (const auto& witness : impl.associated_type_witnesses) {
                    if (witness.name != ast_type.associated_projection) continue;
                    IrType witness_type = resolve_executable_type(witness.type);
                    if (!impl.generics.empty()) witness_type = substitute_inferred_type(witness_type, inferred);
                    candidates.push_back(std::move(witness_type));
                }
            }

            current_type_substitutions_ = std::move(previous_substitutions);
            current_module_name_ = previous_module;
        });

        if (candidates.empty()) {
            fail(ast_type.loc,
                 "associated type projection '" + type_ref_key(ast_type) +
                     "' has no matching impl witness");
        }
        if (candidates.size() > 1) {
            fail(ast_type.loc,
                 "associated type projection '" + type_ref_key(ast_type) +
                     "' is ambiguous across impl witnesses");
        }
        return finish_associated_projection_type(ast_type, std::move(candidates.front()));
    }

    IrType resolve_executable_type(const TypeRef& ast_type) {
        if (ast_type.is_macro_invocation) return resolve_type_macro_invocation(ast_type);
        if (ast_type.has_associated_projection) {
            return resolve_associated_type_projection(ast_type);
        }
        if (ast_type.nullable) return resolve_nullable_type(ast_type);

        auto substitution = current_type_substitutions_.find(ast_type.name);
        if (substitution != current_type_substitutions_.end() && ast_type.args.empty()) {
            IrType type = substitution->second;
            type.qualifier = ast_type.qualifier;
            type.loc = ast_type.loc;
            return type;
        }

        IrType type;
        type.qualifier = ast_type.qualifier;
        type.name = ast_type.name;
        type.loc = ast_type.loc;

        if (type.qualifier != TypeQualifier::Value &&
            type.qualifier != TypeQualifier::Own &&
            type.qualifier != TypeQualifier::Ref &&
            type.qualifier != TypeQualifier::MutRef &&
            type.qualifier != TypeQualifier::Ptr) {
            fail(type.loc, "only value, own, ref, ref mut, and ptr types are supported in the executable subset yet");
        }
        IrPrimitiveKind c_primitive = IrPrimitiveKind::Unknown;
        std::string c_canonical;
        if (ast_type.is_dyn_object) {
            if (type.qualifier != TypeQualifier::Value) {
                fail(type.loc, "trait object ownership qualifiers are planned; use dyn Trait as a value type for now");
            }

            std::string trait_name = resolve_trait_name(type.name);
            auto trait_found = traits_.find(trait_name);
            if (trait_found == traits_.end()) {
                fail(type.loc, "unknown trait '" + type.name + "' in trait object type");
            }
            const TraitInfo& trait = trait_found->second;
            require_trait_access(type.loc, trait);
            if (ast_type.args.size() != trait.generic_arity) {
                fail(type.loc,
                     "trait '" + trait.name + "' expects " + std::to_string(trait.generic_arity) +
                         " type argument" + (trait.generic_arity == 1 ? "" : "s"));
            }

            type.primitive = IrPrimitiveKind::TraitObject;
            type.name = trait.name;
            for (const auto& arg : ast_type.args) {
                type.args.push_back(resolve_executable_type(arg));
            }
        } else if (c_abi_type_alias(type.name, target_, c_primitive, c_canonical)) {
            reject_type_args(ast_type);
            type.primitive = c_primitive;
            type.name = c_canonical;
        } else if (type.name == "fn") {
            if (type.qualifier != TypeQualifier::Value) {
                fail(type.loc, "function pointer types do not support ownership qualifiers");
            }
            if (ast_type.args.empty() || ast_type.array_size + 1 != ast_type.args.size()) {
                fail(type.loc, "function pointer types use fn(param_types...) -> result_type");
            }
            type.primitive = IrPrimitiveKind::Function;
            type.name = "fn";
            type.array_size = ast_type.array_size;
            for (std::size_t i = 0; i < type.array_size; ++i) {
                IrType param_type = resolve_executable_type(ast_type.args[i]);
                bool vec_view = false;
                type.args.push_back(function_parameter_abi_type(
                    ast_type.args[i].loc,
                    param_type,
                    "a function pointer parameter",
                    vec_view));
                if (is_void_value_type(type.args[i])) {
                    fail(ast_type.args[i].loc, "function pointer parameter cannot have void type");
                }
            }
            type.args.push_back(resolve_executable_type(ast_type.args[type.array_size]));
            require_root_vector_runtime_abi(
                ast_type.args[type.array_size].loc,
                type.args[type.array_size],
                "a function pointer return type");
        } else if (type.name == "i8") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::I8;
        } else if (type.name == "i16") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::I16;
        } else if (type.name == "i32") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::I32;
        } else if (type.name == "i64" || type.name == "int") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::I64;
            type.name = "i64";
        } else if (type.name == "u8") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::U8;
        } else if (type.name == "u16") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::U16;
        } else if (type.name == "u32") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::U32;
        } else if (type.name == "u64") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::U64;
        } else if (type.name == "f32") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::F32;
        } else if (type.name == "f64" || type.name == "float") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::F64;
            type.name = "f64";
        } else if (type.name == "f128") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::F128;
        } else if (type.name == "bool") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::Bool;
        } else if (type.name == "string") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::String;
        } else if (type.name == "void") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::Void;
        } else if (type.name == "type") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::MetaType;
        } else if (type.name == "Zone" ||
                   type.name == "mem::Zone" ||
                   type.name == "std::Zone" ||
                   type.name == "prelude::Zone") {
            reject_type_args(ast_type);
            type.primitive = IrPrimitiveKind::Zone;
            type.name = "Zone";
        } else if (type.name == "Tuple") {
            if (ast_type.args.size() == 1) fail(type.loc, "single-element tuple types are not supported");
            type.primitive = IrPrimitiveKind::Tuple;
            for (const auto& arg : ast_type.args) type.args.push_back(resolve_executable_type(arg));
        } else if (type.name == "Array") {
            if (ast_type.args.size() != 1 || ast_type.array_size == 0) {
                fail(type.loc, "array types use [element_type, size]");
            }
            if (type.qualifier != TypeQualifier::Value &&
                type.qualifier != TypeQualifier::Ptr &&
                type.qualifier != TypeQualifier::Ref &&
                type.qualifier != TypeQualifier::MutRef) {
                fail(type.loc, "array ownership qualifiers are not supported in the executable subset yet");
            }
            type.primitive = IrPrimitiveKind::Array;
            type.name = "Array";
            type.array_size = ast_type.array_size;
            IrType element = resolve_executable_type(ast_type.args[0]);
            require_plain_prelude_aggregate_element(type.loc, element, "array");
            type.args.push_back(element);
            for (std::uint64_t i = 0; i < type.array_size; ++i) {
                type.field_types.push_back(element);
                type.field_mutable.push_back(false);
            }
        } else if (type.name == "Vec" || type.name == "prelude::Vec") {
            if (ast_type.args.size() != 1) fail(type.loc, "Vec requires exactly one element type");
            type.primitive = IrPrimitiveKind::Vector;
            type.name = "Vec";
            type.args.push_back(resolve_executable_type(ast_type.args[0]));
        } else if (is_prelude_range_type_name(type.name)) {
            if (ast_type.args.size() != 1) {
                fail(type.loc,
                     "prelude type '" + unqualified_name(type.name) + "' expects 1 type argument");
            }
            if (type.qualifier != TypeQualifier::Value) {
                fail(type.loc, "range ownership qualifiers are not supported in the executable subset yet");
            }
            IrType bound = resolve_executable_type(ast_type.args[0]);
            if (!is_value_integer_type(bound)) {
                fail(type.loc, "prelude range types require integer bounds");
            }
            type = make_prelude_range_type(type.loc, unqualified_name(type.name) == "RangeInclusive", bound);
        } else {
            std::string resolved_struct_name = resolve_struct_type_name(type.name);
            auto struct_found = structs_.find(resolved_struct_name);
            if (struct_found != structs_.end()) {
                const StructInfo& info = struct_found->second;
                require_struct_access(type.loc, info);
                if (info.deprecated) {
                    warn_deprecated_use(type.loc, "struct", info.name, info.deprecated_message);
                }
                if (ast_type.args.size() != info.generic_arity) {
                    fail(type.loc,
                         "struct '" + info.name + "' expects " + std::to_string(info.generic_arity) +
                             " type argument" + (info.generic_arity == 1 ? "" : "s"));
                }
                if (type.qualifier != TypeQualifier::Value &&
                    type.qualifier != TypeQualifier::Ptr &&
                    type.qualifier != TypeQualifier::Ref &&
                    type.qualifier != TypeQualifier::MutRef) {
                    fail(type.loc, "struct ownership qualifiers are not supported in the executable subset yet");
                }
                type.primitive = IrPrimitiveKind::Struct;
                type.name = info.name;

                std::map<std::string, IrType> substitutions;
                for (std::size_t i = 0; i < ast_type.args.size(); ++i) {
                    IrType arg = resolve_executable_type(ast_type.args[i]);
                    type.args.push_back(arg);
                    substitutions.emplace(info.generic_names[i], arg);
                }

                std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
                for (const auto& item : substitutions) current_type_substitutions_.emplace(item.first, item.second);
                for (const auto& field : info.fields) {
                    type.field_names.push_back(field.name);
                    type.field_types.push_back(resolve_executable_type(field.type));
                    type.field_mutable.push_back(field.mutable_field);
                }
                current_type_substitutions_ = std::move(previous_substitutions);
                require_root_vector_runtime_abi(type.loc, type, "a struct field");
            } else {
                std::string resolved_name = resolve_enum_type_name(type.name);
                auto enum_found = enums_.find(resolved_name);
                if (enum_found != enums_.end()) {
                    TypeQualifier requested_qualifier = type.qualifier;
                    if (requested_qualifier != TypeQualifier::Value &&
                        requested_qualifier != TypeQualifier::Ptr &&
                        requested_qualifier != TypeQualifier::Ref &&
                        requested_qualifier != TypeQualifier::MutRef) {
                        fail(type.loc, "enum ownership qualifiers are not supported in the executable subset yet");
                    }
                    require_module_path_access(type.loc, enum_found->second.module_name);
                    if (!can_access(enum_found->second.module_name, enum_found->second.is_public)) {
                        fail(type.loc, "enum '" + enum_found->second.name + "' is not public");
                    }
                    if (enum_found->second.deprecated) {
                        warn_deprecated_use(type.loc, "enum", enum_found->second.name, enum_found->second.deprecated_message);
                    }
                    std::vector<IrType> type_args;
                    type_args.reserve(ast_type.args.size());
                    for (const auto& arg : ast_type.args) {
                        type_args.push_back(resolve_executable_type(arg));
                    }
                    type = resolve_enum_type_application(ast_type.loc, enum_found->second, type_args);
                    type.qualifier = requested_qualifier;
                    type.loc = ast_type.loc;
                } else {
                    std::size_t planned_arity = 0;
                    if (planned_prelude_type_arity(type.name, planned_arity)) {
                        if (ast_type.args.size() != planned_arity) {
                            fail(type.loc,
                                 "prelude type '" + unqualified_name(type.name) + "' expects " +
                                     std::to_string(planned_arity) + " type argument" +
                                     (planned_arity == 1 ? "" : "s"));
                        }
                        fail(type.loc, planned_prelude_type_message(type.name));
                    }
                    reject_type_args(ast_type);
                    fail(type.loc, "unsupported executable type '" + type.name + "'");
                }
            }
        }

        if (type.primitive == IrPrimitiveKind::MetaType) {
            fail(type.loc, "type values are only supported in meta functions yet");
        }
        if (type.qualifier == TypeQualifier::Own && !is_owned_executable_primitive(type.primitive)) {
            fail(type.loc, "own is not supported for " + type_name(type) + " in the executable subset yet");
        }
        if ((type.qualifier == TypeQualifier::Ref || type.qualifier == TypeQualifier::MutRef) &&
            !is_borrowable_executable_primitive(type.primitive)) {
            fail(type.loc, "borrowed references are not supported for " + type_name(type) + " in the executable subset yet");
        }
        return type;
    }

    std::string generic_origin_from_type_ref(const TypeRef& type) const {
        if (type.is_dyn_object) return "";
        if (!type.args.empty()) return "";
        return current_type_substitutions_.count(type.name) ? type.name : "";
    }

    std::string generic_origin_from_expr(const Expr& expr) {
        if (expr.kind != ExprKind::Name) return "";
        LocalInfo* local = find_local_slot(expr.name);
        return local ? local->generic_origin : "";
    }

    IrFunction check_function(const FunctionDecl& fn) {
        return check_function_as(fn, fn.name, {});
    }

    IrFunction check_function_as(const FunctionDecl& fn, const std::string& lowered_name, std::map<std::string, IrType> substitutions) {
        std::string previous_module = current_module_name_;
        std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
        std::vector<GenericTraitBound> previous_generic_bounds = std::move(current_generic_bounds_);
        std::optional<std::size_t> previous_zone_pointer_return_param_index = current_zone_pointer_return_param_index_;
        std::string previous_zone_pointer_return_source = std::move(current_zone_pointer_return_source_);
        std::optional<std::size_t> previous_borrow_return_param_index = current_borrow_return_param_index_;
        std::string previous_borrow_return_param_name = std::move(current_borrow_return_param_name_);
        std::optional<std::string> previous_borrow_return_path = std::move(current_borrow_return_path_);
        current_module_name_ = fn.module_name;
        current_type_substitutions_ = std::move(substitutions);
        current_generic_bounds_.clear();
        for (const auto& generic : fn.generics) {
            if (generic.has_constraint) current_generic_bounds_.push_back(resolve_generic_trait_bound(generic));
        }
        current_zone_pointer_return_param_index_.reset();
        current_zone_pointer_return_source_.clear();
        current_borrow_return_param_index_.reset();
        current_borrow_return_param_name_.clear();
        current_borrow_return_path_.reset();
        allow_zone_temp_init_ = false;
        local_scopes_.clear();
        loops_.clear();
        borrow_context_.clear();
        push_scope();

        IrFunction ir_fn;
        ir_fn.name = lowered_name;
        ir_fn.module_name = fn.module_name;
        ir_fn.loc = fn.loc;
        auto sig_found = functions_.find(lowered_name);
        const FunctionSig* active_sig = sig_found != functions_.end() ? &sig_found->second : nullptr;
        current_return_ = active_sig
            ? active_sig->result
            : (fn.has_return_type ? resolve_executable_type(fn.return_type) : void_type(fn.loc));
        if (fn.has_return_type) {
            require_root_vector_runtime_abi(fn.return_type.loc, current_return_, "a function return type");
        }
        ir_fn.return_type = current_return_;
        if (sig_found != functions_.end()) {
            ir_fn.link_name = sig_found->second.link_name;
            ir_fn.shared_export = sig_found->second.is_public || !sig_found->second.link_name.empty();
        }

        std::vector<IrStmtPtr> parameter_pattern_prelude;
        for (std::size_t param_index = 0; param_index < fn.params.size(); ++param_index) {
            const auto& param = fn.params[param_index];
            IrType type;
            if (active_sig && param_index < active_sig->params.size()) {
                type = active_sig->params[param_index];
            } else {
                bool vec_view = false;
                type = function_parameter_abi_type(
                    param.type.loc,
                    resolve_executable_type(param.type),
                    "a function parameter",
                    vec_view);
            }
            const Pattern* param_pattern = param.has_pattern ? &expanded_pattern(param.pattern) : nullptr;
            if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Void) {
                fail(fn.loc, "parameter cannot have void type");
            }
            if (param.has_pattern) {
                if (is_owner_type(type)) {
                    fail(param_pattern->loc, "owning parameter patterns are planned after ownership through parameter destructuring is defined");
                }
                if (contains_borrow_type(type)) {
                    fail(param_pattern->loc, "borrow parameter patterns are not supported yet; use a named parameter");
                }
            }
            std::string ir_param_name = param.has_pattern
                ? make_hidden_local("$param")
                : param.name;
            declare_local(fn.loc, ir_param_name, type, false);
            LocalInfo& param_local = local_slot_by_name(ir_param_name);
            param_local.function_parameter = true;
            param_local.generic_origin = param.has_pattern ? "" : generic_origin_from_type_ref(param.type);
            if (contains_borrow_type(type) && !is_borrow_type(type)) {
                seed_parameter_aggregate_borrow_sources(param_local, ir_param_name, type, "");
            }
            ir_fn.params.push_back(IrParam{ir_param_name, type});
            if (param.has_pattern) {
                lower_binding_pattern_from_local(
                    *param_pattern,
                    ir_param_name,
                    type,
                    false,
                    parameter_pattern_prelude
                );
            }
        }
        std::vector<IrType> current_param_types;
        current_param_types.reserve(ir_fn.params.size());
        for (const auto& param : ir_fn.params) current_param_types.push_back(param.type);
        current_zone_pointer_return_param_index_ =
            zone_pointer_return_param_index(current_param_types, current_return_);
        if (current_zone_pointer_return_param_index_) {
            current_zone_pointer_return_source_ = ir_fn.params[*current_zone_pointer_return_param_index_].name;
        }
        current_borrow_return_param_index_ =
            sig_found != functions_.end()
                ? sig_found->second.borrow_return_param_index
                : borrow_return_param_index(current_param_types, current_return_);
        if (current_borrow_return_param_index_) {
            current_borrow_return_param_name_ = ir_fn.params[*current_borrow_return_param_index_].name;
            if (sig_found != functions_.end() &&
                sig_found->second.borrow_return_contract_explicit) {
                current_borrow_return_path_ = sig_found->second.borrow_return_path;
            }
        }
        seed_source_std_same_zone_parameter_sources(fn);

        CheckedStatements body = check_statements(fn.body, false);
        if (body.flow == Flow::Continues) {
            append_auto_destroy_zone_cleanup(fn.loc, body.statements);
        }
        ir_fn.body = std::move(parameter_pattern_prelude);
        for (auto& stmt : body.statements) ir_fn.body.push_back(std::move(stmt));
        if (body.flow == Flow::Continues && !is_void_value_type(current_return_)) {
            fail(fn.loc, "function '" + fn.name + "' must return a value on all paths");
        }
        if (body.flow == Flow::Returns) discard_scope();
        else pop_scope();
        if (current_borrow_return_param_index_) {
            auto found = functions_.find(lowered_name);
            if (found != functions_.end()) {
                found->second.borrow_return_path = current_borrow_return_path_.value_or("");
            }
        }
        current_module_name_ = previous_module;
        current_type_substitutions_ = std::move(previous_substitutions);
        current_generic_bounds_ = std::move(previous_generic_bounds);
        current_zone_pointer_return_param_index_ = previous_zone_pointer_return_param_index;
        current_zone_pointer_return_source_ = std::move(previous_zone_pointer_return_source);
        current_borrow_return_param_index_ = previous_borrow_return_param_index;
        current_borrow_return_param_name_ = std::move(previous_borrow_return_param_name);
        current_borrow_return_path_ = std::move(previous_borrow_return_path);
        return ir_fn;
    }

    void push_scope() {
        local_scopes_.push_scope();
    }

    void end_scope(bool check_owners) {
        local_scopes_.end_scope(
            check_owners,
            [this](const LocalInfo& local) {
                release_named_borrow(local);
            },
            [this](const LocalInfo& local) {
                return local_has_live_owner(local);
            },
            [](const std::string& name, const LocalInfo& local) {
                fail(local.loc, "owning binding '" + name + "' must be moved or dropped before scope exit");
            }
        );
    }

    void pop_scope() {
        end_scope(true);
    }

    void discard_scope() {
        end_scope(false);
    }

    void add_borrow_source(LocalInfo& source, const std::string& path, bool mutable_borrow) {
        borrow_context_.add_source(source, path, mutable_borrow);
    }

    void release_borrow_source(const std::string& name, const std::string& path, bool mutable_borrow) {
        borrow_context_.release_source(name, path, mutable_borrow);
    }

    void release_named_borrow(const LocalInfo& borrow) {
        if (borrow.borrow_sources_released) return;
        borrow_context_.release_named(borrow);
    }

    void release_named_borrow_now(LocalInfo& borrow) {
        if (borrow.borrow_sources_released) return;
        release_named_borrow(borrow);
        borrow.borrow_source.clear();
        borrow.borrow_source_path.clear();
        borrow.borrow_source_mutable = false;
        borrow.aggregate_borrow_sources.clear();
        borrow.borrow_sources_released = true;
    }

    void promote_temporary_borrow_to_named(SourceLocation loc, const IrExpr& init, const std::string& binding_name) {
        LocalInfo& binding = local_slot_by_name(binding_name);
        borrow_context_.promote_to_named(loc, init, binding_name, binding);
    }

    void promote_temporary_borrows_to_aggregate(std::size_t mark,
                                                const std::string& binding_name,
                                                const std::string& target_path = "") {
        LocalInfo& binding = local_slot_by_name(binding_name);
        borrow_context_.promote_to_aggregate(mark, binding, target_path);
    }

    void release_aggregate_borrow_sources_at(const std::string& binding_name,
                                             const std::string& target_path) {
        LocalInfo& binding = local_slot_by_name(binding_name);
        local_scopes_.release_aggregate_borrow_sources_at(binding, target_path);
    }

    void copy_aggregate_borrow_sources_to_temporaries(SourceLocation loc,
                                                      const std::string& binding_name,
                                                      const LocalInfo& binding) {
        (void)binding_name;
        if (!contains_borrow_type(binding.type) || is_borrow_type(binding.type) || is_owner_type(binding.type)) {
            return;
        }
        for (const auto& source_info : binding.aggregate_borrow_sources) {
            LocalInfo& source = require_live_local(loc, source_info.name);
            add_borrow_source(source, source_info.path, source_info.mutable_borrow);
            borrow_context_.push_temporary(
                source_info.aggregate_path,
                source_info.name,
                source_info.path,
                source_info.mutable_borrow);
        }
    }

    void seed_parameter_aggregate_borrow_sources(LocalInfo& binding,
                                                 const std::string& binding_name,
                                                 const IrType& type,
                                                 const std::string& path) {
        if (is_borrow_type(type)) {
            add_local_aggregate_borrow_source(
                binding,
                path,
                binding_name,
                path,
                type.qualifier == TypeQualifier::MutRef,
                false);
            return;
        }
        if (type.qualifier != TypeQualifier::Value) return;
        if (type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            type.primitive == IrPrimitiveKind::Struct) {
            const std::vector<IrType>& fields = aggregate_field_types(type);
            for (std::size_t i = 0; i < fields.size(); ++i) {
                seed_parameter_aggregate_borrow_sources(
                    binding,
                    binding_name,
                    fields[i],
                    local_owned_field_path(path, i));
            }
            return;
        }
        if (type.primitive == IrPrimitiveKind::Vector && type.args.size() == 1 && type.array_size != 0) {
            for (std::uint64_t i = 0; i < type.array_size; ++i) {
                seed_parameter_aggregate_borrow_sources(
                    binding,
                    binding_name,
                    type.args[0],
                    local_owned_field_path(path, static_cast<std::size_t>(i)));
            }
        }
    }

    const LocalInfo::BorrowSource* aggregate_borrow_source_for_path(const LocalInfo& binding,
                                                                    const std::string& target_path) const {
        for (const auto& source_info : binding.aggregate_borrow_sources) {
            if (source_info.aggregate_path == target_path) return &source_info;
        }
        return nullptr;
    }

    void activate_tracked_borrow_field_access(SourceLocation loc,
                                              const TrackedAggregateAccess& access,
                                              IrExpr& result) {
        if (!is_borrow_type(access.type)) return;
        LocalInfo& binding = local_slot_by_name(access.base_name);
        const LocalInfo::BorrowSource* source_info =
            aggregate_borrow_source_for_path(binding, access.path);
        if (!source_info) {
            fail(loc,
                 "borrow-valued aggregate field '" +
                     local_borrow_path_display(access.base_name, access.path) +
                     "' has no tracked source");
        }
        if (source_info->mutable_borrow) {
            require_can_borrow_path(loc, access.base_name, binding, access.path, true);
            add_borrow_source(binding, access.path, true);
            borrow_context_.push_temporary(access.base_name, access.path, true);
            set_borrow_result_source(result, BorrowResultSource{
                access.base_name,
                access.path,
                true
            });
            return;
        }
        LocalInfo& source = require_live_local(loc, source_info->name);
        require_can_borrow_path(loc, source_info->name, source, source_info->path, false);
        add_borrow_source(source, source_info->path, false);
        borrow_context_.push_temporary(source_info->name, source_info->path, false);
        set_borrow_result_source(result, BorrowResultSource{
            source_info->name,
            source_info->path,
            false
        });
    }

    void declare_local(SourceLocation loc, const std::string& name, const IrType& type, bool mutable_binding) {
        bool name_was_used = local_scopes_.name_was_used(name);
        if (name_was_used && !local_scopes_.reusable_pattern_binding(name)) {
            fail(loc, "local name '" + name + "' shadows or redeclares an existing local");
        }
        if (!name_was_used) local_scopes_.mark_name_used(name);
        LocalInfo local = make_local_info(loc, type, mutable_binding);
        initialize_owned_field_states(local);
        local_scopes_.declare_current(name, std::move(local));
    }

    AutoDestroyZoneCleanupContext auto_destroy_zone_cleanup_context() {
        return AutoDestroyZoneCleanupContext{
            local_scopes_,
            zone_pointer_source_resolver(),
            zone_pointer_locals(),
            [this](const std::string& prefix) {
                return make_hidden_local(prefix);
            }
        };
    }

    bool has_auto_destroy_zone_cleanup(std::size_t first_scope_index) const {
        return ari::has_auto_destroy_zone_cleanup(local_scopes_, first_scope_index);
    }

    void require_zone_pointer_not_escape_temporary_scope(
        SourceLocation loc,
        const IrExpr& value,
        std::size_t first_scope_index,
        const std::string& context
    ) {
        auto cleanup = auto_destroy_zone_cleanup_context();
        if (auto error = ari::require_no_temporary_zone_pointer_escape(
                value,
                first_scope_index,
                context,
                cleanup)) {
            fail(loc, *error);
        }
    }

    void append_auto_destroy_zone_cleanup(
        SourceLocation loc,
        std::vector<IrStmtPtr>& statements,
        std::size_t first_scope_index
    ) {
        auto cleanup = auto_destroy_zone_cleanup_context();
        if (auto error = ari::append_auto_destroy_zone_cleanup(
                loc,
                statements,
                cleanup,
                first_scope_index)) {
            fail(loc, *error);
        }
    }

    void append_auto_destroy_zone_cleanup(SourceLocation loc, std::vector<IrStmtPtr>& statements) {
        append_auto_destroy_zone_cleanup(loc, statements, 0);
    }

    void append_current_scope_auto_destroy_cleanup(SourceLocation loc, std::vector<IrStmtPtr>& statements) {
        if (local_scopes_.empty()) return;
        append_auto_destroy_zone_cleanup(loc, statements, local_scopes_.size() - 1);
    }

    IrExprPtr materialize_value_before_auto_destroy_cleanup(
        SourceLocation loc,
        IrExprPtr value,
        std::vector<IrStmtPtr>& statements,
        std::size_t first_scope_index,
        const std::string& hidden_prefix,
        const std::string& escape_context
    ) {
        auto cleanup = auto_destroy_zone_cleanup_context();
        AutoDestroyZoneMaterialization materialized = ari::materialize_value_before_auto_destroy_cleanup(
            loc,
            std::move(value),
            statements,
            cleanup,
            first_scope_index,
            hidden_prefix,
            escape_context);
        if (materialized.error) fail(loc, *materialized.error);
        return std::move(materialized.value);
    }

    void materialize_values_before_auto_destroy_cleanup(
        SourceLocation loc,
        std::vector<IrExprPtr>& values,
        std::vector<IrStmtPtr>& statements,
        std::size_t first_scope_index,
        const std::string& hidden_prefix,
        const std::string& escape_context
    ) {
        auto cleanup = auto_destroy_zone_cleanup_context();
        if (auto error = ari::materialize_values_before_auto_destroy_cleanup(
                loc,
                values,
                statements,
                cleanup,
                first_scope_index,
                hidden_prefix,
                escape_context)) {
            fail(loc, *error);
        }
    }

    static bool is_zone_temp_call(const IrExpr& expr) {
        return expr.kind == IrExprKind::Call && ir_expr_name(expr) == "zone::temp";
    }

    static bool contains_zone_temp_call(const IrExpr& expr) {
        if (is_zone_temp_call(expr)) return true;
        auto has_temp = [](const IrExprPtr& child) {
            return child && contains_zone_temp_call(*child);
        };
        if (has_temp(ir_expr_operand(expr)) || has_temp(ir_expr_payload(expr)) || has_temp(ir_expr_left(expr)) ||
            has_temp(ir_expr_right(expr)) || has_temp(ir_expr_if_condition(expr)) ||
            has_temp(ir_expr_if_then_value(expr)) || has_temp(ir_expr_if_else_value(expr)) ||
            has_temp(ir_expr_block_value(expr)) || has_temp(ir_expr_match_value(expr))) {
            return true;
        }
        for (const auto& arg : expr.args) {
            if (has_temp(arg)) return true;
        }
        return false;
    }

    LocalInfo* find_local_slot(const std::string& name) {
        return local_scopes_.find(name);
    }

    LocalInfo& require_local_slot(SourceLocation loc, const std::string& name) {
        if (LocalInfo* local = find_local_slot(name)) return *local;
        fail(loc, "unknown name '" + name + "'");
    }

    LocalInfo& local_slot_by_name(const std::string& name) {
        return local_scopes_.require_for_restore(name);
    }

    LocalInfo& require_live_local(SourceLocation loc, const std::string& name) {
        LocalInfo& local = require_local_slot(loc, name);
        if (auto error = local_unavailable_binding_error(name, local)) fail(loc, *error);
        return local;
    }

    StateSnapshot snapshot_states() const {
        return local_scopes_.snapshot_states();
    }

    void restore_states(const StateSnapshot& snapshot) {
        local_scopes_.restore_states(snapshot);
    }

    void restore_merged_states(StateSnapshot target, const StateSnapshot& source) {
        local_scopes_.restore_merged_zone_generations(std::move(target), source);
    }

    static void require_same_states(
        SourceLocation loc,
        const StateSnapshot& left,
        const StateSnapshot& right,
        const std::string& message
    ) {
        if (auto error = state_snapshot_mismatch_error(left, right, message)) fail(loc, *error);
    }

    static std::set<std::string> loop_binding_name_set(const LoopInfo& loop) {
        return std::set<std::string>(loop.names.begin(), loop.names.end());
    }

    static void require_same_states_except_loop_bindings(
        SourceLocation loc,
        const StateSnapshot& left,
        const StateSnapshot& right,
        const LoopInfo& loop,
        const std::string& message
    ) {
        if (auto error = loop_state_mismatch_error_ignoring_bindings(
                left,
                right,
                loop_binding_name_set(loop),
                message)) {
            fail(loc, *error);
        }
    }

    static bool loop_has_owner_bindings(const LoopInfo& loop) {
        for (const auto& type : loop.types) {
            if (is_owner_type(type)) return true;
        }
        return false;
    }

    static void merge_break_exit_states(
        SourceLocation loc,
        StateSnapshot& exit_state,
        const LoopInfo& loop,
        const std::string& message
    ) {
        if (auto error = merge_loop_exit_states(exit_state, loop.break_state_snapshots, message)) {
            fail(loc, *error);
        }
    }

    static void merge_continue_states(
        SourceLocation loc,
        StateSnapshot& loop_state,
        const LoopInfo& loop
    ) {
        if (auto error = merge_loop_state_snapshots_conservatively(
                loop_state,
                loop.continue_state_snapshots,
                "has incompatible ownership states at loop continue")) {
            fail(loc, *error);
        }
    }

    static StateSnapshot loop_exit_base_state(
        const StateSnapshot& loop_input,
        const LoopInfo& loop,
        bool has_zero_iteration_exit
    ) {
        return !has_zero_iteration_exit && !loop.break_state_snapshots.empty()
            ? project_loop_state_snapshot(loop_input, loop.break_state_snapshots.front())
            : loop_input;
    }

    static void merge_loop_continue_snapshots(
        SourceLocation loc,
        StateSnapshot& exit_state,
        StateSnapshot next_iteration_state,
        const LoopInfo& loop,
        bool has_zero_iteration_exit
    ) {
        if (has_zero_iteration_exit) {
            merge_continue_states(loc, exit_state, loop);
            return;
        }
        merge_continue_states(loc, next_iteration_state, loop);
        merge_existing_zone_generations_into(exit_state, next_iteration_state);
    }

    void recheck_loop_body_under_state(
        SourceLocation loc,
        const LoopInfo& loop,
        const StateSnapshot& recheck_state,
        const LoopBodyRecheck& recheck
    ) {
        if (!recheck.statements) {
            fail(loc, "internal error: missing loop body for widened owner-state recheck");
        }

        StateSnapshot saved_state = snapshot_states();
        LocalScopeStack::NameState saved_name_state = local_scopes_.snapshot_name_state();
        std::vector<LoopInfo> saved_loops = loops_;
        std::size_t saved_warning_count = warnings_.size();
        int saved_hidden_local_counter = hidden_local_counter_;
        std::size_t borrow_mark = temporary_borrow_mark();

        restore_states(recheck_state);
        local_scopes_.restore_name_state(recheck.name_state);

        LoopInfo rechecked_loop = loop;
        rechecked_loop.break_state_snapshots.clear();
        rechecked_loop.continue_state_snapshots.clear();
        push_loop(loc, std::move(rechecked_loop));
        (void)check_statements(*recheck.statements, recheck.scoped);
        loops_.pop_back();

        release_temporary_borrows(borrow_mark);
        restore_states(saved_state);
        local_scopes_.restore_name_state(std::move(saved_name_state));
        loops_ = std::move(saved_loops);
        warnings_.resize(saved_warning_count);
        hidden_local_counter_ = saved_hidden_local_counter;
    }

    StateSnapshot checked_loop_exit_state(
        SourceLocation loc,
        const StateSnapshot& loop_input,
        const StateSnapshot& loop_body_state,
        const LoopInfo& loop,
        Flow body_flow,
        bool has_zero_iteration_exit = true,
        const LoopBodyRecheck* recheck = nullptr
    ) {
        StateSnapshot exit_state = loop_exit_base_state(loop_input, loop, has_zero_iteration_exit);
        StateSnapshot next_iteration_state = loop_input;
        const bool allow_owner_widening = !has_zero_iteration_exit && recheck && recheck->statements;
        bool widened_owner_state = false;
        auto merge_next_iteration_states =
            [&](const std::vector<StateSnapshot>& snapshots, const std::string& message) {
                if (allow_owner_widening) {
                    if (auto error = merge_loop_state_snapshots_with_owner_widening(
                            next_iteration_state,
                            snapshots,
                            message,
                            widened_owner_state)) {
                        fail(loc, *error);
                    }
                    return;
                }
                if (auto error = merge_loop_state_snapshots_conservatively(
                        next_iteration_state,
                        snapshots,
                        message)) {
                    fail(loc, *error);
                }
            };

        if (body_flow == Flow::Continues) {
            merge_next_iteration_states(
                std::vector<StateSnapshot>{loop_body_state},
                "cannot change ownership state inside loop yet"
            );
            if (has_zero_iteration_exit) {
                if (auto error = merge_loop_state_snapshots_conservatively(
                        exit_state,
                        std::vector<StateSnapshot>{loop_body_state},
                        "cannot change ownership state inside loop yet")) {
                    fail(loc, *error);
                }
            }
        }
        if (has_zero_iteration_exit) {
            merge_continue_states(loc, exit_state, loop);
        } else {
            merge_next_iteration_states(
                loop.continue_state_snapshots,
                "has incompatible ownership states at loop continue"
            );
            if (widened_owner_state) {
                recheck_loop_body_under_state(loc, loop, next_iteration_state, *recheck);
            }
            merge_existing_zone_generations_into(exit_state, next_iteration_state);
        }
        merge_break_exit_states(loc, exit_state, loop, "has incompatible ownership states after loop exits");
        return exit_state;
    }

    static std::optional<bool> literal_bool_condition_value(const IrExpr& condition) {
        if (condition.kind != IrExprKind::Bool) return std::nullopt;
        return condition.bool_value;
    }

    static bool is_literal_true_condition(const IrExpr& condition) {
        return literal_bool_condition_value(condition).value_or(false);
    }

    std::optional<bool> known_bool_condition_value(const IrExpr& condition) {
        if (auto literal = literal_bool_condition_value(condition)) return literal;
        if (condition.kind != IrExprKind::Local) return std::nullopt;
        LocalInfo* local = find_local_slot(ir_expr_name(condition));
        if (!local || local->mutable_binding || !local->ir_init_expr) return std::nullopt;
        if (local->ir_init_expr->kind != IrExprKind::Bool) return std::nullopt;
        return local->ir_init_expr->bool_value;
    }

    static bool literal_true_loop_never_falls_through(const LoopInfo& loop, Flow body_flow) {
        return body_flow != Flow::Returns && loop.break_state_snapshots.empty();
    }

    void collect_owned_field_states(const IrType& type,
                                    const std::string& path,
                                    std::map<std::string, LocalState>& states) const {
        if (type.qualifier == TypeQualifier::Own) {
            if (!path.empty()) states.emplace(path, LocalState::Alive);
            return;
        }
        if (type.qualifier != TypeQualifier::Value) {
            return;
        }

        if (type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            type.primitive == IrPrimitiveKind::Struct) {
            const std::vector<IrType>& fields = aggregate_field_types(type);
            for (std::size_t i = 0; i < fields.size(); ++i) {
                std::string child_path = path.empty()
                    ? std::to_string(i)
                    : path + "." + std::to_string(i);
                collect_owned_field_states(fields[i], child_path, states);
            }
            return;
        }

        if (type.primitive == IrPrimitiveKind::Vector && type.args.size() == 1 && type.array_size != 0) {
            for (std::uint64_t i = 0; i < type.array_size; ++i) {
                std::string child_path = path.empty()
                    ? std::to_string(i)
                    : path + "." + std::to_string(i);
                collect_owned_field_states(type.args[0], child_path, states);
            }
        }
    }

    void initialize_owned_field_states(LocalInfo& local) const {
        local.owned_field_states.clear();
        collect_owned_field_states(local.type, "", local.owned_field_states);
    }

    ZonePointerLocalAdapter zone_pointer_locals() {
        ZonePointerLocalAdapter locals;
        locals.find_local = [this](const std::string& name) {
            return find_local_slot(name);
        };
        return locals;
    }

    bool zone_source_name_from_arg(const IrExpr& zone_arg, std::string& out) {
        return ari::zone_source_name_from_arg(zone_arg, zone_pointer_locals(), out);
    }

    ZonePointerSourceResolver zone_pointer_source_resolver() {
        ZonePointerSourceResolver resolver;
        resolver.local_zone_pointer_source = [this](const std::string& name, std::string& out) {
            const LocalInfo* local = find_local_slot(name);
            if (!local || !local->zone_pointer) return false;
            out = local->zone_pointer_source;
            return true;
        };
        resolver.zone_source_from_arg = [this](const IrExpr& zone_arg, std::string& out) {
            return zone_source_name_from_arg(zone_arg, out);
        };
        resolver.call_zone_return_param_index = [this](const std::string& name) -> std::optional<std::size_t> {
            auto found = functions_.find(name);
            if (found == functions_.end()) return std::nullopt;
            return found->second.zone_pointer_return_param_index;
        };
        return resolver;
    }

    bool zone_pointer_source_name_from_expr(const IrExpr& value, std::string& out) {
        return ari::zone_pointer_source_name_from_expr(value, zone_pointer_source_resolver(), out);
    }

    bool is_zone_pointer_expr(const IrExpr& value) {
        std::string source_name;
        return zone_pointer_source_name_from_expr(value, source_name);
    }

    void seed_source_std_same_zone_parameter_sources(const FunctionDecl& fn) {
        if (fn.params.size() < 2) return;

        const std::string method_name = basename_of_qualified_name(fn.name);
        LocalInfo* receiver = find_local_slot(fn.params[0].name);
        LocalInfo* zone = find_local_slot(fn.params[1].name);
        if (!receiver || !zone || !is_zone_source_type(zone->type)) return;

        IrType receiver_type = value_qualified_type(receiver->type);
        bool same_zone_source_method =
            (fn.module_name == "std::boxed" &&
             std_box_method_requires_same_zone_argument(method_name) &&
             is_std_box_handle_type(receiver_type)) ||
            (fn.module_name == "std::string" &&
             std_string_method_requires_same_zone_argument(method_name) &&
             is_std_string_handle_type(receiver_type)) ||
            (fn.module_name == "std::vec" &&
             std_vec_method_requires_same_zone_argument(method_name) &&
             is_std_vec_handle_type(receiver_type));
        if (!same_zone_source_method) return;

        set_zone_pointer_source_from_name(*receiver, fn.params[1].name, zone_pointer_locals());
    }

    void require_no_zone_pointer_escape(SourceLocation loc, const IrExpr& value, const std::string& context) {
        if (auto error = zone_pointer_escape_error(
                value,
                context,
                zone_pointer_source_resolver(),
                zone_pointer_locals(),
                local_scopes_)) {
            fail(loc, *error);
        }
    }

    void require_std_vec_same_zone_method_matches_source(SourceLocation loc,
                                                         const std::string& method_name,
                                                         const IrType& receiver_type,
                                                         const std::vector<IrExprPtr>& args) {
        std::optional<std::string> violation = std_vec_same_zone_method_violation(
            method_name,
            receiver_type,
            args,
            [this](const IrExpr& value, std::string& out) {
                return zone_pointer_source_name_from_expr(value, out);
            },
            [this](const IrExpr& value, std::string& out) {
                return zone_source_name_from_arg(value, out);
            });
        if (violation) {
            fail(loc, *violation);
        }
    }

    void require_std_box_same_zone_method_matches_source(SourceLocation loc,
                                                         const std::string& method_name,
                                                         const IrType& receiver_type,
                                                         const std::vector<IrExprPtr>& args) {
        std::optional<std::string> violation = std_box_same_zone_method_violation(
            method_name,
            receiver_type,
            args,
            [this](const IrExpr& value, std::string& out) {
                return zone_pointer_source_name_from_expr(value, out);
            },
            [this](const IrExpr& value, std::string& out) {
                return zone_source_name_from_arg(value, out);
            });
        if (violation) {
            fail(loc, *violation);
        }
    }

    void require_std_string_same_zone_method_matches_source(SourceLocation loc,
                                                            const std::string& method_name,
                                                            const IrType& receiver_type,
                                                            const std::vector<IrExprPtr>& args) {
        std::optional<std::string> violation = std_string_same_zone_method_violation(
            method_name,
            receiver_type,
            args,
            [this](const IrExpr& value, std::string& out) {
                return zone_pointer_source_name_from_expr(value, out);
            },
            [this](const IrExpr& value, std::string& out) {
                return zone_source_name_from_arg(value, out);
            });
        if (violation) {
            fail(loc, *violation);
        }
    }

    ExprPtr make_zone_argument_expr_from_source(SourceLocation loc,
                                                const std::string& source_name,
                                                const std::string& context) {
        if (source_name == "<multiple zones>") {
            fail(loc, context + " receiver must have one tracked allocation zone");
        }
        const LocalInfo* zone = find_local_slot(source_name);
        if (!zone || !is_zone_source_type(zone->type)) {
            fail(loc, context + " tracked zone source '" + source_name + "' is not available");
        }
        if (is_zone_borrow_type(zone->type)) {
            if (zone->type.qualifier != TypeQualifier::MutRef) {
                fail(loc, context + " requires a mutable zone source");
            }
            return make_ast_name_expr(loc, source_name);
        }
        return make_ast_borrow_expr(loc, make_ast_name_expr(loc, source_name), true);
    }

    LocalInfo* implicit_zone_receiver_local(const Expr& expr,
                                            bool (*is_handle_type)(const IrType&)) {
        const Expr* receiver = expr_operand(expr).get();
        if (!receiver || receiver->kind != ExprKind::Name) return nullptr;
        LocalInfo* local = find_local_slot(receiver->name);
        if (!local) return nullptr;
        if (!is_handle_type(value_qualified_type(local->type))) return nullptr;
        return local;
    }

    ExprPtr rewrite_std_vec_implicit_zone_method_call(const Expr& expr) {
        if (current_module_name_ == "std::vec") return nullptr;
        std::optional<StdVecImplicitZoneMethod> implicit =
            std_vec_implicit_zone_method_for_call(expr.name, expr.args.size());
        if (!implicit || !expr_type_args(expr).empty()) return nullptr;

        LocalInfo* receiver = implicit_zone_receiver_local(expr, is_std_vec_handle_type);
        if (!receiver) return nullptr;
        const std::string context = "std::vec::Vec." + expr.name;
        if (!receiver->zone_pointer || receiver->zone_pointer_source.empty()) {
            if (implicit->allow_untracked_fallback) return nullptr;
            fail(expr.loc,
                 context +
                     " receiver must come from a tracked zone allocation to infer its zone; pass an explicit zone argument instead");
        }

        std::vector<ExprPtr> args;
        args.reserve(expr.args.size() + 1);
        args.push_back(make_zone_argument_expr_from_source(
            expr.loc,
            receiver->zone_pointer_source,
            context));
        for (const auto& arg : expr.args) {
            args.push_back(clone_expression_tree(*arg));
        }

        return make_ast_method_call_expr(
            expr.loc,
            clone_expression_tree(*expr_operand(expr)),
            implicit->lowered_name,
            {},
            std::move(args));
    }

    ExprPtr rewrite_std_string_implicit_zone_method_call(const Expr& expr) {
        if (current_module_name_ == "std::string") return nullptr;
        std::optional<StdStringImplicitZoneMethod> implicit =
            std_string_implicit_zone_method_for_call(expr.name, expr.args.size());
        if (!implicit || !expr_type_args(expr).empty()) return nullptr;

        LocalInfo* receiver = implicit_zone_receiver_local(expr, is_std_string_handle_type);
        if (!receiver) return nullptr;
        const std::string context = "std::string::String." + expr.name;
        if (!receiver->zone_pointer || receiver->zone_pointer_source.empty()) {
            if (implicit->allow_untracked_fallback) return nullptr;
            fail(expr.loc,
                 context +
                     " receiver must come from a tracked zone allocation to infer its zone; pass an explicit zone argument instead");
        }

        std::vector<ExprPtr> args;
        args.reserve(expr.args.size() + 1);
        args.push_back(make_zone_argument_expr_from_source(
            expr.loc,
            receiver->zone_pointer_source,
            context));
        for (const auto& arg : expr.args) {
            args.push_back(clone_expression_tree(*arg));
        }

        return make_ast_method_call_expr(
            expr.loc,
            clone_expression_tree(*expr_operand(expr)),
            implicit->lowered_name,
            {},
            std::move(args));
    }

    void set_zone_pointer_source_from_expr(LocalInfo& target, const IrExpr& value) {
        ari::set_zone_pointer_source_from_expr(
            target,
            value,
            zone_pointer_source_resolver(),
            zone_pointer_locals());
    }

    void require_zone_pointer_valid(SourceLocation loc,
                                    const std::string& pointer_name,
                                    const LocalInfo& pointer) {
        if (std::optional<std::string> error =
                zone_pointer_invalid_error(pointer_name, pointer, zone_pointer_locals())) {
            fail(loc, *error);
        }
    }

    void mark_zone_reset_call(const IrExpr& call) {
        ari::mark_zone_reset_call(call, zone_pointer_locals());
    }

    void require_no_live_owners_before_return(SourceLocation loc) const {
        local_scopes_.for_each_local_from(
            0,
            [&](const std::string& name, const LocalInfo& local) {
                if (local_has_live_owner(local)) {
                    fail(loc, "owning binding '" + name + "' must be moved or dropped before return");
                }
            }
        );
    }

    void require_no_live_owners_before_scope_jump(
        SourceLocation loc,
        std::size_t first_scope_index,
        const std::string& jump_name
    ) const {
        local_scopes_.for_each_local_from(
            first_scope_index,
            [&](const std::string& name, const LocalInfo& local) {
                if (local_has_live_owner(local)) {
                    fail(loc, "owning binding '" + name + "' must be moved or dropped before " + jump_name);
                }
            }
        );
    }

    std::size_t temporary_borrow_mark() const {
        return borrow_context_.mark();
    }

    void release_temporary_borrows(std::size_t mark) {
        borrow_context_.release_to_mark(mark);
    }

    void prefix_temporary_borrow_targets(std::size_t mark, const std::string& target_path) {
        borrow_context_.prefix_temporary_targets(mark, target_path);
    }

    BorrowCallLocalAdapter borrow_call_locals() {
        BorrowCallLocalAdapter locals;
        locals.find_local = [this](const std::string& name) {
            return find_local_slot(name);
        };
        locals.require_live_local = [this](SourceLocation loc, const std::string& name) -> LocalInfo& {
            return require_live_local(loc, name);
        };
        locals.add_borrow_source = [this](LocalInfo& local, const std::string& path, bool mutable_borrow) {
            add_borrow_source(local, path, mutable_borrow);
        };
        locals.push_temporary_borrow =
            [this](std::string name, std::string path, bool mutable_borrow) {
                borrow_context_.push_temporary(std::move(name), std::move(path), mutable_borrow);
            };
        return locals;
    }

    void require_borrow_return_source(SourceLocation loc, const IrExpr& value) {
        if (!current_borrow_return_param_index_) {
            fail(loc, "borrow-valued function returns currently require exactly one borrow parameter");
        }
        std::optional<BorrowResultSource> source = expr_borrow_result_source(value);
        if (!source) {
            fail(loc,
                 "function return borrow result must come directly from a borrow parameter, ref, ref mut, or compatible borrow control-flow result");
        }
        BorrowCallLocalAdapter locals = borrow_call_locals();
        BorrowResultSource root = root_borrow_result_source(*source, locals);
        if (root.name != current_borrow_return_param_name_) {
            fail(loc, "function return cannot return a borrow of local binding '" + root.name + "'");
        }
        if (!current_borrow_return_path_) {
            current_borrow_return_path_ = root.path;
        } else if (*current_borrow_return_path_ != root.path) {
            fail(loc, "function return borrow result must use the same source path on every return");
        }
    }

    IrExprPtr finish_tracked_call(SourceLocation loc,
                                  const std::string& display_name,
                                  std::string lowered_name,
                                  const FunctionSig& sig,
                                  std::vector<IrExprPtr> args,
                                  std::size_t borrow_mark) {
        BorrowCallContract contract{
            sig.result,
            sig.borrow_return_param_index,
            sig.borrow_return_path,
            sig.is_extern,
            sig.borrow_return_contract_explicit
        };
        std::optional<BorrowResultSource> source =
            call_borrow_result_source(loc, display_name, contract, args);
        release_temporary_borrows(borrow_mark);
        IrExprPtr call = make_ir_call_expr(loc, std::move(lowered_name), sig.result, std::move(args));
        if (source) {
            BorrowCallLocalAdapter locals = borrow_call_locals();
            activate_borrow_result(loc, *call, *source, locals);
        }
        return call;
    }

    bool is_diverging_call(const IrExpr& expr) const {
        if (is_diverging_builtin_call(expr)) return true;
        if (expr.kind != IrExprKind::Call) return false;
        auto found = functions_.find(ir_expr_name(expr));
        return found != functions_.end() &&
               found->second.is_extern &&
               found->second.extern_abi == "ari" &&
               is_diverging_builtin_symbol(found->second.link_name);
    }

    bool is_diverging_source_call(const Expr& expr) const {
        if (expr.kind != ExprKind::Call || expr_operand(expr)) return false;
        std::string prelude_name = resolve_use_path(expr.name);
        bool local_decl_shadows_prelude = unqualified_decl_shadows_prelude_name(expr.name, prelude_name);
        if (!local_decl_shadows_prelude) {
            if (is_diverging_builtin_source_name(prelude_name)) return true;
        }
        std::string function_name = resolve_function_name(expr.name);
        auto found = functions_.find(function_name);
        return found != functions_.end() &&
               found->second.is_extern &&
               found->second.extern_abi == "ari" &&
               is_diverging_builtin_symbol(found->second.link_name);
    }

    bool is_diverging_value_expr(const Expr& source, const IrExpr& lowered) const {
        return is_diverging_call(lowered) || is_diverging_source_call(source);
    }

    void require_borrow_result_source_outlives_scope(SourceLocation loc,
                                                     const BorrowResultSource& source,
                                                     std::size_t scope_index,
                                                     const std::string& context) const {
        std::size_t source_scope = 0;
        if (!local_scopes_.scope_index(source.name, source_scope)) {
            throw CompileError("internal error: missing borrow result source '" + source.name + "'");
        }
        if (source_scope >= scope_index) {
            fail(loc, context + " cannot return a borrow of local binding '" + source.name + "'");
        }
    }

    std::optional<BorrowResultSource> finish_control_flow_borrow_result(
        SourceLocation loc,
        const std::string& context,
        IrExpr& value,
        std::vector<IrStmtPtr>& statements,
        std::size_t scope_index,
        std::size_t borrow_mark
    ) {
        if (!is_borrow_type(value.type)) return std::nullopt;
        std::optional<BorrowResultSource> source = expr_borrow_result_source(value);
        if (!source) {
            fail(loc, context + " borrow result must come directly from ref, ref mut, or a compatible borrow control-flow result");
        }
        require_borrow_result_source_outlives_scope(loc, *source, scope_index, context);
        require_zone_pointer_not_escape_temporary_scope(loc, value, scope_index, context + " result");
        release_temporary_borrows(borrow_mark);
        append_auto_destroy_zone_cleanup(loc, statements, scope_index);
        return source;
    }

    void require_same_borrow_result_source(SourceLocation loc,
                                           const std::string& context,
                                           std::optional<BorrowResultSource>& expected,
                                           const BorrowResultSource& actual) const {
        if (!expected) {
            expected = actual;
            return;
        }
        if (!same_borrow_result_source(*expected, actual)) {
            fail(loc, context + " must borrow the same source path and mode in every result arm");
        }
    }

    void activate_control_flow_borrow_result(SourceLocation loc,
                                             IrExpr& result,
                                             const BorrowResultSource& source) {
        BorrowCallLocalAdapter locals = borrow_call_locals();
        activate_borrow_result(loc, result, source, locals);
    }

    void release_dead_named_borrows(const NameUseCounts& remaining, std::size_t first_scope_index) {
        bool released = false;
        do {
            released = false;
            local_scopes_.for_each_local_from_inner_to_outer(
                first_scope_index,
                [&](const std::string& name, LocalInfo& local) {
                    if (!is_borrow_type(local.type)) return;
                    if (local.function_parameter || local.borrow_sources_released) return;
                    if (local.borrow_source.empty() && local.aggregate_borrow_sources.empty()) return;
                    if (has_remaining_name_use(remaining, name)) return;
                    if (local_has_active_borrows(local) || local_has_active_field_borrows(local)) return;
                    release_named_borrow_now(local);
                    released = true;
                }
            );
        } while (released);
    }

    CheckedStatements check_statements(const std::vector<StmtPtr>& statements, bool scoped) {
        if (scoped) push_scope();
        std::size_t nll_scope_index = local_scopes_.empty() ? 0 : local_scopes_.size() - 1;
        StatementNameUses name_uses = collect_statement_name_uses(statements);

        CheckedStatements checked;
        checked.statements.reserve(statements.size());
        for (std::size_t i = 0; i < statements.size(); ++i) {
            const auto& stmt = statements[i];
            if (checked.flow == Flow::Returns) fail(stmt->loc, "unreachable statement after return");
            CheckedStatement lowered = check_statement(*stmt);
            if (lowered.flow != Flow::Continues) checked.flow = lowered.flow;
            checked.statements.push_back(std::move(lowered.statement));
            subtract_name_uses(name_uses.remaining, name_uses.per_statement[i]);
            release_dead_named_borrows(name_uses.remaining, nll_scope_index);
        }

        if (scoped) {
            if (checked.flow == Flow::Returns) discard_scope();
            else if (checked.flow == Flow::Stops) discard_scope();
            else {
                SourceLocation cleanup_loc = statements.empty() ? SourceLocation{} : statements.back()->loc;
                append_current_scope_auto_destroy_cleanup(cleanup_loc, checked.statements);
                pop_scope();
            }
        }
        return checked;
    }

    CheckedStatement check_statement(const Stmt& stmt) {
        auto lowered = std::make_unique<IrStmt>();
        lowered->kind = lower_stmt_kind(stmt.kind, stmt.loc);
        lowered->loc = stmt.loc;
        Flow flow = Flow::Continues;

        switch (stmt.kind) {
            case StmtKind::Block:
                {
                    const std::string& label = stmt_label(stmt);
                    set_ir_stmt_label(*lowered, label);
                    if (!label.empty()) push_labeled_block(stmt.loc, label);
                    CheckedStatements block = check_statements(stmt_statements(stmt), true);
                    if (!label.empty()) loops_.pop_back();
                    flow = block.flow;
                    if (!label.empty() && flow == Flow::Stops) flow = Flow::Continues;
                    set_ir_stmt_statements(*lowered, std::move(block.statements));
                }
                break;
            case StmtKind::VarDecl:
                check_var_decl(stmt, *lowered);
                break;
            case StmtKind::Assign:
                check_assign(stmt, *lowered);
                break;
            case StmtKind::ExprStmt:
                {
                    std::size_t borrow_mark = temporary_borrow_mark();
                    lowered->expr = check_expr(*stmt.expr);
                    if (!contains_borrow_type(lowered->expr->type)) release_temporary_borrows(borrow_mark);
                }
                if (contains_borrow_type(lowered->expr->type)) {
                    fail(stmt.loc, "borrow expression result must be passed directly to a call");
                }
                if (is_owner_type(lowered->expr->type)) {
                    fail(stmt.loc, "owning expression result must be bound, returned, passed, or dropped");
                }
                if (is_diverging_value_expr(*stmt.expr, *lowered->expr)) {
                    flow = Flow::Stops;
                }
                break;
            case StmtKind::Return:
                check_return(stmt, *lowered);
                flow = Flow::Returns;
                break;
            case StmtKind::If:
                flow = check_if(stmt, *lowered);
                break;
            case StmtKind::While:
                flow = check_while(stmt, *lowered);
                break;
            case StmtKind::WhileLet:
                check_while_let(stmt, *lowered);
                break;
            case StmtKind::For:
                check_for(stmt, *lowered);
                break;
            case StmtKind::InitWhile:
                flow = check_init_while(stmt, *lowered);
                break;
            case StmtKind::Continue:
                check_continue(stmt, *lowered);
                flow = Flow::Stops;
                break;
            case StmtKind::Break:
                check_break(stmt, *lowered);
                flow = Flow::Stops;
                break;
            case StmtKind::Match:
                flow = check_match(stmt, *lowered);
                break;
            case StmtKind::Drop:
                check_drop(stmt, *lowered);
                break;
        }

        return CheckedStatement{std::move(lowered), flow};
    }

    bool is_zone_scratch_initializer(const Expr& expr) const {
        if (expr.kind != ExprKind::Call || !prelude_specials_available()) return false;
        std::string name = resolve_use_path(expr.name);
        return !unqualified_decl_shadows_prelude_name(expr.name, name) &&
               is_zone_scratch_function_name(name);
    }

    IrExprPtr make_mutable_zone_borrow(SourceLocation loc, const std::string& zone_name) {
        Expr borrow;
        borrow.kind = ExprKind::Borrow;
        borrow.loc = loc;
        borrow.name = zone_name;
        borrow.mutable_borrow = true;
        return check_expr(borrow);
    }

    void check_zone_scratch_var_decl(const Stmt& stmt, IrStmt& lowered) {
        const Expr& call = *stmt.binding.init;
        const ExprTypeArgs& type_args = expr_type_args(call);
        if (type_args.size() != 1) {
            fail(call.loc, "zone::scratch<T> expects exactly one type argument");
        }
        if (call.args.size() != 2) {
            fail(call.loc, "zone::scratch<T> expects a capacity and a value");
        }

        IrType allocated = resolve_executable_type(type_args[0]);
        if (allocated.qualifier != TypeQualifier::Value) {
            fail(type_args[0].loc, "zone::scratch<T> expects a value type, got " + type_name(allocated));
        }
        if (is_owner_type(allocated) || contains_borrow_type(allocated)) {
            fail(type_args[0].loc, "zone::scratch<T> cannot place ownership- or borrow-valued types yet");
        }

        std::uint64_t size_bytes = 0;
        std::uint64_t align_bytes = 0;
        if (!ari_layout_size_bytes(allocated, size_bytes) ||
            !ari_layout_align_bytes(allocated, align_bytes)) {
            fail(type_args[0].loc, "zone::scratch<T> does not support " + type_name(allocated));
        }
        if (size_bytes == 0) {
            fail(type_args[0].loc, "zone::scratch<T> requires a non-zero-sized type");
        }
        if (size_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
            align_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            fail(type_args[0].loc, "zone::scratch<T> layout is too large for i64");
        }

        IrType pointer_type = allocated;
        pointer_type.qualifier = TypeQualifier::Ptr;
        IrType declared = stmt.binding.has_type ? resolve_executable_type(stmt.binding.type) : pointer_type;

        std::size_t borrow_mark = temporary_borrow_mark();
        IrType i64 = i64_type(call.loc);
        IrExprPtr capacity = check_expr(*call.args[0]);
        coerce_expr_to_expected(*capacity, i64);
        require_assignable(call.args[0]->loc, i64, capacity->type);

        IrType own_zone = primitive_type(IrPrimitiveKind::Zone, "Zone", call.loc);
        own_zone.qualifier = TypeQualifier::Own;
        std::string zone_name = make_hidden_local("$scratch_" + stmt.binding.name + "_zone");
        std::vector<IrExprPtr> temp_args;
        temp_args.push_back(std::move(capacity));
        IrExprPtr temp_init = make_builtin_call(call.loc, "zone::temp", std::move(temp_args), own_zone);

        lowered.kind = IrStmtKind::Block;
        lowered.loc = stmt.loc;
        declare_local(call.loc, zone_name, own_zone, true);
        IrStmtPtr zone_decl = make_ir_var_decl(call.loc, zone_name, own_zone, std::move(temp_init), true);
        IrStmt* zone_decl_ptr = zone_decl.get();
        ir_stmt_statements(lowered).push_back(std::move(zone_decl));
        LocalInfo& zone_local = local_slot_by_name(zone_name);
        zone_local.ir_storage_type = &zone_decl_ptr->binding.type;
        zone_local.ir_init_expr = zone_decl_ptr->binding.init.get();
        zone_local.auto_destroy_zone = true;

        IrType mut_zone = primitive_type(IrPrimitiveKind::Zone, "Zone", call.loc);
        mut_zone.qualifier = TypeQualifier::MutRef;

        IrExprPtr zone_arg = make_mutable_zone_borrow(call.loc, zone_name);
        coerce_expr_to_expected(*zone_arg, mut_zone);
        require_assignable(call.loc, mut_zone, zone_arg->type);

        IrExprPtr value = check_expr(*call.args[1]);
        coerce_expr_to_expected(*value, allocated);
        require_assignable(call.args[1]->loc, allocated, value->type);

        std::vector<IrExprPtr> init_args;
        init_args.reserve(4);
        init_args.push_back(std::move(zone_arg));
        init_args.push_back(make_integer_literal(call.loc, i64_type(call.loc), size_bytes));
        init_args.push_back(make_integer_literal(call.loc, i64_type(call.loc), align_bytes));
        init_args.push_back(std::move(value));
        IrExprPtr init = make_ir_call_expr(call.loc, "zone::new", pointer_type, std::move(init_args));

        coerce_expr_to_expected(*init, declared);
        require_assignable(stmt.loc, declared, init->type);
        release_temporary_borrows(borrow_mark);

        declare_local(stmt.binding.loc, stmt.binding.name, declared, stmt.binding.mutable_binding);
        IrStmtPtr value_decl = make_ir_var_decl(
            stmt.loc,
            stmt.binding.name,
            declared,
            std::move(init),
            stmt.binding.mutable_binding
        );
        IrStmt* value_decl_ptr = value_decl.get();
        ir_stmt_statements(lowered).push_back(std::move(value_decl));

        LocalInfo& local = local_slot_by_name(stmt.binding.name);
        local.ir_storage_type = &value_decl_ptr->binding.type;
        local.ir_init_expr = value_decl_ptr->binding.init.get();
        set_zone_pointer_source_from_expr(local, *local.ir_init_expr);
    }

    VectorKnownLength vector_known_length_from_source_expr(const Expr& source) {
        return vector_known_length_from_source_tree(source, [this](const std::string& name) -> VectorKnownLength {
            const LocalInfo* source_local = find_local_slot(name);
            if (!source_local || !is_vector_storage_type(source_local->type)) return {};
            return local_vector_known_length(*source_local);
        });
    }

    VectorKnownLength vector_known_length_from_source_expr(const IrType& storage_type,
                                                           const Expr& source,
                                                           const IrExpr& lowered) {
        if (!is_vector_storage_type(storage_type)) return {};
        VectorKnownLength direct = vector_known_length_from_expr(storage_type, lowered);
        if (direct.known) return direct;
        return vector_known_length_from_source_expr(source);
    }

    void set_known_integer_value_from_expr(LocalInfo& local, const Expr& source, const IrExpr& expr) {
        clear_local_integer_known_value(local);
        if (local.mutable_binding || !is_value_integer_type(local.type)) {
            return;
        }
        StaticIntegerValue value;
        if (!try_fold_static_integer_value(expr, value) && !known_integer_capacity(source, value)) return;
        set_local_integer_known_value(local, value.value, value.negative);
    }

    void require_nullable_pointer_initializer(SourceLocation loc,
                                              const Binding& binding,
                                              const IrType& declared,
                                              const IrExpr& init) const {
        if (!binding.has_type || !binding.type.nullable) return;
        if (init.type.qualifier == TypeQualifier::Ptr) return;
        if (init.type.qualifier == TypeQualifier::Value &&
            init.type.primitive == IrPrimitiveKind::String &&
            (declared.primitive == IrPrimitiveKind::I8 ||
             declared.primitive == IrPrimitiveKind::U8 ||
             declared.primitive == IrPrimitiveKind::Void)) {
            return;
        }
        fail(loc,
             "nullable type '" + type_ref_key(binding.type) +
             "' is a raw pointer spelling; initialize it with null, a raw pointer, or a compatible string pointer value");
    }

    void check_var_decl(const Stmt& stmt, IrStmt& lowered) {
        if (stmt.binding.has_pattern) {
            check_pattern_var_decl(stmt, lowered);
            return;
        }
        if (is_zone_scratch_initializer(*stmt.binding.init)) {
            check_zone_scratch_var_decl(stmt, lowered);
            return;
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        IrType declared;
        IrExprPtr init;
        bool previous_zone_temp_init = allow_zone_temp_init_;
        allow_zone_temp_init_ = true;
        if (stmt.binding.has_type) {
            declared = resolve_executable_type(stmt.binding.type);
            init = check_expr_with_expected(*stmt.binding.init, declared);
        } else {
            init = check_expr(*stmt.binding.init);
            declared = init->type;
        }
        allow_zone_temp_init_ = previous_zone_temp_init;
        if (contains_zone_temp_call(*init) && !is_zone_temp_call(*init)) {
            fail(stmt.binding.init->loc, "zone::temp can only initialize a local temporary zone binding");
        }
        specialize_vector_storage_from_init(declared, *init);
        coerce_expr_to_expected(*init, declared);
        require_nullable_pointer_initializer(stmt.loc, stmt.binding, declared, *init);
        require_assignable(stmt.loc, declared, init->type);
        VectorKnownLength init_vector_length =
            vector_known_length_from_source_expr(declared, *stmt.binding.init, *init);
        bool borrow_binding = is_borrow_type(declared);
        if (borrow_binding && !borrow_result_source(*init)) {
            fail(stmt.loc, "borrow bindings must be initialized from ref, ref mut, or compatible borrow control-flow results");
        }
        std::string generic_origin = stmt.binding.has_type
            ? generic_origin_from_type_ref(stmt.binding.type)
            : generic_origin_from_expr(*stmt.binding.init);
        declare_local(stmt.binding.loc, stmt.binding.name, declared, stmt.binding.mutable_binding);
        if (borrow_binding) {
            promote_temporary_borrow_to_named(stmt.loc, *init, stmt.binding.name);
        } else if (contains_borrow_type(declared)) {
            promote_temporary_borrows_to_aggregate(borrow_mark, stmt.binding.name);
        } else {
            release_temporary_borrows(borrow_mark);
        }

        lowered.binding.name = stmt.binding.name;
        lowered.binding.loc = stmt.binding.loc;
        lowered.binding.mutable_binding = stmt.binding.mutable_binding;
        lowered.binding.type = declared;
        lowered.binding.init = std::move(init);
        LocalInfo& local = local_slot_by_name(stmt.binding.name);
        local.ir_storage_type = &lowered.binding.type;
        local.ir_init_expr = lowered.binding.init.get();
        local.generic_origin = std::move(generic_origin);
        local.auto_destroy_zone = is_zone_temp_call(*local.ir_init_expr);
        set_local_vector_known_length(local, init_vector_length);
        set_known_integer_value_from_expr(local, *stmt.binding.init, *local.ir_init_expr);
        set_zone_pointer_source_from_expr(local, *local.ir_init_expr);
    }

    void check_pattern_var_decl(const Stmt& stmt, IrStmt& lowered) {
        std::size_t borrow_mark = temporary_borrow_mark();
        IrType declared;
        IrExprPtr init;
        if (stmt.binding.has_type) {
            declared = resolve_executable_type(stmt.binding.type);
            init = check_expr_with_expected(*stmt.binding.init, declared);
        } else {
            init = check_expr(*stmt.binding.init);
            declared = init->type;
        }
        specialize_vector_storage_from_init(declared, *init);
        coerce_expr_to_expected(*init, declared);
        require_nullable_pointer_initializer(stmt.loc, stmt.binding, declared, *init);
        require_assignable(stmt.loc, declared, init->type);
        if (is_borrow_type(declared)) {
            fail(stmt.loc, "borrow pattern bindings are not supported yet; pass ref values directly to calls");
        }
        if (contains_borrow_type(declared)) {
            fail(stmt.loc, "borrow pattern bindings are not supported yet; pass ref values directly to calls");
        }
        release_temporary_borrows(borrow_mark);

        lowered.kind = IrStmtKind::Block;
        const Pattern& binding_pattern = expanded_pattern(stmt.binding.pattern);
        if (binding_pattern.kind == PatternKind::Wildcard && !is_aggregate_type(declared)) {
            if (is_owner_type(declared)) {
                fail(stmt.loc, "owning expression result must be bound, returned, passed, or dropped");
            }
            auto discard = std::make_unique<IrStmt>();
            discard->kind = IrStmtKind::ExprStmt;
            discard->loc = stmt.loc;
            discard->expr = std::move(init);
            ir_stmt_statements(lowered).push_back(std::move(discard));
            return;
        }

        if (is_owner_type(declared)) {
            fail(stmt.loc, "owning pattern bindings are planned after ownership through aggregates is implemented");
        }

        std::string source_name = make_hidden_local("$pattern");
        declare_local(stmt.loc, source_name, declared, false);
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(stmt.loc, source_name, declared, std::move(init), false));
        lower_binding_pattern_from_local(
            binding_pattern,
            source_name,
            declared,
            stmt.binding.mutable_binding,
            ir_stmt_statements(lowered)
        );
    }

    void widen_vector_storage(LocalInfo& local, std::uint64_t capacity) const {
        if (!is_vector_storage_type(local.type) || capacity <= local.type.array_size) return;
        widen_vector_storage_type(local.type, capacity);
        if (local.ir_storage_type) {
            widen_vector_storage_type(*local.ir_storage_type, capacity);
        }
        if (local.ir_init_expr) {
            widen_vector_storage_literal(*local.ir_init_expr, capacity);
        }
    }

    void widen_vector_storage_for_push(LocalInfo& local) const {
        if (!is_vector_storage_type(local.type)) return;
        VectorKnownLength current = local_vector_known_length(local);
        widen_vector_storage(local, vector_required_capacity_for_append(local.type, current));
        set_local_vector_known_length(local, vector_known_length_after_append(current));
    }

    void widen_vector_storage_for_assignment(LocalInfo& local, const IrExpr& value) const {
        if (!is_vector_storage_type(local.type)) return;
        widen_vector_storage(local, vector_storage_capacity_from_expr(value));
    }

    static bool is_unsized_vector_storage_type(const IrType* type) {
        return type && is_vector_storage_type(*type) && type->array_size == 0;
    }

    static void widen_vector_result_storage(IrType& type, const IrExpr& value) {
        if (!is_vector_storage_type(type)) return;
        widen_vector_storage_type(type, vector_storage_capacity_from_expr(value));
    }

    std::uint64_t vector_storage_capacity_from_source_expr(const Expr& expr) {
        return vector_storage_capacity_from_source_tree(expr, [this](const std::string& name) {
            const LocalInfo* local = find_local_slot(name);
            return local && is_vector_storage_type(local->type) ? local->type.array_size : 0;
        });
    }

    void widen_vector_result_storage_from_source(IrType& type, const Expr& expr) {
        if (!is_vector_storage_type(type)) return;
        widen_vector_storage_type(type, vector_storage_capacity_from_source_expr(expr));
    }

    IrType sized_control_flow_expected_type(const IrType& expected,
                                            const std::vector<const Expr*>& values) {
        IrType sized = expected;
        if (!is_vector_storage_type(sized) || sized.array_size != 0) return sized;
        for (const Expr* value : values) {
            if (value) widen_vector_result_storage_from_source(sized, *value);
        }
        return sized;
    }

    std::vector<const Expr*> match_arm_value_exprs(const Expr& expr) const {
        std::vector<const Expr*> values;
        values.reserve(expr_match_arms(expr).size());
        for (const auto& arm : expr_match_arms(expr)) values.push_back(arm.value.get());
        return values;
    }

    bool known_integer_capacity(const Expr& expr, StaticIntegerValue& out) {
        if (expr.kind == ExprKind::Integer) {
            out.value = expr.int_value;
            out.negative = expr.int_negative;
            return true;
        }
        if (expr.kind == ExprKind::Name) {
            const LocalInfo* local = find_local_slot(expr.name);
            if (local) {
                if (!local->integer_value_known) return false;
                out.value = local->integer_known_value;
                out.negative = local->integer_known_negative;
                return true;
            }
            ConstantValue constant;
            if (!resolve_constant_value(expr.loc, expr.name, constant) ||
                constant.kind != ConstantValueKind::Integer ||
                !is_value_integer_type(constant.type)) {
                return false;
            }
            out.value = constant.int_value;
            out.negative = constant.int_negative;
            return true;
        }
        if (expr.kind == ExprKind::Unary && expr_operand(expr)) {
            StaticIntegerValue operand;
            return known_integer_capacity(*expr_operand(expr), operand) &&
                   fold_static_integer_unary(expr.op, operand, out);
        }
        if (expr.kind == ExprKind::Binary && expr_left(expr) && expr_right(expr)) {
            StaticIntegerValue left;
            StaticIntegerValue right;
            return known_integer_capacity(*expr_left(expr), left) &&
                   known_integer_capacity(*expr_right(expr), right) &&
                   fold_static_integer_binary(expr.op, left, right, out);
        }
        return false;
    }

    const StaticIntegerValue* known_integer_argument_value(const Expr& source,
                                                           const IrExpr& lowered,
                                                           StaticIntegerValue& source_known,
                                                           StaticIntegerValue& folded_known) {
        if (known_integer_capacity(source, source_known)) return &source_known;
        if (try_fold_static_integer_value(lowered, folded_known)) return &folded_known;
        return nullptr;
    }

    void require_local_vec_static_index_argument(const Expr& source,
                                                 const IrExpr& lowered,
                                                 LocalVecMethod method,
                                                 const char* role,
                                                 VectorKnownLength length,
                                                 bool allow_end = false) {
        StaticIntegerValue source_known;
        StaticIntegerValue folded_known;
        const StaticIntegerValue* known =
            known_integer_argument_value(source, lowered, source_known, folded_known);
        if (known) {
            require_local_vec_static_index_in_known_bounds(source.loc, method, role, *known, length, allow_end);
        }
    }

    static bool is_runtime_sequence_pattern_subject(const IrType& type) {
        return is_vector_storage_type(type) || is_prelude_slice_type(type);
    }

    static std::string runtime_sequence_pattern_subject_name(const IrType& type) {
        if (is_vector_storage_type(type)) return "Vec[T]";
        if (is_prelude_slice_type(type)) return "Slice[T]";
        return type_name(type);
    }

    static const IrType& runtime_sequence_element_type(SourceLocation loc, const IrType& source_type) {
        if (!is_runtime_sequence_pattern_subject(source_type) || source_type.args.size() != 1) {
            fail(loc, "runtime sequence pattern requires a Vec[T] or Slice[T] value");
        }
        return source_type.args[0];
    }

    bool runtime_sequence_array_pattern_is_irrefutable(const Pattern& pattern) const {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) return runtime_sequence_array_pattern_is_irrefutable(effective_pattern);
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::Binding:
                return true;
            case PatternKind::Alias:
                return pattern.alias_pattern &&
                       runtime_sequence_array_pattern_is_irrefutable(*pattern.alias_pattern);
            case PatternKind::Or:
                for (const auto& alternative : pattern.alternatives) {
                    if (runtime_sequence_array_pattern_is_irrefutable(alternative)) return true;
                }
                return false;
            case PatternKind::Array:
                return pattern.has_rest && pattern.elements.empty();
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
            case PatternKind::EnumCase:
            case PatternKind::Tuple:
            case PatternKind::Struct:
                return false;
        }
        return false;
    }

    static IrExprPtr make_i64_binary_expr(SourceLocation loc,
                                          IrBinaryOp op,
                                          IrExprPtr left,
                                          IrExprPtr right) {
        auto expr = std::make_unique<IrExpr>();
        expr->kind = IrExprKind::Binary;
        expr->loc = loc;
        expr->op = op;
        expr->type = i64_type(loc);
        set_ir_expr_left(*expr, std::move(left));
        set_ir_expr_right(*expr, std::move(right));
        return expr;
    }

    IrExprPtr make_runtime_sequence_len_expr(SourceLocation loc,
                                             const std::string& source_name,
                                             const IrType& source_type) const {
        return make_collection_len_expr(loc, make_local_lvalue_expr(loc, source_name, source_type));
    }

    IrExprPtr make_runtime_sequence_pattern_index_expr(SourceLocation loc,
                                                       const Pattern& pattern,
                                                       std::size_t pattern_index,
                                                       const std::string& source_name,
                                                       const IrType& source_type) const {
        if (!pattern.has_rest || pattern_index < pattern.rest_index) {
            return make_integer_literal(loc, i64_type(loc), pattern_index);
        }

        std::size_t suffix_count = pattern.elements.size() - pattern.rest_index;
        std::size_t suffix_offset = pattern_index - pattern.rest_index;
        IrExprPtr index = make_i64_binary_expr(
            loc,
            IrBinaryOp::Sub,
            make_runtime_sequence_len_expr(loc, source_name, source_type),
            make_integer_literal(loc, i64_type(loc), suffix_count)
        );
        if (suffix_offset == 0) return index;
        return make_i64_binary_expr(
            loc,
            IrBinaryOp::Add,
            std::move(index),
            make_integer_literal(loc, i64_type(loc), suffix_offset)
        );
    }

    void require_runtime_sequence_element_materializable(SourceLocation loc,
                                                         const IrType& source_type,
                                                         const IrType& element_type,
                                                         const std::string& operation) const {
        if (is_prelude_slice_type(source_type)) {
            require_slice_element_materializable(loc, element_type, operation);
            return;
        }
        if (is_owner_type(element_type) || contains_borrow_type(element_type)) {
            fail(loc, operation + " cannot copy ownership- or borrow-valued Vec elements yet");
        }
    }

    IrExprPtr make_runtime_sequence_index_expr(SourceLocation loc,
                                               const std::string& source_name,
                                               const IrType& source_type,
                                               IrExprPtr index,
                                               const std::string& operation) const {
        const IrType& element_type = runtime_sequence_element_type(loc, source_type);
        require_runtime_sequence_element_materializable(loc, source_type, element_type, operation);
        return make_ir_index_expr(
            loc,
            make_local_lvalue_expr(loc, source_name, source_type),
            std::move(index)
        );
    }

    IrExprPtr make_runtime_sequence_pattern_element_expr(SourceLocation loc,
                                                         const Pattern& pattern,
                                                         std::size_t pattern_index,
                                                         const std::string& source_name,
                                                         const IrType& source_type,
                                                         const std::string& operation) const {
        return make_runtime_sequence_index_expr(
            loc,
            source_name,
            source_type,
            make_runtime_sequence_pattern_index_expr(loc, pattern, pattern_index, source_name, source_type),
            operation
        );
    }

    IrExprPtr make_runtime_sequence_rest_slice_expr(SourceLocation loc,
                                                    const Pattern& pattern,
                                                    const std::string& source_name,
                                                    const IrType& source_type) const {
        const IrType& element_type = runtime_sequence_element_type(loc, source_type);
        require_runtime_sequence_element_materializable(
            loc,
            source_type,
            element_type,
            "runtime sequence rest binding"
        );

        const std::size_t prefix_count = pattern.rest_index;
        const std::size_t suffix_count = pattern.elements.size() - pattern.rest_index;
        IrExprPtr data;
        if (is_prelude_slice_type(source_type)) {
            data = make_tuple_index_expr(
                loc,
                make_local_lvalue_expr(loc, source_name, source_type),
                0
            );
        } else {
            data = make_slice_data_pointer_expr(
                loc,
                make_vec_storage_lvalue_expr(loc, source_name, source_type),
                element_type
            );
        }
        if (prefix_count != 0) {
            data = make_pointer_add_expr(
                loc,
                std::move(data),
                make_integer_literal(loc, i64_type(loc), prefix_count)
            );
        }

        IrExprPtr length = make_runtime_sequence_len_expr(loc, source_name, source_type);
        const std::size_t skipped_count = prefix_count + suffix_count;
        if (skipped_count != 0) {
            length = make_i64_binary_expr(
                loc,
                IrBinaryOp::Sub,
                std::move(length),
                make_integer_literal(loc, i64_type(loc), skipped_count)
            );
        }
        return make_slice_view_expr(
            loc,
            std::move(data),
            std::move(length),
            make_prelude_slice_type(loc, element_type)
        );
    }

    IrExprPtr lower_runtime_sequence_pattern_length_condition(const Pattern& pattern,
                                                              const std::string& source_name,
                                                              const IrType& source_type) const {
        if (runtime_sequence_array_pattern_is_irrefutable(pattern)) return nullptr;
        const std::uint64_t required = static_cast<std::uint64_t>(pattern.elements.size());
        return make_bool_binary_expr(
            pattern.loc,
            pattern.has_rest ? IrBinaryOp::Ge : IrBinaryOp::Eq,
            make_runtime_sequence_len_expr(pattern.loc, source_name, source_type),
            make_integer_literal(pattern.loc, i64_type(pattern.loc), required)
        );
    }

    void materialize_runtime_sequence_element(SourceLocation loc,
                                              const Pattern& sequence_pattern,
                                              std::size_t pattern_index,
                                              const std::string& source_name,
                                              const IrType& source_type,
                                              const std::string& local_name,
                                              std::vector<IrStmtPtr>& statements,
                                              const std::string& operation) {
        const IrType& element_type = runtime_sequence_element_type(loc, source_type);
        declare_local(loc, local_name, element_type, false);
        statements.push_back(make_ir_var_decl(
            loc,
            local_name,
            element_type,
            make_runtime_sequence_pattern_element_expr(
                loc,
                sequence_pattern,
                pattern_index,
                source_name,
                source_type,
                operation
            ),
            false
        ));
    }

    IrExprPtr lower_runtime_sequence_element_match_condition(const Pattern& pattern,
                                                             const Pattern& sequence_pattern,
                                                             std::size_t pattern_index,
                                                             const std::string& source_name,
                                                             const IrType& source_type,
                                                             std::vector<IrStmtPtr>& prelude) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_runtime_sequence_element_match_condition(
                effective_pattern, sequence_pattern, pattern_index, source_name, source_type, prelude);
        }
        const IrType& element_type = runtime_sequence_element_type(pattern.loc, source_type);
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::Binding:
                return nullptr;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                return lower_runtime_sequence_element_match_condition(
                    *pattern.alias_pattern, sequence_pattern, pattern_index, source_name, source_type, prelude);
            case PatternKind::IntegerLiteral:
                if (!is_value_integer_type(element_type)) {
                    fail(pattern.loc, "integer runtime sequence patterns require integer elements");
                }
                return make_bool_binary_expr(
                    pattern.loc,
                    IrBinaryOp::Eq,
                    make_runtime_sequence_pattern_element_expr(
                        pattern.loc, sequence_pattern, pattern_index, source_name, source_type, "runtime sequence pattern"),
                    make_pattern_integer_expr(pattern, element_type)
                );
            case PatternKind::BoolLiteral:
                if (element_type.qualifier != TypeQualifier::Value ||
                    element_type.primitive != IrPrimitiveKind::Bool) {
                    fail(pattern.loc, "bool runtime sequence patterns require bool elements");
                }
                return make_bool_binary_expr(
                    pattern.loc,
                    IrBinaryOp::Eq,
                    make_runtime_sequence_pattern_element_expr(
                        pattern.loc, sequence_pattern, pattern_index, source_name, source_type, "runtime sequence pattern"),
                    make_bool_literal_expr(pattern.loc, pattern.bool_value)
                );
            case PatternKind::Range: {
                if (!is_value_integer_type(element_type)) {
                    fail(pattern.loc, "range runtime sequence patterns require integer elements");
                }
                if (!range_start_le_end(pattern, element_type)) {
                    fail(pattern.loc, "range pattern start must be <= end");
                }
                IrExprPtr lower = make_bool_binary_expr(
                    pattern.loc,
                    IrBinaryOp::Ge,
                    make_runtime_sequence_pattern_element_expr(
                        pattern.loc, sequence_pattern, pattern_index, source_name, source_type, "runtime sequence pattern"),
                    make_pattern_range_endpoint_expr(
                        pattern.loc,
                        pattern.int_value,
                        pattern.int_negative,
                        pattern.literal_suffix,
                        element_type
                    )
                );
                IrExprPtr upper = make_bool_binary_expr(
                    pattern.loc,
                    pattern.range_inclusive ? IrBinaryOp::Le : IrBinaryOp::Lt,
                    make_runtime_sequence_pattern_element_expr(
                        pattern.loc, sequence_pattern, pattern_index, source_name, source_type, "runtime sequence pattern"),
                    make_pattern_range_endpoint_expr(
                        pattern.loc,
                        pattern.range_end_value,
                        pattern.range_end_negative,
                        pattern.range_end_suffix,
                        element_type
                    )
                );
                return combine_tuple_match_conditions(
                    pattern.loc, IrBinaryOp::LogicalAnd, std::move(lower), std::move(upper));
            }
            case PatternKind::Or: {
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                IrExprPtr condition;
                for (const auto& alternative : pattern.alternatives) {
                    IrExprPtr alternative_condition = lower_runtime_sequence_element_match_condition(
                        alternative, sequence_pattern, pattern_index, source_name, source_type, prelude);
                    if (!alternative_condition) return nullptr;
                    condition = combine_tuple_match_conditions(
                        pattern.loc,
                        IrBinaryOp::LogicalOr,
                        std::move(condition),
                        std::move(alternative_condition)
                    );
                }
                return condition;
            }
            case PatternKind::EnumCase: {
                ConstantValue constant_pattern;
                if (try_constant_pattern_value(pattern, constant_pattern)) {
                    if (element_type.qualifier == TypeQualifier::Value &&
                        element_type.primitive == IrPrimitiveKind::Bool) {
                        if (!constant_pattern.is_bool) {
                            fail(pattern.loc, "bool runtime sequence constant pattern must have type bool");
                        }
                    } else if (is_value_integer_type(element_type)) {
                        if (constant_pattern.is_bool || !is_value_integer_type(constant_pattern.type)) {
                            fail(pattern.loc, "integer runtime sequence constant pattern must have an integer type");
                        }
                        require_assignable(pattern.loc, element_type, constant_pattern.type);
                    } else {
                        fail(pattern.loc, "constant runtime sequence patterns require integer or bool elements");
                    }
                    return make_bool_binary_expr(
                        pattern.loc,
                        IrBinaryOp::Eq,
                        make_runtime_sequence_pattern_element_expr(
                            pattern.loc, sequence_pattern, pattern_index, source_name, source_type, "runtime sequence pattern"),
                        make_constant_expr(pattern.loc, constant_pattern)
                    );
                }
                if (element_type.primitive != IrPrimitiveKind::Struct) {
                    fail(pattern.loc, "tuple-struct runtime sequence element patterns require tuple-struct elements");
                }
                std::string nested_name = make_hidden_local("$match_seq");
                materialize_runtime_sequence_element(
                    pattern.loc,
                    sequence_pattern,
                    pattern_index,
                    source_name,
                    source_type,
                    nested_name,
                    prelude,
                    "runtime sequence pattern"
                );
                return lower_struct_match_pattern_condition(pattern, nested_name, element_type, prelude);
            }
            case PatternKind::Tuple: {
                if (element_type.primitive != IrPrimitiveKind::Tuple) {
                    fail(pattern.loc, "nested tuple runtime sequence pattern requires tuple elements, got " +
                                      type_name(element_type));
                }
                std::string nested_name = make_hidden_local("$match_seq");
                materialize_runtime_sequence_element(
                    pattern.loc, sequence_pattern, pattern_index, source_name, source_type,
                    nested_name, prelude, "runtime sequence pattern");
                return lower_tuple_match_pattern_condition(pattern, nested_name, element_type, prelude);
            }
            case PatternKind::Array: {
                if (element_type.primitive != IrPrimitiveKind::Array &&
                    !is_runtime_sequence_pattern_subject(element_type)) {
                    fail(pattern.loc, "nested array runtime sequence pattern requires array, Vec, or Slice elements, got " +
                                      type_name(element_type));
                }
                std::string nested_name = make_hidden_local("$match_seq");
                materialize_runtime_sequence_element(
                    pattern.loc, sequence_pattern, pattern_index, source_name, source_type,
                    nested_name, prelude, "runtime sequence pattern");
                if (is_runtime_sequence_pattern_subject(element_type)) {
                    return lower_runtime_sequence_match_pattern_condition(pattern, nested_name, element_type, prelude);
                }
                return lower_tuple_match_pattern_condition(pattern, nested_name, element_type, prelude);
            }
            case PatternKind::Struct: {
                if (element_type.primitive != IrPrimitiveKind::Struct) {
                    fail(pattern.loc, "struct runtime sequence element patterns require struct elements");
                }
                std::string nested_name = make_hidden_local("$match_seq");
                materialize_runtime_sequence_element(
                    pattern.loc, sequence_pattern, pattern_index, source_name, source_type,
                    nested_name, prelude, "runtime sequence pattern");
                return lower_struct_match_pattern_condition(pattern, nested_name, element_type, prelude);
            }
        }
        fail(pattern.loc, "unsupported runtime sequence element pattern");
    }

    IrExprPtr lower_runtime_sequence_match_pattern_condition(const Pattern& pattern,
                                                             const std::string& source_name,
                                                             const IrType& source_type,
                                                             std::vector<IrStmtPtr>& prelude) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_runtime_sequence_match_pattern_condition(
                effective_pattern, source_name, source_type, prelude);
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::Binding:
                return nullptr;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                return lower_runtime_sequence_match_pattern_condition(*pattern.alias_pattern, source_name, source_type, prelude);
            case PatternKind::Or: {
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                IrExprPtr condition;
                for (const auto& alternative : pattern.alternatives) {
                    IrExprPtr alternative_condition =
                        lower_runtime_sequence_match_pattern_condition(alternative, source_name, source_type, prelude);
                    if (!alternative_condition) return nullptr;
                    condition = combine_tuple_match_conditions(
                        pattern.loc,
                        IrBinaryOp::LogicalOr,
                        std::move(condition),
                        std::move(alternative_condition)
                    );
                }
                return condition;
            }
            case PatternKind::Array:
                break;
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
            case PatternKind::EnumCase:
            case PatternKind::Tuple:
            case PatternKind::Struct:
                fail(pattern.loc, runtime_sequence_pattern_subject_name(source_type) +
                                  " match patterns must use [..], _, or a binding");
        }

        IrExprPtr condition = lower_runtime_sequence_pattern_length_condition(pattern, source_name, source_type);
        for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
            IrExprPtr element_condition = lower_runtime_sequence_element_match_condition(
                pattern.elements[i],
                pattern,
                i,
                source_name,
                source_type,
                prelude
            );
            condition = combine_tuple_match_conditions(
                pattern.elements[i].loc,
                IrBinaryOp::LogicalAnd,
                std::move(condition),
                std::move(element_condition)
            );
        }
        return condition;
    }

    void lower_runtime_sequence_match_pattern_bindings_from_local(const Pattern& pattern,
                                                                  const std::string& source_name,
                                                                  const IrType& source_type,
                                                                  std::vector<IrStmtPtr>& statements,
                                                                  bool mutable_binding = false) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_runtime_sequence_match_pattern_bindings_from_local(
                effective_pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
            case PatternKind::EnumCase:
            case PatternKind::Tuple:
            case PatternKind::Struct:
                return;
            case PatternKind::Binding:
                lower_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                return;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                declare_local(pattern.loc, pattern.alias_name, source_type, mutable_binding);
                statements.push_back(make_ir_var_decl(
                    pattern.loc,
                    pattern.alias_name,
                    source_type,
                    make_local_lvalue_expr(pattern.loc, source_name, source_type),
                    mutable_binding
                ));
                lower_product_match_pattern_bindings_from_local(
                    *pattern.alias_pattern,
                    source_name,
                    source_type,
                    statements,
                    mutable_binding
                );
                return;
            case PatternKind::Or:
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                return;
            case PatternKind::Array:
                break;
        }

        const IrType& element_type = runtime_sequence_element_type(pattern.loc, source_type);
        if (!pattern.rest_alias_name.empty()) {
            IrType slice_type = make_prelude_slice_type(pattern.rest_alias_loc, element_type);
            declare_local(pattern.rest_alias_loc, pattern.rest_alias_name, slice_type, mutable_binding);
            statements.push_back(make_ir_var_decl(
                pattern.rest_alias_loc,
                pattern.rest_alias_name,
                slice_type,
                make_runtime_sequence_rest_slice_expr(
                    pattern.rest_alias_loc,
                    pattern,
                    source_name,
                    source_type
                ),
                mutable_binding
            ));
        }
        for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
            const Pattern& item = pattern.elements[i];
            if (item.kind == PatternKind::Wildcard) continue;
            lower_tuple_match_value_bindings(
                item,
                element_type,
                make_runtime_sequence_pattern_element_expr(
                    item.loc,
                    pattern,
                    i,
                    source_name,
                    source_type,
                    "runtime sequence pattern binding"
                ),
                statements,
                mutable_binding
            );
        }
    }

    void lower_binding_pattern_from_local(
        const Pattern& pattern,
        const std::string& source_name,
        const IrType& source_type,
        bool mutable_binding,
        std::vector<IrStmtPtr>& statements
    ) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_binding_pattern_from_local(effective_pattern, source_name, source_type, mutable_binding, statements);
            return;
        }
        if (is_runtime_sequence_pattern_subject(source_type) && pattern_contains_array_pattern(pattern)) {
            std::vector<Pattern> alternatives = expand_or_pattern_alternatives(pattern);
            std::set<std::string> reusable_names;
            if (pattern_contains_or(pattern)) {
                reusable_names = require_same_or_pattern_bindings(pattern.loc, alternatives, source_type);
            }

            bool has_irrefutable_alternative = false;
            std::vector<TupleCheckedStmtArm> checked_arms;
            checked_arms.reserve(alternatives.size() + 1);
            for (std::size_t i = 0; i < alternatives.size(); ++i) {
                const Pattern& alternative = alternatives[i];
                TupleCheckedStmtArm arm;
                arm.loc = alternative.loc;
                std::vector<IrStmtPtr> condition_prelude;
                arm.condition = lower_runtime_sequence_match_pattern_condition(
                    alternative,
                    source_name,
                    source_type,
                    condition_prelude
                );
                if (!arm.condition) has_irrefutable_alternative = true;
                for (auto& statement : condition_prelude) {
                    statements.push_back(std::move(statement));
                }

                local_scopes_.set_reusable_pattern_bindings(i == 0 ? std::set<std::string>{} : reusable_names);
                lower_runtime_sequence_match_pattern_bindings_from_local(
                    alternative,
                    source_name,
                    source_type,
                    arm.body,
                    mutable_binding
                );
                local_scopes_.clear_reusable_pattern_bindings();
                checked_arms.push_back(std::move(arm));
            }

            if (!has_irrefutable_alternative) {
                TupleCheckedStmtArm failure;
                failure.loc = pattern.loc;
                failure.body.push_back(make_panic_stmt(pattern.loc));
                checked_arms.push_back(std::move(failure));
            }

            std::vector<IrStmtPtr> lowered = build_tuple_match_if_chain(checked_arms);
            for (auto& statement : lowered) {
                statements.push_back(std::move(statement));
            }
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
                return;
            case PatternKind::Binding:
                if (is_borrow_type(source_type)) {
                    fail(pattern.loc, "borrow pattern bindings are not supported yet; pass ref values directly to calls");
                }
                if (is_owner_type(source_type)) {
                    fail(pattern.loc, "owning pattern bindings are planned after ownership through aggregates is implemented");
                }
                declare_local(pattern.loc, pattern.payload_name, source_type, mutable_binding);
                statements.push_back(make_ir_var_decl(
                    pattern.loc,
                    pattern.payload_name,
                    source_type,
                    make_local_lvalue_expr(pattern.loc, source_name, source_type),
                    mutable_binding
                ));
                return;
            case PatternKind::Tuple:
                if (!is_aggregate_type(source_type)) {
                    lower_tuple_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                    return;
                }
                lower_refutable_product_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                return;
            case PatternKind::Array:
                if (!is_aggregate_type(source_type)) {
                    lower_tuple_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                    return;
                }
                lower_refutable_product_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                return;
            case PatternKind::Struct:
                if (!is_aggregate_type(source_type)) {
                    lower_struct_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                    return;
                }
                lower_refutable_product_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                return;
            case PatternKind::EnumCase:
            case PatternKind::Alias:
            case PatternKind::Or:
                if (is_aggregate_type(source_type)) {
                    lower_refutable_product_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                    return;
                }
                lower_refutable_enum_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                return;
            default:
                fail(pattern.loc, "let/var enum pattern bindings are planned but are not supported yet");
        }
    }

    void lower_refutable_enum_binding_pattern_from_local(
        const Pattern& pattern,
        const std::string& source_name,
        const IrType& source_type,
        bool mutable_binding,
        std::vector<IrStmtPtr>& statements
    ) {
        IrExprPtr source = make_local_lvalue_expr(pattern.loc, source_name, source_type);
        const EnumInfo& enum_info = require_enum_match_value(pattern.loc, *source);
        EnumMatchCoverage coverage;
        std::vector<IrMatchArm> success_arms = lower_match_arm_patterns(pattern, enum_info, source_type, coverage);
        if (success_arms.empty()) {
            fail(pattern.loc, "refutable enum binding pattern did not lower to a match arm");
        }
        declare_refutable_binding_locals(success_arms.front(), mutable_binding);

        IrMatchArm failure;
        failure.loc = pattern.loc;
        failure.wildcard = true;
        failure.body.push_back(make_panic_stmt(pattern.loc));

        auto match = std::make_unique<IrStmt>();
        match->kind = IrStmtKind::Match;
        match->loc = pattern.loc;
        match->match_value = std::move(source);
        IrStmtMatchArms& match_arms = ensure_ir_stmt_match_arms(*match);
        for (auto& success : success_arms) {
            match_arms.push_back(std::move(success));
        }
        match_arms.push_back(std::move(failure));
        statements.push_back(std::move(match));
    }

    void declare_refutable_binding_locals(const IrMatchArm& arm, bool mutable_binding) {
        if (arm.has_value_binding) {
            if (is_owner_type(arm.value_type)) {
                fail(arm.loc, "owning pattern bindings are planned after ownership through aggregates is implemented");
            }
            declare_local(arm.loc, arm.value_name, arm.value_type, mutable_binding);
        }
        if (!arm.payload_bindings.empty()) {
            for (const auto& binding : arm.payload_bindings) {
                if (is_owner_type(binding.type)) {
                    fail(arm.loc, "owning pattern bindings are planned after ownership through aggregates is implemented");
                }
                declare_local(arm.loc, binding.name, binding.type, mutable_binding);
            }
        } else if (arm.has_payload_binding) {
            if (is_owner_type(arm.payload_type)) {
                fail(arm.loc, "owning pattern bindings are planned after ownership through aggregates is implemented");
            }
            declare_local(arm.loc, arm.payload_name, arm.payload_type, mutable_binding);
        }
    }

    void validate_product_binding_pattern_shape(const Pattern& pattern, const IrType& source_type) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            validate_product_binding_pattern_shape(effective_pattern, source_type);
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::Binding:
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
                return;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                validate_product_binding_pattern_shape(*pattern.alias_pattern, source_type);
                return;
            case PatternKind::Or:
                for (const auto& alternative : pattern.alternatives) {
                    validate_product_binding_pattern_shape(alternative, source_type);
                }
                return;
            case PatternKind::Tuple:
            case PatternKind::Array: {
                if (pattern.kind == PatternKind::Array && is_runtime_sequence_pattern_subject(source_type)) {
                    const IrType& element_type = runtime_sequence_element_type(pattern.loc, source_type);
                    for (const auto& element : pattern.elements) {
                        validate_product_binding_pattern_shape(element, element_type);
                    }
                    return;
                }
                bool array_pattern = pattern.kind == PatternKind::Array;
                IrPrimitiveKind expected = array_pattern ? IrPrimitiveKind::Array : IrPrimitiveKind::Tuple;
                const char* pattern_name = array_pattern ? "array" : "tuple";
                if (!pattern.rest_alias_name.empty()) {
                    fail(pattern.rest_alias_loc,
                         std::string(pattern_name) +
                             " rest bindings currently require a Vec[T] or Slice[T] value");
                }
                if (source_type.primitive != expected) {
                    fail(pattern.loc, std::string(pattern_name) + " binding pattern requires a " +
                                      pattern_name + " value, got " + type_name(source_type));
                }
                const std::vector<IrType>& fields = aggregate_field_types(source_type);
                if (pattern.has_rest) {
                    if (pattern.elements.size() > fields.size()) {
                        fail(pattern.loc,
                             std::string(pattern_name) + " binding pattern has " + std::to_string(pattern.elements.size()) +
                             " non-rest elements but value has " + std::to_string(fields.size()));
                    }
                } else if (pattern.elements.size() != fields.size()) {
                    fail(pattern.loc,
                         std::string(pattern_name) + " binding pattern has " + std::to_string(pattern.elements.size()) +
                         " elements but value has " + std::to_string(fields.size()));
                }
                for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
                    std::size_t field_index = tuple_pattern_field_index(pattern, fields.size(), i);
                    validate_product_binding_pattern_shape(pattern.elements[i], fields[field_index]);
                }
                return;
            }
            case PatternKind::Struct: {
                if (source_type.primitive != IrPrimitiveKind::Struct) {
                    fail(pattern.loc, "struct binding pattern requires a struct value, got " + type_name(source_type));
                }
                std::string struct_name = resolve_struct_type_name(pattern.case_name);
                auto struct_found = structs_.find(struct_name);
                if (struct_found == structs_.end()) {
                    fail(pattern.loc, "unknown struct '" + pattern.case_name + "' in binding pattern");
                }
                require_struct_access(pattern.loc, struct_found->second);
                if (struct_name != source_type.name) {
                    fail(pattern.loc,
                         "struct binding pattern type '" + struct_name + "' does not match value type " + type_name(source_type));
                }
                if (pattern.field_names.size() != pattern.elements.size()) {
                    throw CompileError("internal error: struct binding pattern field/value arity mismatch");
                }
                std::set<std::string> seen_fields;
                for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
                    const std::string& field_name = pattern.field_names[i];
                    if (!seen_fields.insert(field_name).second) {
                        fail(pattern.elements[i].loc, "duplicate field '" + field_name + "' in struct binding pattern");
                    }
                    std::size_t field_index = struct_field_index(pattern.elements[i].loc, source_type, field_name);
                    validate_product_binding_pattern_shape(pattern.elements[i], source_type.field_types[field_index]);
                }
                return;
            }
            case PatternKind::EnumCase:
                if (source_type.primitive != IrPrimitiveKind::Struct) return;
                {
                    const StructInfo& info = require_struct_match_pattern_type(pattern.loc, pattern.case_name, source_type);
                    if (!info.tuple_struct || !pattern.has_payload_pattern || !pattern.payload_pattern) return;
                    const std::vector<IrType>& fields = aggregate_field_types(source_type);
                    const Pattern& payload = *pattern.payload_pattern;
                    if (payload.kind == PatternKind::Tuple) {
                        if (payload.has_rest) {
                            if (payload.elements.size() > fields.size()) {
                                fail(payload.loc,
                                     "tuple-struct binding pattern has " + std::to_string(payload.elements.size()) +
                                     " non-rest elements but value has " + std::to_string(fields.size()));
                            }
                        } else if (payload.elements.size() != fields.size()) {
                            fail(payload.loc,
                                 "tuple-struct binding pattern has " + std::to_string(payload.elements.size()) +
                                 " elements but value has " + std::to_string(fields.size()));
                        }
                        for (std::size_t i = 0; i < payload.elements.size(); ++i) {
                            std::size_t field_index = tuple_pattern_field_index(payload, fields.size(), i);
                            validate_product_binding_pattern_shape(payload.elements[i], fields[field_index]);
                        }
                        return;
                    }
                    if (fields.size() == 1) {
                        validate_product_binding_pattern_shape(payload, fields[0]);
                    }
                }
                return;
        }
    }

    void lower_refutable_product_binding_pattern_from_local(
        const Pattern& pattern,
        const std::string& source_name,
        const IrType& source_type,
        bool mutable_binding,
        std::vector<IrStmtPtr>& statements
    ) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_refutable_product_binding_pattern_from_local(
                effective_pattern, source_name, source_type, mutable_binding, statements);
            return;
        }
        if (!is_aggregate_type(source_type)) {
            fail(pattern.loc, "aggregate binding pattern requires tuple, array, or struct value");
        }

        std::vector<Pattern> alternatives = expand_or_pattern_alternatives(pattern);
        std::set<std::string> reusable_names;
        if (pattern_contains_or(pattern)) {
            reusable_names = require_same_or_pattern_bindings(pattern.loc, alternatives, source_type);
        }

        bool has_irrefutable_alternative = false;
        std::vector<TupleCheckedStmtArm> checked_arms;
        checked_arms.reserve(alternatives.size() + 1);

        for (std::size_t i = 0; i < alternatives.size(); ++i) {
            const Pattern& alternative = alternatives[i];
            validate_product_binding_pattern_shape(alternative, source_type);

            TupleCheckedStmtArm arm;
            arm.loc = alternative.loc;
            std::vector<IrStmtPtr> condition_prelude;
            arm.condition = lower_product_match_pattern_condition(
                alternative,
                source_name,
                source_type,
                condition_prelude
            );
            if (!arm.condition) has_irrefutable_alternative = true;
            for (auto& statement : condition_prelude) {
                statements.push_back(std::move(statement));
            }

            local_scopes_.set_reusable_pattern_bindings(i == 0 ? std::set<std::string>{} : reusable_names);
            lower_product_match_pattern_bindings_from_local(
                alternative,
                source_name,
                source_type,
                arm.body,
                mutable_binding
            );
            local_scopes_.clear_reusable_pattern_bindings();
            checked_arms.push_back(std::move(arm));
        }

        if (!has_irrefutable_alternative) {
            TupleCheckedStmtArm failure;
            failure.loc = pattern.loc;
            failure.body.push_back(make_panic_stmt(pattern.loc));
            checked_arms.push_back(std::move(failure));
        }

        std::vector<IrStmtPtr> lowered = build_tuple_match_if_chain(checked_arms);
        for (auto& statement : lowered) {
            statements.push_back(std::move(statement));
        }
    }

    IrStmtPtr make_panic_stmt(SourceLocation loc) const {
        auto stmt = std::make_unique<IrStmt>();
        stmt->kind = IrStmtKind::ExprStmt;
        stmt->loc = loc;
        stmt->expr = make_builtin_call(loc, "panic", {}, void_type(loc));
        return stmt;
    }

    void lower_tuple_binding_pattern_from_local(
        const Pattern& pattern,
        const std::string& source_name,
        const IrType& source_type,
        bool mutable_binding,
        std::vector<IrStmtPtr>& statements
    ) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_tuple_binding_pattern_from_local(effective_pattern, source_name, source_type, mutable_binding, statements);
            return;
        }
        bool array_pattern = pattern.kind == PatternKind::Array;
        IrPrimitiveKind expected_primitive = array_pattern ? IrPrimitiveKind::Array : IrPrimitiveKind::Tuple;
        const char* pattern_name = array_pattern ? "array" : "tuple";
        if (!pattern.rest_alias_name.empty()) {
            fail(pattern.rest_alias_loc,
                 std::string(pattern_name) +
                     " rest bindings currently require a Vec[T] or Slice[T] value");
        }
        if (source_type.primitive != expected_primitive) {
            fail(pattern.loc, std::string(pattern_name) + " binding pattern requires a " +
                              pattern_name + " value, got " + type_name(source_type));
        }
        const std::vector<IrType>& fields = aggregate_field_types(source_type);
        if (pattern.has_rest) {
            if (pattern.elements.size() > fields.size()) {
                fail(pattern.loc,
                     std::string(pattern_name) + " binding pattern has " + std::to_string(pattern.elements.size()) +
                     " non-rest elements but value has " + std::to_string(fields.size()));
            }
        } else if (pattern.elements.size() != fields.size()) {
            fail(pattern.loc,
                 std::string(pattern_name) + " binding pattern has " + std::to_string(pattern.elements.size()) +
                 " elements but value has " + std::to_string(fields.size()));
        }
        std::size_t suffix_count = 0;
        if (pattern.has_rest) suffix_count = pattern.elements.size() - pattern.rest_index;
        for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
            const Pattern& item = pattern.elements[i];
            if (item.kind == PatternKind::Wildcard) continue;
            std::size_t field_index = i;
            if (pattern.has_rest && i >= pattern.rest_index) {
                field_index = fields.size() - suffix_count + (i - pattern.rest_index);
            }
            IrExprPtr field = make_tuple_index_expr(item.loc, source_name, source_type, field_index);
            if (item.kind == PatternKind::Tuple) {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(item.loc, nested_name, fields[field_index], false);
                statements.push_back(make_ir_var_decl(item.loc, nested_name, fields[field_index], std::move(field), false));
                lower_tuple_binding_pattern_from_local(item, nested_name, fields[field_index], mutable_binding, statements);
                continue;
            }
            if (item.kind == PatternKind::Array) {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(item.loc, nested_name, fields[field_index], false);
                statements.push_back(make_ir_var_decl(item.loc, nested_name, fields[field_index], std::move(field), false));
                if (is_runtime_sequence_pattern_subject(fields[field_index])) {
                    lower_runtime_sequence_match_pattern_bindings_from_local(
                        item,
                        nested_name,
                        fields[field_index],
                        statements,
                        mutable_binding
                    );
                    continue;
                }
                lower_tuple_binding_pattern_from_local(item, nested_name, fields[field_index], mutable_binding, statements);
                continue;
            }
            if (item.kind == PatternKind::Struct) {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(item.loc, nested_name, fields[field_index], false);
                statements.push_back(make_ir_var_decl(item.loc, nested_name, fields[field_index], std::move(field), false));
                lower_struct_binding_pattern_from_local(item, nested_name, fields[field_index], mutable_binding, statements);
                continue;
            }
            lower_binding_pattern_value(item, fields[field_index], std::move(field), mutable_binding, statements);
        }
    }

    void lower_struct_binding_pattern_from_local(
        const Pattern& pattern,
        const std::string& source_name,
        const IrType& source_type,
        bool mutable_binding,
        std::vector<IrStmtPtr>& statements
    ) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_struct_binding_pattern_from_local(effective_pattern, source_name, source_type, mutable_binding, statements);
            return;
        }
        if (source_type.primitive != IrPrimitiveKind::Struct) {
            fail(pattern.loc, "struct binding pattern requires a struct value, got " + type_name(source_type));
        }
        std::string struct_name = resolve_struct_type_name(pattern.case_name);
        auto struct_found = structs_.find(struct_name);
        if (struct_found == structs_.end()) {
            fail(pattern.loc, "unknown struct '" + pattern.case_name + "' in binding pattern");
        }
        require_struct_access(pattern.loc, struct_found->second);
        if (struct_name != source_type.name) {
            fail(pattern.loc,
                 "struct binding pattern type '" + struct_name + "' does not match value type " + type_name(source_type));
        }
        if (pattern.field_names.size() != pattern.elements.size()) {
            throw CompileError("internal error: struct binding pattern field/value arity mismatch");
        }

        std::set<std::string> seen_fields;
        for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
            const std::string& field_name = pattern.field_names[i];
            if (!seen_fields.insert(field_name).second) {
                fail(pattern.elements[i].loc, "duplicate field '" + field_name + "' in struct binding pattern");
            }
            std::size_t field_index = struct_field_index(pattern.elements[i].loc, source_type, field_name);
            const IrType& field_type = source_type.field_types[field_index];
            const Pattern& item = pattern.elements[i];
            if (item.kind == PatternKind::Wildcard) continue;
            IrExprPtr field = make_tuple_index_expr(item.loc, source_name, source_type, field_index);
            if (item.kind == PatternKind::Tuple) {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(item.loc, nested_name, field_type, false);
                statements.push_back(make_ir_var_decl(item.loc, nested_name, field_type, std::move(field), false));
                lower_tuple_binding_pattern_from_local(item, nested_name, field_type, mutable_binding, statements);
                continue;
            }
            if (item.kind == PatternKind::Array) {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(item.loc, nested_name, field_type, false);
                statements.push_back(make_ir_var_decl(item.loc, nested_name, field_type, std::move(field), false));
                if (is_runtime_sequence_pattern_subject(field_type)) {
                    lower_runtime_sequence_match_pattern_bindings_from_local(
                        item,
                        nested_name,
                        field_type,
                        statements,
                        mutable_binding
                    );
                    continue;
                }
                lower_tuple_binding_pattern_from_local(item, nested_name, field_type, mutable_binding, statements);
                continue;
            }
            if (item.kind == PatternKind::Struct) {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(item.loc, nested_name, field_type, false);
                statements.push_back(make_ir_var_decl(item.loc, nested_name, field_type, std::move(field), false));
                lower_struct_binding_pattern_from_local(item, nested_name, field_type, mutable_binding, statements);
                continue;
            }
            lower_binding_pattern_value(item, field_type, std::move(field), mutable_binding, statements);
        }
    }

    void lower_binding_pattern_value(
        const Pattern& pattern,
        const IrType& value_type,
        IrExprPtr value,
        bool mutable_binding,
        std::vector<IrStmtPtr>& statements
    ) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_binding_pattern_value(effective_pattern, value_type, std::move(value), mutable_binding, statements);
            return;
        }
        if (pattern.kind == PatternKind::Wildcard) return;
        if (pattern.kind != PatternKind::Binding) {
            fail(pattern.loc, "let/var enum pattern bindings are planned but are not supported yet");
        }
        if (is_borrow_type(value_type)) {
            fail(pattern.loc, "borrow pattern bindings are not supported yet; pass ref values directly to calls");
        }
        if (is_owner_type(value_type)) {
            fail(pattern.loc, "owning pattern bindings are planned after ownership through aggregates is implemented");
        }
        declare_local(pattern.loc, pattern.payload_name, value_type, mutable_binding);
        statements.push_back(make_ir_var_decl(pattern.loc, pattern.payload_name, value_type, std::move(value), mutable_binding));
    }

    void require_owned_field_alive(SourceLocation loc, const std::string& base_name, LocalInfo& local, const std::string& path) {
        if (!local_owned_field_has_state(local, path)) {
            throw CompileError("internal error: missing owned field state for '" + base_name + "." + path + "'");
        }
        for (const auto& item : local.owned_field_states) {
            if (!local_owned_field_path_matches(item.first, path)) continue;
            if (item.second != LocalState::Alive) {
                fail(loc, "cannot use " + local_state_name(item.second) + " owned field '" + base_name + "." + path + "'");
            }
        }
    }

    void mark_owned_field_moved(SourceLocation loc, const std::string& base_name, const std::string& path) {
        LocalInfo& local = require_live_local(loc, base_name);
        require_not_borrowed(loc, base_name, local, "move field from");
        require_owned_field_alive(loc, base_name, local, path);
        mark_local_owned_field_state(local, path, LocalState::Moved);
    }

    void require_can_assign_owned_field(SourceLocation loc, const std::string& base_name, const std::string& path) {
        LocalInfo& local = require_live_local(loc, base_name);
        if (!local_owned_field_has_state(local, path)) {
            throw CompileError("internal error: missing owned field assignment state for '" + base_name + "." + path + "'");
        }
        if (local_owned_field_is_live(local, path)) {
            fail(loc, "cannot overwrite owning field '" + base_name + "." + path + "' before it is moved or dropped");
        }
    }

    bool tracked_ir_access_path(const IrExpr& expr, std::string& base_name, std::string& path) const {
        if (expr.kind == IrExprKind::Local) {
            base_name = ir_expr_name(expr);
            path.clear();
            return true;
        }
        if (expr.kind == IrExprKind::TupleIndex && ir_expr_operand(expr)) {
            if (!tracked_ir_access_path(*ir_expr_operand(expr), base_name, path)) return false;
            path = local_owned_field_path(path, static_cast<std::size_t>(expr.tuple_index));
            return true;
        }
        if (expr.kind == IrExprKind::Index && ir_expr_operand(expr) && ir_expr_right(expr) &&
            ir_expr_right(expr)->kind == IrExprKind::Integer && !ir_expr_right(expr)->int_negative) {
            if (!tracked_ir_access_path(*ir_expr_operand(expr), base_name, path)) return false;
            path = local_owned_field_path(path, static_cast<std::size_t>(ir_expr_right(expr)->int_value));
            return true;
        }
        return false;
    }

    static std::string local_owned_field_path_from_indices(const std::vector<std::size_t>& indices) {
        std::string path;
        for (std::size_t index : indices) {
            path = local_owned_field_path(path, index);
        }
        return path;
    }

    bool assignment_target_allows_zone_pointer_storage(const IrExpr& target) {
        if (current_module_name_ != "std::boxed" &&
            current_module_name_ != "std::vec" &&
            current_module_name_ != "std::string") {
            return false;
        }
        std::string base_name;
        std::string path;
        if (!tracked_ir_access_path(target, base_name, path)) return false;
        const LocalInfo* local = find_local_slot(base_name);
        if (!local) return false;
        if (current_module_name_ == "std::boxed") {
            std::optional<std::vector<std::size_t>> data_path =
                std_box_zone_handle_data_field_path_indices(value_qualified_type(local->type));
            return data_path && path == local_owned_field_path_from_indices(*data_path);
        }
        if (current_module_name_ == "std::string") {
            std::optional<std::vector<std::size_t>> data_path =
                std_string_zone_handle_data_field_path_indices(value_qualified_type(local->type));
            return data_path && path == local_owned_field_path_from_indices(*data_path);
        }
        std::optional<std::vector<std::size_t>> data_path =
            std_vec_zone_handle_data_field_path_indices(value_qualified_type(local->type));
        return data_path && path == local_owned_field_path_from_indices(*data_path);
    }

    void mark_owned_field_assigned(const IrExpr& target) {
        if (!is_owner_type(target.type)) return;
        std::string base_name;
        std::string path;
        if (!tracked_ir_access_path(target, base_name, path) || path.empty()) return;
        LocalInfo& local = local_slot_by_name(base_name);
        mark_local_owned_field_state(local, path, LocalState::Alive);
    }

    void update_aggregate_borrow_sources_after_assignment(SourceLocation loc,
                                                          const IrExpr& target,
                                                          std::size_t borrow_mark) {
        if (!contains_borrow_type(target.type)) {
            release_temporary_borrows(borrow_mark);
            return;
        }
        std::string base_name;
        std::string path;
        if (!tracked_ir_access_path(target, base_name, path)) {
            fail(loc, "assigning borrow-valued raw pointer fields independently is planned but not supported yet");
        }
        release_aggregate_borrow_sources_at(base_name, path);
        promote_temporary_borrows_to_aggregate(borrow_mark, base_name, path);
    }

    void check_assign(const Stmt& stmt, IrStmt& lowered) {
        const ExprPtr& assign_target = stmt_assign_target(stmt);
        const ExprPtr& rhs = stmt_assign_rhs(stmt);
        if (assign_target) {
            IrExprPtr target = check_assignment_target(*assign_target);
            IrType target_type = target->type;
            std::size_t borrow_mark = temporary_borrow_mark();
            IrExprPtr value = check_expr_with_expected(*rhs, target_type);
            coerce_expr_to_expected(*value, target_type);
            require_assignable(stmt.loc, target_type, value->type);
            if (!assignment_target_allows_zone_pointer_storage(*target)) {
                require_no_zone_pointer_escape(rhs->loc, *value, "aggregate or raw-pointer storage");
            }
            mark_owned_field_assigned(*target);
            update_aggregate_borrow_sources_after_assignment(stmt.loc, *target, borrow_mark);
            set_ir_stmt_assign_target(lowered, std::move(target));
            set_ir_stmt_assign_rhs(lowered, std::move(value));
            return;
        }

        const std::string& assign_name = stmt_assign_name(stmt);
        LocalInfo& target = require_local_slot(stmt.loc, assign_name);
        if (auto error = local_assignment_target_error(assign_name, target)) fail(stmt.loc, *error);
        require_not_borrowed(stmt.loc, assign_name, target, "assign to");
        if (auto error = local_assignment_storage_error(assign_name, target)) fail(stmt.loc, *error);

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr_with_expected(*rhs, target.type);
        widen_vector_storage_for_assignment(target, *value);
        coerce_expr_to_expected(*value, target.type);
        require_assignable(stmt.loc, target.type, value->type);
        VectorKnownLength assigned_vector_length =
            vector_known_length_from_source_expr(target.type, *rhs, *value);
        if (contains_borrow_type(target.type)) {
            release_aggregate_borrow_sources_at(assign_name, "");
            promote_temporary_borrows_to_aggregate(borrow_mark, assign_name);
        } else {
            release_temporary_borrows(borrow_mark);
        }
        mark_local_alive(target);
        initialize_owned_field_states(target);
        set_local_vector_known_length(target, assigned_vector_length);
        set_zone_pointer_source_from_expr(target, *value);
        set_ir_stmt_assign_name(lowered, assign_name);
        set_ir_stmt_assign_rhs(lowered, std::move(value));
    }

    IrExprPtr check_assignment_target(const Expr& expr) {
        if (expr.kind == ExprKind::FieldAccess ||
            expr.kind == ExprKind::TupleIndex ||
            expr.kind == ExprKind::Index) {
            TrackedAggregateAccess access;
            if (try_build_tracked_aggregate_access(expr, access)) {
                return check_tracked_assignment_target(expr.loc, std::move(access));
            }
            if (has_raw_pointer_deref_base(expr)) {
                return check_raw_pointer_aggregate_assignment_target(expr);
            }
        }
        if (expr.kind == ExprKind::FieldAccess) {
            return check_field_assignment_target(expr);
        }
        if (expr.kind == ExprKind::TupleIndex) {
            return check_tuple_field_assignment_target(expr);
        }
        if (expr.kind == ExprKind::Index) {
            return check_slice_index_assignment_target(expr);
        }
        if (expr.kind == ExprKind::Unary && expr.op == TokenKind::Star) {
            return check_pointer_deref_assignment_target(expr);
        }
        fail(expr.loc, "assignment target must be a binding, field access, index access, or pointer dereference");
    }

    bool has_raw_pointer_deref_base(const Expr& expr) const {
        if (expr.kind == ExprKind::Unary && expr.op == TokenKind::Star) return true;
        if ((expr.kind == ExprKind::FieldAccess ||
            expr.kind == ExprKind::TupleIndex ||
             expr.kind == ExprKind::Index) &&
            expr_operand(expr)) {
            return has_raw_pointer_deref_base(*expr_operand(expr));
        }
        return false;
    }

    static bool is_raw_pointer_backed_lvalue(const IrExpr& expr) {
        if (expr.kind == IrExprKind::PointerLoad) return true;
        if ((expr.kind == IrExprKind::TupleIndex ||
             expr.kind == IrExprKind::Index) &&
            ir_expr_operand(expr)) {
            return is_raw_pointer_backed_lvalue(*ir_expr_operand(expr));
        }
        return false;
    }

    IrExprPtr check_raw_pointer_aggregate_assignment_target(const Expr& expr) {
        IrExprPtr target = check_expr(expr);
        if (!is_raw_pointer_backed_lvalue(*target)) {
            fail(expr.loc, "assignment target must be a raw-pointer field or element access");
        }
        if (is_aggregate_type(target->type) &&
            (is_owner_type(target->type) || contains_borrow_type(target->type))) {
            fail(expr.loc, "raw pointer aggregate assignment cannot copy ownership- or borrow-valued aggregates yet");
        }
        if (contains_borrow_type(target->type)) {
            fail(expr.loc, "assigning borrow-valued raw pointer fields independently is planned but not supported yet");
        }
        return target;
    }

    LocalInfo& require_mutable_base_local(SourceLocation loc, const Expr& expr, std::string& base_name) {
        if (expr.kind != ExprKind::Name) {
            fail(loc, "field assignment currently requires a local struct binding");
        }
        base_name = expr.name;
        LocalInfo& local = require_live_local(loc, expr.name);
        if (auto error = local_field_assignment_base_error(expr.name, local)) fail(loc, *error);
        require_not_borrowed(loc, expr.name, local, "assign to");
        return local;
    }

    bool try_build_tracked_aggregate_access(const Expr& expr, TrackedAggregateAccess& out) {
        if (expr.kind == ExprKind::Name) {
            LocalInfo* local_slot = find_local_slot(expr.name);
            if (!local_slot) return false;
            LocalInfo& local = *local_slot;
            if (auto error = local_unavailable_binding_error(expr.name, local)) fail(expr.loc, *error);
            require_zone_pointer_valid(expr.loc, expr.name, local);
            out.base_name = expr.name;
            out.base_type = local.type;
            out.type = local.type;
            out.path.clear();
            out.expr = make_local_lvalue_expr(expr.loc, expr.name, local.type);
            out.has_final_field_mutability = false;
            out.final_field_mutable = true;
            out.final_field_label.clear();
            out.final_container_name.clear();
            return true;
        }

        if (expr.kind == ExprKind::FieldAccess) {
            TrackedAggregateAccess base;
            if (!try_build_tracked_aggregate_access(*expr_operand(expr), base)) return false;
            if (base.type.primitive != IrPrimitiveKind::Struct) {
                fail(expr.loc, "field access requires a struct value, got " + type_name(base.type));
            }
            std::size_t index = struct_field_index(expr.loc, base.type, expr.name);
            const IrType field_type = base.type.field_types[index];

            out.base_name = std::move(base.base_name);
            out.base_type = std::move(base.base_type);
            out.type = field_type;
            out.path = local_owned_field_path(base.path, index);
            out.expr = make_tuple_index_expr(expr.loc, std::move(base.expr), index);
            out.has_final_field_mutability = true;
            out.final_field_mutable = base.type.field_mutable[index];
            out.final_field_label = expr.name;
            out.final_container_name = base.type.name;
            return true;
        }

        if (expr.kind == ExprKind::TupleIndex) {
            TrackedAggregateAccess base;
            if (!try_build_tracked_aggregate_access(*expr_operand(expr), base)) return false;
            if (base.type.primitive != IrPrimitiveKind::Tuple &&
                base.type.primitive != IrPrimitiveKind::Struct) {
                fail(expr.loc, "tuple index access requires tuple or tuple struct value, got " + type_name(base.type));
            }
            const std::vector<IrType>& fields = aggregate_field_types(base.type);
            if (expr.tuple_index >= fields.size()) {
                fail(expr.loc,
                     "tuple index " + std::to_string(expr.tuple_index) +
                     " is out of range for " + type_name(base.type));
            }
            std::size_t field_index = static_cast<std::size_t>(expr.tuple_index);
            const IrType field_type = fields[field_index];

            out.base_name = std::move(base.base_name);
            out.base_type = std::move(base.base_type);
            out.type = field_type;
            out.path = local_owned_field_path(base.path, field_index);
            out.expr = make_tuple_index_expr(expr.loc, std::move(base.expr), field_index);
            out.has_final_field_mutability = base.type.primitive == IrPrimitiveKind::Struct;
            out.final_field_mutable = !out.has_final_field_mutability ||
                base.type.field_mutable[field_index];
            out.final_field_label = std::to_string(expr.tuple_index);
            out.final_container_name = base.type.name;
            return true;
        }

        if (expr.kind == ExprKind::Index) {
            TrackedAggregateAccess base;
            if (!try_build_tracked_aggregate_access(*expr_operand(expr), base)) return false;
            if (is_prelude_slice_type(base.type)) return false;
            if (base.type.primitive != IrPrimitiveKind::Array &&
                base.type.primitive != IrPrimitiveKind::Vector) {
                fail(expr.loc, "index access requires an array or vector value, got " + type_name(base.type));
            }
            if (base.type.args.size() != 1) {
                throw CompileError("internal error: aggregate index without element type");
            }
            IrExprPtr index = check_expr(*expr_right(expr));
            if (!is_value_integer_type(index->type)) {
                fail(expr.loc, "index expression must be an integer, got " + type_name(index->type));
            }
            if (index->kind != IrExprKind::Integer || index->int_negative) {
                if (is_owner_type(base.type.args[0])) {
                    fail(expr.loc,
                         "moving owning aggregate elements through dynamic indexes is not supported; "
                         "use a constant index or move the whole aggregate");
                }
                return false;
            }
            std::uint64_t index_value = index->int_value;
            const bool vector_index = base.type.primitive == IrPrimitiveKind::Vector;
            if (vector_index) {
                require_vector_index_in_known_bounds(
                    expr.loc,
                    StaticIntegerValue{index_value, false},
                    known_local_vec_length_for_access(base)
                );
            }
            if (index_value >= base.type.array_size) {
                std::string label = vector_index ? "vector" : "array";
                fail(expr.loc,
                     label + " index " + std::to_string(index_value) +
                     " is out of range for " + std::to_string(base.type.array_size) + " elements");
            }

            const IrType element_type = base.type.args[0];
            IrExprPtr lowered = base.type.primitive == IrPrimitiveKind::Array
                ? make_tuple_index_expr(expr.loc, std::move(base.expr), static_cast<std::size_t>(index_value))
                : make_ir_index_expr(expr.loc, std::move(base.expr), std::move(index));

            out.base_name = std::move(base.base_name);
            out.base_type = std::move(base.base_type);
            out.type = element_type;
            out.path = local_owned_field_path(base.path, static_cast<std::size_t>(index_value));
            out.expr = std::move(lowered);
            out.has_final_field_mutability = false;
            out.final_field_mutable = true;
            out.final_field_label.clear();
            out.final_container_name.clear();
            return true;
        }

        return false;
    }

    IrExprPtr check_tracked_assignment_target(SourceLocation loc, TrackedAggregateAccess access) {
        LocalInfo& local = require_live_local(loc, access.base_name);
        if (auto error = local_aggregate_assignment_base_error(access.base_name, local, access.base_type)) {
            fail(loc, *error);
        }
        require_can_assign_borrow_path(loc, access.base_name, local, access.path);
        if (access.has_final_field_mutability && !access.final_field_mutable) {
            fail(loc,
                 "cannot assign to immutable field '" + access.final_field_label +
                     "' of struct '" + access.final_container_name + "'");
        }
        if (is_owner_type(access.type)) {
            require_can_assign_owned_field(loc, access.base_name, access.path);
        }
        return std::move(access.expr);
    }

    IrExprPtr check_field_assignment_target(const Expr& expr) {
        std::string base_name;
        const Expr& operand_expr = *expr_operand(expr);
        LocalInfo& local = require_mutable_base_local(expr.loc, operand_expr, base_name);
        if (local.type.primitive != IrPrimitiveKind::Struct) {
            fail(expr.loc, "field assignment requires a struct value, got " + type_name(local.type));
        }

        std::size_t index = struct_field_index(expr.loc, local.type, expr.name);
        const IrType& field_type = local.type.field_types[index];
        if (is_owner_type(field_type)) {
            require_can_assign_owned_field(expr.loc, base_name, std::to_string(index));
        }
        if (!local.type.field_mutable[index]) {
            fail(expr.loc, "cannot assign to immutable field '" + expr.name + "' of struct '" + local.type.name + "'");
        }

        return make_tuple_index_expr(
            expr.loc,
            make_local_lvalue_expr(operand_expr.loc, base_name, local.type),
            index
        );
    }

    IrExprPtr check_tuple_field_assignment_target(const Expr& expr) {
        std::string base_name;
        const Expr& operand_expr = *expr_operand(expr);
        LocalInfo& local = require_mutable_base_local(expr.loc, operand_expr, base_name);
        if (local.type.primitive != IrPrimitiveKind::Struct) {
            fail(expr.loc, "tuple-field assignment requires a tuple struct value, got " + type_name(local.type));
        }
        if (expr.tuple_index >= local.type.field_types.size()) {
            fail(expr.loc,
                 "tuple field index " + std::to_string(expr.tuple_index) +
                 " is out of range for " + type_name(local.type));
        }
        const IrType& field_type = local.type.field_types[static_cast<std::size_t>(expr.tuple_index)];
        if (is_owner_type(field_type)) {
            require_can_assign_owned_field(expr.loc, base_name, std::to_string(expr.tuple_index));
        }
        if (!local.type.field_mutable[static_cast<std::size_t>(expr.tuple_index)]) {
            fail(expr.loc,
                 "cannot assign to immutable field '" + std::to_string(expr.tuple_index) +
                 "' of struct '" + local.type.name + "'");
        }

        return make_tuple_index_expr(
            expr.loc,
            make_local_lvalue_expr(operand_expr.loc, base_name, local.type),
            static_cast<std::size_t>(expr.tuple_index)
        );
    }

    IrExprPtr check_slice_index_assignment_target(const Expr& expr) {
        IrExprPtr operand = check_aggregate_access_operand(*expr_operand(expr));
        if (!is_prelude_slice_type(operand->type)) {
            fail(expr.loc,
                 "dynamic index assignment currently supports Slice values; "
                 "use Vec.set(...) for local Vec values or a constant index for local arrays/vectors");
        }
        IrExprPtr index = check_expr(*expr_right(expr));
        if (!is_value_integer_type(index->type)) {
            fail(expr.loc, "index expression must be an integer, got " + type_name(index->type));
        }
        if (index->kind == IrExprKind::Integer && index->int_negative) {
            fail(expr.loc, "Slice index must be non-negative");
        }
        const IrType element_type = operand->type.args[0];
        require_slice_element_materializable(expr.loc, element_type, "Slice element assignment");

        return make_ir_index_expr(expr.loc, std::move(operand), std::move(index));
    }

    IrExprPtr check_pointer_deref_assignment_target(const Expr& expr) {
        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr pointer = check_expr(*expr_operand(expr));
        release_temporary_borrows(borrow_mark);

        IrType element_type = require_raw_pointer_materializable_type(expr.loc, pointer->type, "pointer dereference");
        return make_pointer_load_expr(expr.loc, std::move(pointer), element_type);
    }

    IrExprPtr check_pointer_deref_access_operand(const Expr& expr) {
        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr pointer = check_expr(*expr_operand(expr));
        release_temporary_borrows(borrow_mark);

        IrType element_type = require_raw_pointer_deref_type(expr.loc, pointer->type, "pointer dereference");
        return make_pointer_load_expr(expr.loc, std::move(pointer), element_type);
    }

    using DropValueFactory = std::function<IrExprPtr()>;

    IrExprPtr make_field_value_expr(SourceLocation loc, DropValueFactory make_source, std::size_t index, const IrType& field_type) const {
        (void)field_type;
        return make_tuple_index_expr(loc, make_source(), index);
    }

    IrExprPtr make_vector_element_expr(SourceLocation loc, DropValueFactory make_source, std::uint64_t index, const IrType& element_type) const {
        (void)element_type;
        return make_ir_index_expr(loc, make_source(), make_integer_literal(loc, i64_type(loc), index));
    }

    IrStmtPtr make_drop_call_stmt(SourceLocation loc, const ImplMethodInfo& method, DropValueFactory make_value) {
        if (!is_valid_drop_method_signature(method.sig.params.size(), method.sig.result)) {
            throw CompileError(invalid_drop_impl_internal_message(method.receiver_type));
        }
        std::vector<IrExprPtr> args;
        args.push_back(make_value());
        auto call = make_ir_call_expr(loc, method.lowered_name, method.sig.result, std::move(args));
        queue_impl_method_for_lowering(method);

        auto stmt = std::make_unique<IrStmt>();
        stmt->kind = IrStmtKind::ExprStmt;
        stmt->loc = loc;
        stmt->expr = std::move(call);
        return stmt;
    }

    std::optional<ImplMethodInfo> find_drop_impl(SourceLocation loc, const IrType& dropped_type) {
        auto exact = drop_impls_.find(drop_impl_key(dropped_type));
        if (exact != drop_impls_.end()) return exact->second;

        std::vector<ImplMethodInfo> matches;
        for (const auto& candidate : generic_method_impls_) {
            if (!is_drop_impl_method(candidate)) continue;
            if (!is_valid_drop_method_signature(candidate.sig.params.size(), candidate.sig.result)) {
                continue;
            }

            std::map<std::string, IrType> substitutions;
            if (!infer_generic_impl_method_substitutions(candidate, dropped_type, substitutions)) {
                continue;
            }

            std::set<std::string> visiting;
            std::string bound_failure;
            if (!impl_generic_bounds_satisfied(candidate.generic_bounds, substitutions, visiting, &bound_failure)) {
                continue;
            }
            if (!impl_generic_bounds_satisfied(candidate.method_generic_bounds, substitutions, visiting, &bound_failure)) {
                continue;
            }

            matches.push_back(specialize_generic_impl_method_with_substitutions(
                candidate,
                dropped_type,
                "drop",
                std::move(substitutions)));
        }

        if (matches.empty()) {
            return std::nullopt;
        }
        if (matches.size() > 1) {
            fail(loc, ambiguous_drop_impl_message(dropped_type));
        }
        return matches.front();
    }

    void append_drop_stmts_for_value(SourceLocation loc,
                                     const IrType& type,
                                     DropValueFactory make_value,
                                     std::vector<IrStmtPtr>& statements,
                                     const LocalInfo* tracked_local = nullptr,
                                     const std::string& path = "") {
        if (tracked_local && !path.empty() && !local_owned_field_is_live(*tracked_local, path)) {
            return;
        }

        IrType dropped_type = type;
        dropped_type.qualifier = TypeQualifier::Value;
        std::optional<ImplMethodInfo> destructor = find_drop_impl(loc, dropped_type);
        bool can_call_destructor = destructor.has_value();
        if (can_call_destructor && tracked_local && path.empty() && local_has_moved_or_dropped_owned_fields(*tracked_local)) {
            can_call_destructor = false;
        }
        if (can_call_destructor) {
            statements.push_back(make_drop_call_stmt(loc, *destructor, make_value));
        }

        if (type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            type.primitive == IrPrimitiveKind::Struct) {
            const std::vector<IrType>& fields = aggregate_field_types(type);
            for (std::size_t i = 0; i < fields.size(); ++i) {
                const IrType& field_type = fields[i];
                if (!is_owner_type(field_type)) continue;
                std::string field_path = local_owned_field_path(path, i);
                DropValueFactory make_field = [this, loc, make_value, i, field_type]() {
                    return make_field_value_expr(loc, make_value, i, field_type);
                };
                append_drop_stmts_for_value(loc, field_type, make_field, statements, tracked_local, field_path);
            }
            return;
        }

        if (type.primitive == IrPrimitiveKind::Vector && type.args.size() == 1 && type.array_size != 0 && is_owner_type(type.args[0])) {
            const IrType element_type = type.args[0];
            for (std::uint64_t i = 0; i < type.array_size; ++i) {
                std::string element_path = local_owned_field_path(path, static_cast<std::size_t>(i));
                DropValueFactory make_element = [this, loc, make_value, i, element_type]() {
                    return make_vector_element_expr(loc, make_value, i, element_type);
                };
                append_drop_stmts_for_value(loc, element_type, make_element, statements, tracked_local, element_path);
            }
        }
    }

    void check_drop(const Stmt& stmt, IrStmt& lowered) {
        const std::string& drop_name = stmt_drop_name(stmt);
        LocalInfo& local = require_live_local(stmt.loc, drop_name);
        require_not_borrowed(stmt.loc, drop_name, local, "drop");
        if (local.type.qualifier == TypeQualifier::Own && local.type.primitive == IrPrimitiveKind::Zone) {
            fail(stmt.loc, "use zone::destroy(" + drop_name + ") to release a Zone");
        }
        DropValueFactory make_value = [this, loc = stmt.loc, name = drop_name, type = local.type]() {
            return make_local_lvalue_expr(loc, name, type);
        };
        std::vector<IrStmtPtr> drop_statements;
        append_drop_stmts_for_value(stmt.loc, local.type, make_value, drop_statements, &local);
        if (!drop_statements.empty()) {
            lowered.kind = IrStmtKind::Block;
            set_ir_stmt_statements(lowered, std::move(drop_statements));
        }
        mark_all_local_owned_fields(local, LocalState::Dropped);
        mark_local_dropped(local);
        set_ir_stmt_drop_name(lowered, drop_name);
    }

    void check_return(const Stmt& stmt, IrStmt& lowered) {
        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = stmt.expr ? check_expr_with_expected(*stmt.expr, current_return_) : nullptr;
        IrType actual = value ? value->type : void_type(stmt.loc);
        if (value) {
            coerce_expr_to_expected(*value, current_return_);
            actual = value->type;
        }
        require_assignable(stmt.loc, current_return_, actual);
        if (is_borrow_type(actual)) {
            require_borrow_return_source(stmt.loc, *value);
            release_temporary_borrows(borrow_mark);
        } else if (contains_borrow_type(actual)) {
            fail(stmt.loc, "function returns cannot contain borrow-valued aggregates yet");
        }
        if (value) {
            require_zone_pointer_not_escape_temporary_scope(stmt.loc, *value, 0, "function return");
            std::string zone_source;
            if (zone_pointer_source_name_from_expr(*value, zone_source) &&
                (!current_zone_pointer_return_param_index_ ||
                 zone_source != current_zone_pointer_return_source_)) {
                fail(stmt.loc,
                     "zone pointer returns must come from the function's single zone borrow parameter");
            }
        }
        std::vector<IrStmtPtr> cleanup;
        value = materialize_value_before_auto_destroy_cleanup(
            stmt.loc,
            std::move(value),
            cleanup,
            0,
            "$return",
            "function return"
        );
        append_active_loop_exit_owner_cleanup(stmt.loc, cleanup);
        require_no_live_owners_before_return(stmt.loc);
        if (!cleanup.empty()) {
            std::vector<IrStmtPtr> block;
            for (auto& cleanup_stmt : cleanup) block.push_back(std::move(cleanup_stmt));
            auto return_stmt = std::make_unique<IrStmt>();
            return_stmt->kind = IrStmtKind::Return;
            return_stmt->loc = stmt.loc;
            return_stmt->expr = std::move(value);
            block.push_back(std::move(return_stmt));
            lowered.kind = IrStmtKind::Block;
            set_ir_stmt_statements(lowered, std::move(block));
            return;
        }
        lowered.expr = std::move(value);
    }

    Flow check_if(const Stmt& stmt, IrStmt& lowered) {
        if (stmt.has_condition_pattern) return check_if_let(stmt, lowered);
        lowered.condition = check_expr(*stmt.condition);
        coerce_condition_to_bool(stmt.loc, lowered.condition);
        std::optional<bool> literal_condition = literal_bool_condition_value(*lowered.condition);
        StateSnapshot branch_input = snapshot_states();
        std::vector<LoopInfo> loop_input = loops_;

        CheckedStatements then_checked = check_statements(stmt_then_body(stmt), true);
        set_ir_stmt_then_body(lowered, std::move(then_checked.statements));
        StateSnapshot then_state = snapshot_states();
        std::vector<LoopInfo> then_loops = loops_;

        restore_states(branch_input);
        if (literal_condition && !*literal_condition) loops_ = loop_input;

        if (stmt_else_body(stmt).empty()) {
            if (literal_condition && !*literal_condition) {
                restore_states(branch_input);
                return Flow::Continues;
            }
            if (literal_condition && *literal_condition) {
                if (then_checked.flow == Flow::Continues) restore_states(then_state);
                else restore_states(branch_input);
                return then_checked.flow;
            }
            if (then_checked.flow == Flow::Continues) {
                require_same_states(stmt.loc, branch_input, then_state, "changes ownership state in if without else");
                restore_merged_states(branch_input, then_state);
            } else {
                restore_states(branch_input);
            }
            return Flow::Continues;
        }

        if (literal_condition && *literal_condition) loops_ = loop_input;
        CheckedStatements else_checked = check_statements(stmt_else_body(stmt), true);
        set_ir_stmt_else_body(lowered, std::move(else_checked.statements));
        StateSnapshot else_state = snapshot_states();
        std::vector<LoopInfo> else_loops = loops_;

        if (literal_condition) {
            Flow selected_flow = *literal_condition ? then_checked.flow : else_checked.flow;
            loops_ = *literal_condition ? std::move(then_loops) : std::move(else_loops);
            if (selected_flow == Flow::Continues) {
                restore_states(*literal_condition ? then_state : else_state);
            } else {
                restore_states(branch_input);
            }
            return selected_flow;
        }

        if (then_checked.flow != Flow::Continues && else_checked.flow != Flow::Continues) {
            restore_states(branch_input);
            if (then_checked.flow == Flow::Returns && else_checked.flow == Flow::Returns) return Flow::Returns;
            return Flow::Stops;
        }
        if (then_checked.flow != Flow::Continues) {
            restore_states(else_state);
            return Flow::Continues;
        }
        if (else_checked.flow != Flow::Continues) {
            restore_states(then_state);
            return Flow::Continues;
        }

        require_same_states(stmt.loc, then_state, else_state, "has incompatible ownership states after if branches");
        restore_merged_states(then_state, else_state);
        return Flow::Continues;
    }

    ConstantValue evaluate_constant(ConstantInfo& info) {
        if (info.evaluated) return info.value;
        if (info.evaluating) {
            fail(info.loc, "constant cycle: " + format_constant_cycle_path(constant_eval_stack_, info.name));
        }

        ConstantEvaluationStackGuard stack_guard(constant_eval_stack_, info.name);

        info.evaluating = true;
        std::string previous_module = current_module_name_;
        std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
        current_module_name_ = info.module_name;
        current_type_substitutions_.clear();

        IrType expected = resolve_executable_type(info.type_ref);
        info.value = evaluate_constant_expr(*info.init, expected);
        info.value.type = expected;
        info.evaluated = true;
        info.evaluating = false;

        current_type_substitutions_ = std::move(previous_substitutions);
        current_module_name_ = previous_module;
        return info.value;
    }

    static bool is_untyped_integer_literal_expr(const Expr& expr) {
        return expr.kind == ExprKind::Integer && expr.literal_suffix.empty();
    }

    IrType infer_constant_expr_type(const Expr& expr, const IrType& fallback) {
        if (expr.kind == ExprKind::Integer) {
            return expr.literal_suffix.empty()
                ? fallback
                : integer_literal_suffix_type(expr.literal_suffix, expr.loc);
        }
        if (expr.kind == ExprKind::Bool) return bool_type(expr.loc);
        if (expr.kind == ExprKind::Name) {
            ConstantValue value;
            if (!resolve_constant_value(expr.loc, expr.name, value)) {
                fail(expr.loc, "constant initializer name '" + expr.name + "' must refer to a constant");
            }
            return value.type;
        }
        if (expr.kind == ExprKind::Unary) {
            if (expr.op == TokenKind::Bang) return bool_type(expr.loc);
            return infer_constant_expr_type(*expr_operand(expr), fallback);
        }
        if (expr.kind == ExprKind::Cast) {
            return resolve_executable_type(expr.cast_type);
        }
        if (expr.kind == ExprKind::Binary) {
            if (expr.op == TokenKind::AmpAmp ||
                expr.op == TokenKind::PipePipe ||
                expr.op == TokenKind::EqEq ||
                expr.op == TokenKind::BangEq ||
                expr.op == TokenKind::Less ||
                expr.op == TokenKind::LessEq ||
                expr.op == TokenKind::Greater ||
                expr.op == TokenKind::GreaterEq) {
                return bool_type(expr.loc);
            }
            return infer_constant_binary_integer_type(expr.loc, *expr_left(expr), *expr_right(expr), fallback);
        }
        if (expr.kind == ExprKind::FieldAccess) {
            if (!expr_operand(expr)) fail(expr.loc, "missing constant field operand");
            IrType operand_type = infer_constant_expr_type(*expr_operand(expr), fallback);
            if (operand_type.primitive != IrPrimitiveKind::Struct) {
                fail(expr.loc, "constant field access requires a struct value, got " + type_name(operand_type));
            }
            return operand_type.field_types[struct_field_index(expr.loc, operand_type, expr.name)];
        }
        if (expr.kind == ExprKind::TupleIndex) {
            if (!expr_operand(expr)) fail(expr.loc, "missing constant tuple index operand");
            IrType operand_type = infer_constant_expr_type(*expr_operand(expr), fallback);
            if (operand_type.primitive != IrPrimitiveKind::Tuple &&
                operand_type.primitive != IrPrimitiveKind::Struct) {
                fail(expr.loc, "constant tuple index access requires tuple or tuple struct value, got " +
                               type_name(operand_type));
            }
            const std::vector<IrType>& fields = aggregate_field_types(operand_type);
            if (expr.tuple_index >= fields.size()) {
                fail(expr.loc,
                     "tuple index " + std::to_string(expr.tuple_index) +
                         " is out of range for " + type_name(operand_type));
            }
            return fields[static_cast<std::size_t>(expr.tuple_index)];
        }
        if (expr.kind == ExprKind::Index) {
            if (!expr_operand(expr)) fail(expr.loc, "missing constant index operand");
            IrType operand_type = infer_constant_expr_type(*expr_operand(expr), fallback);
            if (operand_type.primitive != IrPrimitiveKind::Array || operand_type.args.size() != 1) {
                fail(expr.loc, "constant index access requires a fixed-array value, got " + type_name(operand_type));
            }
            return operand_type.args[0];
        }
        fail(expr.loc, "constant initializers currently support scalar and aggregate constant expressions");
    }

    IrType infer_constant_binary_integer_type(
        SourceLocation loc,
        const Expr& left,
        const Expr& right,
        const IrType& fallback
    ) {
        IrType left_type = infer_constant_expr_type(left, fallback);
        IrType right_fallback = is_untyped_integer_literal_expr(left) ? fallback : left_type;
        IrType right_type = infer_constant_expr_type(right, right_fallback);
        if (is_untyped_integer_literal_expr(left) && !is_untyped_integer_literal_expr(right)) {
            left_type = right_type;
        } else if (!is_untyped_integer_literal_expr(left) && is_untyped_integer_literal_expr(right)) {
            right_type = left_type;
        }
        if (!is_value_integer_type(left_type) || !is_value_integer_type(right_type)) {
            fail(loc, "constant arithmetic operands must be integers, got " +
                      type_name(left_type) + " and " + type_name(right_type));
        }
        require_numeric_operands(loc, left_type, right_type);
        return left_type;
    }

    ConstantValue evaluate_constant_binary_expr(const Expr& expr, const IrType& expected) {
        if (expected.qualifier == TypeQualifier::Value && expected.primitive == IrPrimitiveKind::Bool) {
            if (expr.op == TokenKind::AmpAmp || expr.op == TokenKind::PipePipe) {
                ConstantValue left = evaluate_constant_expr(*expr_left(expr), expected);
                ConstantValue right = evaluate_constant_expr(*expr_right(expr), expected);
                return evaluate_constant_bool_binary(expr.loc, expr.op, expected, left, right);
            }

            if (expr.op == TokenKind::EqEq || expr.op == TokenKind::BangEq) {
                IrType left_type = infer_constant_expr_type(*expr_left(expr), i64_type(expr.loc));
                if (left_type.qualifier == TypeQualifier::Value && left_type.primitive == IrPrimitiveKind::Bool) {
                    ConstantValue left = evaluate_constant_expr(*expr_left(expr), left_type);
                    ConstantValue right = evaluate_constant_expr(*expr_right(expr), left_type);
                    return evaluate_constant_bool_comparison(expr.loc, expr.op, expected, left, right);
                }
            }

            switch (expr.op) {
                case TokenKind::EqEq:
                case TokenKind::BangEq:
                case TokenKind::Less:
                case TokenKind::LessEq:
                case TokenKind::Greater:
                case TokenKind::GreaterEq:
                    break;
                default:
                    fail(expr.loc, "constant bool expressions support logical and comparison operators");
            }

            IrType operand_type = infer_constant_binary_integer_type(
                expr.loc,
                *expr_left(expr),
                *expr_right(expr),
                i64_type(expr.loc));
            ConstantValue left = evaluate_constant_expr(*expr_left(expr), operand_type);
            ConstantValue right = evaluate_constant_expr(*expr_right(expr), operand_type);
            return evaluate_constant_integer_comparison(expr.loc, expr.op, expected, operand_type, left, right);
        }

        if (!is_value_integer_type(expected)) {
            fail(expr.loc, "constant arithmetic expressions require an integer result type");
        }
        switch (expr.op) {
            case TokenKind::Plus:
            case TokenKind::Minus:
            case TokenKind::Star:
            case TokenKind::Slash:
            case TokenKind::Percent:
            case TokenKind::Amp:
            case TokenKind::Pipe:
            case TokenKind::Caret:
            case TokenKind::LessLess:
            case TokenKind::GreaterGreater:
                break;
            default:
                fail(expr.loc, "constant integer expressions support +, -, *, /, %, &, |, ^, <<, and >>");
        }

        ConstantValue left = evaluate_constant_expr(*expr_left(expr), expected);
        ConstantValue right = evaluate_constant_expr(*expr_right(expr), expected);
        return evaluate_constant_integer_binary(expr.loc, expr.op, expected, left, right);
    }

    ConstantValue evaluate_constant_unary_expr(const Expr& expr, const IrType& expected) {
        if (expr.op == TokenKind::Bang) {
            ConstantValue value = evaluate_constant_expr(*expr_operand(expr), expected);
            if (!value.is_bool) fail(expr.loc, "constant ! operand must be bool");
            return make_bool_constant(expr.loc, expected, !value.bool_value);
        }
        if (expr.op == TokenKind::Minus) {
            if (!is_value_integer_type(expected)) fail(expr.loc, "constant unary - operand must be integer");
            ConstantValue value = evaluate_constant_expr(*expr_operand(expr), expected);
            if (!is_signed_integer_primitive(expected.primitive)) {
                fail(expr.loc, "constant unary - requires a signed integer result type");
            }
            std::int64_t operand = constant_integer_to_i64(expr.loc, value);
            if (operand == std::numeric_limits<std::int64_t>::min()) {
                fail(expr.loc, "constant integer expression result is out of range for " + type_name(expected));
            }
            return make_signed_integer_constant(expr.loc, expected, -operand);
        }
        if (expr.op == TokenKind::Tilde) {
            if (!is_value_integer_type(expected)) fail(expr.loc, "constant bitwise-not operand must be integer");
            ConstantValue value = evaluate_constant_expr(*expr_operand(expr), expected);
            unsigned width = integer_primitive_bit_width(expected.primitive);
            std::uint64_t result_bits = (~constant_integer_raw_bits(value, width)) & integer_bit_mask(width);
            if (is_signed_integer_primitive(expected.primitive)) {
                return make_signed_integer_constant(expr.loc, expected, sign_extend_integer_bits(result_bits, width));
            }
            return make_unsigned_integer_constant(expr.loc, expected, result_bits);
        }
        fail(expr.loc, "unsupported constant unary operator");
    }

    ConstantValue evaluate_constant_cast_expr(const Expr& expr, const IrType& expected) {
        if (!expr_operand(expr)) fail(expr.loc, "missing constant cast operand");
        IrType target = resolve_executable_type(expr.cast_type);
        require_assignable(expr.loc, expected, target);
        IrType source_type = infer_constant_expr_type(*expr_operand(expr), target);
        ConstantValue value = evaluate_constant_expr(*expr_operand(expr), source_type);
        return cast_integer_constant(expr.loc, value, target);
    }

    ConstantValue coerce_constant_selection(SourceLocation loc, ConstantValue value, const IrType& expected) {
        require_assignable(loc, expected, value.type);
        value.type = expected;
        return value;
    }

    ConstantValue evaluate_constant_field_access_expr(const Expr& expr, const IrType& expected) {
        if (!expr_operand(expr)) fail(expr.loc, "missing constant field operand");
        IrType operand_type = infer_constant_expr_type(*expr_operand(expr), expected);
        if (operand_type.primitive != IrPrimitiveKind::Struct) {
            fail(expr.loc, "constant field access requires a struct value, got " + type_name(operand_type));
        }
        ConstantValue value = evaluate_constant_expr(*expr_operand(expr), operand_type);
        if (value.kind != ConstantValueKind::Struct) {
            fail(expr.loc, "constant field access requires a struct value");
        }
        std::size_t field_index = struct_field_index(expr.loc, operand_type, expr.name);
        if (field_index >= value.elements.size()) {
            throw CompileError("internal error: constant struct field index out of range");
        }
        return coerce_constant_selection(expr.loc, value.elements[field_index], expected);
    }

    ConstantValue evaluate_constant_tuple_index_expr(const Expr& expr, const IrType& expected) {
        if (!expr_operand(expr)) fail(expr.loc, "missing constant tuple index operand");
        IrType operand_type = infer_constant_expr_type(*expr_operand(expr), expected);
        if (operand_type.primitive != IrPrimitiveKind::Tuple &&
            operand_type.primitive != IrPrimitiveKind::Struct) {
            fail(expr.loc, "constant tuple index access requires tuple or tuple struct value, got " +
                           type_name(operand_type));
        }
        ConstantValue value = evaluate_constant_expr(*expr_operand(expr), operand_type);
        if (value.kind != ConstantValueKind::Tuple && value.kind != ConstantValueKind::Struct) {
            fail(expr.loc, "constant tuple index access requires tuple or tuple struct value");
        }
        const std::vector<IrType>& fields = aggregate_field_types(operand_type);
        if (expr.tuple_index >= fields.size()) {
            fail(expr.loc,
                 "tuple index " + std::to_string(expr.tuple_index) +
                     " is out of range for " + type_name(operand_type));
        }
        std::size_t index = static_cast<std::size_t>(expr.tuple_index);
        if (index >= value.elements.size()) {
            throw CompileError("internal error: constant tuple index out of range");
        }
        return coerce_constant_selection(expr.loc, value.elements[index], expected);
    }

    ConstantValue evaluate_constant_index_expr(const Expr& expr, const IrType& expected) {
        if (!expr_operand(expr) || !expr_right(expr)) fail(expr.loc, "missing constant index operand");
        IrType operand_type = infer_constant_expr_type(*expr_operand(expr), expected);
        if (operand_type.primitive != IrPrimitiveKind::Array || operand_type.args.size() != 1) {
            fail(expr.loc, "constant index access requires a fixed-array value, got " + type_name(operand_type));
        }
        ConstantValue value = evaluate_constant_expr(*expr_operand(expr), operand_type);
        if (value.kind != ConstantValueKind::Array) {
            fail(expr.loc, "constant index access requires a fixed-array value");
        }
        IrType index_type = infer_constant_expr_type(*expr_right(expr), i64_type(expr.loc));
        ConstantValue index_value = evaluate_constant_expr(*expr_right(expr), index_type);
        if (index_value.kind != ConstantValueKind::Integer) {
            fail(expr_right(expr)->loc, "array index must be an integer");
        }
        if (index_value.int_negative) fail(expr_right(expr)->loc, "array index must be non-negative");
        std::uint64_t index = constant_integer_to_u64(expr_right(expr)->loc, index_value);
        if (index >= value.elements.size()) {
            fail(expr.loc,
                 "array index " + std::to_string(index) +
                     " is out of range for " + std::to_string(value.elements.size()) + " elements");
        }
        return coerce_constant_selection(
            expr.loc,
            value.elements[static_cast<std::size_t>(index)],
            expected
        );
    }

    std::vector<ConstantValue> evaluate_constant_expr_list(
        SourceLocation loc,
        const std::vector<ExprPtr>& args,
        const std::vector<IrType>& expected_types,
        const std::string& aggregate
    ) {
        if (args.size() != expected_types.size()) {
            fail(loc,
                 aggregate + " constant has " + std::to_string(args.size()) +
                     " element" + (args.size() == 1 ? "" : "s") +
                     " but type expects " + std::to_string(expected_types.size()));
        }

        std::vector<ConstantValue> values;
        values.reserve(args.size());
        for (std::size_t i = 0; i < args.size(); ++i) {
            values.push_back(evaluate_constant_expr(*args[i], expected_types[i]));
        }
        return values;
    }

    ConstantValue make_aggregate_constant(ConstantValueKind kind,
                                          SourceLocation loc,
                                          const IrType& expected,
                                          std::vector<ConstantValue> elements) {
        if (contains_borrow_type(expected) || is_owner_type(expected)) {
            fail(loc, "constant aggregate values cannot contain ownership or borrow-qualified fields yet");
        }
        ConstantValue value;
        value.kind = kind;
        value.type = expected;
        value.elements = std::move(elements);
        return value;
    }

    ConstantValue evaluate_constant_tuple_expr(const Expr& expr, const IrType& expected) {
        if (expected.qualifier != TypeQualifier::Value || expected.primitive != IrPrimitiveKind::Tuple) {
            fail(expr.loc, "tuple constant initializer requires a tuple type, got " + type_name(expected));
        }
        return make_aggregate_constant(
            ConstantValueKind::Tuple,
            expr.loc,
            expected,
            evaluate_constant_expr_list(expr.loc, expr.args, expected.args, "tuple")
        );
    }

    ConstantValue evaluate_constant_array_expr(const Expr& expr, const IrType& expected) {
        if (expected.qualifier != TypeQualifier::Value ||
            expected.primitive != IrPrimitiveKind::Array ||
            expected.args.size() != 1) {
            fail(expr.loc, "fixed-array constant initializer requires a fixed-array type, got " + type_name(expected));
        }
        if (expr.args.size() != expected.array_size) {
            fail(expr.loc,
                 "array constant has " + std::to_string(expr.args.size()) +
                     " element" + (expr.args.size() == 1 ? "" : "s") +
                     " but type expects " + std::to_string(expected.array_size));
        }

        std::vector<IrType> element_types;
        element_types.reserve(expr.args.size());
        for (std::size_t i = 0; i < expr.args.size(); ++i) element_types.push_back(expected.args[0]);
        return make_aggregate_constant(
            ConstantValueKind::Array,
            expr.loc,
            expected,
            evaluate_constant_expr_list(expr.loc, expr.args, element_types, "array")
        );
    }

    ConstantValue evaluate_constant_struct_literal_expr(const Expr& expr, const IrType& expected) {
        if (expected.qualifier != TypeQualifier::Value || expected.primitive != IrPrimitiveKind::Struct) {
            fail(expr.loc, "struct constant initializer requires a struct type, got " + type_name(expected));
        }

        std::vector<IrType> explicit_type_args;
        const ExprTypeArgs& ast_type_args = expr_type_args(expr);
        explicit_type_args.reserve(ast_type_args.size());
        for (const auto& type_arg : ast_type_args) {
            explicit_type_args.push_back(resolve_executable_type(type_arg));
        }
        if (explicit_type_args.empty() && !expected.args.empty()) {
            explicit_type_args = expected.args;
        }
        IrType literal_type = resolve_struct_literal_type(expr.loc, expr.name, explicit_type_args);
        require_assignable(expr.loc, expected, literal_type);
        const ExprFieldNames& field_names = expr_field_names(expr);
        if (literal_type.field_names.size() != field_names.size()) {
            fail(expr.loc,
                 "struct literal for '" + literal_type.name + "' expects " +
                     std::to_string(literal_type.field_names.size()) + " field" +
                     (literal_type.field_names.size() == 1 ? "" : "s"));
        }

        std::map<std::string, const Expr*> values;
        for (std::size_t i = 0; i < field_names.size(); ++i) {
            const std::string& field_name = field_names[i];
            if (!values.emplace(field_name, expr.args[i].get()).second) {
                fail(expr.loc, "duplicate field '" + field_name + "' in struct literal");
            }
        }

        std::vector<ConstantValue> elements;
        elements.reserve(expected.field_names.size());
        for (std::size_t i = 0; i < expected.field_names.size(); ++i) {
            const std::string& field_name = expected.field_names[i];
            auto found = values.find(field_name);
            if (found == values.end()) {
                fail(expr.loc, "missing field '" + field_name + "' in struct literal for '" + expected.name + "'");
            }
            elements.push_back(evaluate_constant_expr(*found->second, expected.field_types[i]));
        }
        return make_aggregate_constant(ConstantValueKind::Struct, expr.loc, expected, std::move(elements));
    }

    ConstantValue evaluate_constant_tuple_struct_call_expr(const Expr& expr,
                                                           const StructInfo& info,
                                                           const IrType& expected) {
        if (!info.tuple_struct) {
            fail(expr.loc, "named struct '" + info.name + "' must be constructed with field literal syntax");
        }
        if (expected.qualifier != TypeQualifier::Value || expected.primitive != IrPrimitiveKind::Struct) {
            fail(expr.loc, "tuple-struct constant initializer requires a struct type, got " + type_name(expected));
        }

        std::vector<IrType> explicit_type_args;
        const ExprTypeArgs& ast_type_args = expr_type_args(expr);
        explicit_type_args.reserve(ast_type_args.size());
        for (const auto& type_arg : ast_type_args) {
            explicit_type_args.push_back(resolve_executable_type(type_arg));
        }
        if (explicit_type_args.empty() && !expected.args.empty()) {
            explicit_type_args = expected.args;
        }
        IrType literal_type = resolve_struct_literal_type(expr.loc, info.name, explicit_type_args);
        require_assignable(expr.loc, expected, literal_type);
        if (expr.args.size() != expected.field_types.size()) {
            fail(expr.loc,
                 "tuple struct '" + info.name + "' expects " + std::to_string(expected.field_types.size()) +
                     " value" + (expected.field_types.size() == 1 ? "" : "s"));
        }

        return make_aggregate_constant(
            ConstantValueKind::Struct,
            expr.loc,
            expected,
            evaluate_constant_expr_list(expr.loc, expr.args, expected.field_types, "tuple struct")
        );
    }

    ConstantValue make_enum_constant(SourceLocation loc,
                                     const IrType& expected,
                                     const EnumCaseInfo& info,
                                     std::vector<ConstantValue> payloads) {
        require_enum_case_access(loc, info);
        require_assignable(loc, expected, info.enum_type);
        if (payloads.size() != info.payloads.size()) {
            fail(loc, "wrong payload count for enum case '" + info.name + "'");
        }
        if (contains_borrow_type(expected) || is_owner_type(expected)) {
            fail(loc, "constant enum values cannot contain ownership or borrow-qualified payloads yet");
        }

        ConstantValue value;
        value.kind = ConstantValueKind::Enum;
        value.type = expected;
        value.elements = std::move(payloads);
        value.enum_name = info.enum_name;
        value.case_name = info.name;
        value.enum_tag = info.tag;
        return value;
    }

    ConstantValue evaluate_constant_enum_case_name(SourceLocation loc,
                                                   const std::string& name,
                                                   const IrType& expected) {
        std::string case_name = resolve_enum_case_name(name);
        auto case_found = enum_cases_.find(case_name);
        if (case_found == enum_cases_.end()) {
            fail(loc, "constant initializer name '" + name + "' must refer to a constant");
        }
        EnumCaseInfo info = case_found->second;
        if (info.is_generic) {
            info = specialize_enum_case_info(loc, info, expected.args);
        }
        if (!info.payloads.empty()) {
            fail(loc, "enum case '" + name + "' requires a payload");
        }
        return make_enum_constant(loc, expected, info, {});
    }

    ConstantValue evaluate_constant_enum_call_expr(const Expr& expr,
                                                   const EnumCaseInfo& info,
                                                   const IrType& expected) {
        EnumCaseInfo case_info = info;
        const ExprTypeArgs& ast_type_args = expr_type_args(expr);
        if (info.is_generic) {
            std::vector<IrType> type_args;
            type_args.reserve(ast_type_args.size());
            for (const auto& type_arg : ast_type_args) {
                type_args.push_back(resolve_executable_type(type_arg));
            }
            if (type_args.empty()) {
                type_args = expected.args;
            }
            case_info = specialize_enum_case_info(expr.loc, info, type_args);
        } else if (!ast_type_args.empty()) {
            fail(expr.loc, "enum case constructor '" + expr.name + "' does not take type arguments");
        }
        return make_enum_constant(
            expr.loc,
            expected,
            case_info,
            evaluate_constant_expr_list(expr.loc, expr.args, case_info.payloads, "enum payload")
        );
    }

    ConstantValue evaluate_constant_expr(const Expr& expr, const IrType& expected) {
        if (is_value_integer_type(expected) && expr.kind == ExprKind::Integer) {
            return make_integer_literal_constant(
                expr.loc,
                expected,
                expr.literal_suffix,
                expr.int_value,
                expr.int_negative
            );
        }

        if (expected.qualifier == TypeQualifier::Value &&
            expected.primitive == IrPrimitiveKind::Bool &&
            expr.kind == ExprKind::Bool) {
            return make_bool_literal_constant(expr.loc, expected, expr.bool_value);
        }

        if (expr.kind == ExprKind::Name) {
            ConstantValue value;
            if (!resolve_constant_value(expr.loc, expr.name, value)) {
                if (expected.qualifier == TypeQualifier::Value && expected.primitive == IrPrimitiveKind::Enum) {
                    return evaluate_constant_enum_case_name(expr.loc, expr.name, expected);
                }
                fail(expr.loc, "constant initializer name '" + expr.name + "' must refer to a constant");
            }
            require_assignable(expr.loc, expected, value.type);
            value.type = expected;
            return value;
        }

        if (expr.kind == ExprKind::Tuple) {
            return evaluate_constant_tuple_expr(expr, expected);
        }

        if (expr.kind == ExprKind::StructLiteral) {
            return evaluate_constant_struct_literal_expr(expr, expected);
        }

        if (expr.kind == ExprKind::Vector) {
            return evaluate_constant_array_expr(expr, expected);
        }

        if (expr.kind == ExprKind::Call) {
            std::string case_name = resolve_enum_case_name(expr.name);
            auto case_found = enum_cases_.find(case_name);
            if (case_found != enum_cases_.end()) {
                return evaluate_constant_enum_call_expr(expr, case_found->second, expected);
            }
            std::string struct_name = resolve_struct_type_name(expr.name);
            auto struct_found = structs_.find(struct_name);
            if (struct_found != structs_.end()) {
                require_struct_access(expr.loc, struct_found->second);
                return evaluate_constant_tuple_struct_call_expr(expr, struct_found->second, expected);
            }
        }

        if (expr.kind == ExprKind::Unary) {
            return evaluate_constant_unary_expr(expr, expected);
        }

        if (expr.kind == ExprKind::Cast) {
            return evaluate_constant_cast_expr(expr, expected);
        }

        if (expr.kind == ExprKind::Binary) {
            return evaluate_constant_binary_expr(expr, expected);
        }

        if (expr.kind == ExprKind::FieldAccess) {
            return evaluate_constant_field_access_expr(expr, expected);
        }

        if (expr.kind == ExprKind::TupleIndex) {
            return evaluate_constant_tuple_index_expr(expr, expected);
        }

        if (expr.kind == ExprKind::Index) {
            return evaluate_constant_index_expr(expr, expected);
        }

        fail(expr.loc, "constant initializers currently support scalar and aggregate constant expressions");
    }

    bool resolve_constant_value(SourceLocation loc, const std::string& name, ConstantValue& value) {
        std::string constant_name = resolve_constant_name(name);
        auto found = constants_.find(constant_name);
        if (found == constants_.end()) return false;
        require_constant_access(loc, found->second, constant_name);
        value = evaluate_constant(found->second);
        return true;
    }

    bool try_constant_pattern_value(const Pattern& pattern, ConstantValue& value) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) return try_constant_pattern_value(effective_pattern, value);
        if (pattern.kind != PatternKind::EnumCase ||
            pattern.has_payload_pattern ||
            pattern.has_payload_binding) {
            return false;
        }
        if (!resolve_constant_value(pattern.loc, pattern.case_name, value)) return false;
        return value.kind == ConstantValueKind::Integer || value.kind == ConstantValueKind::Bool;
    }

    EnumCaseInfo enum_case_for_match_value(
        SourceLocation loc,
        const EnumCaseInfo& info,
        const IrType& enum_value_type
    ) {
        if (info.is_generic) {
            return specialize_enum_case_info(loc, info, enum_value_type.args);
        }
        return info;
    }

    IrMatchArm lower_enum_case_pattern(
        const Pattern& pattern,
        const EnumInfo& enum_info,
        const IrType& enum_value_type
    ) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_enum_case_pattern(effective_pattern, enum_info, enum_value_type);
        }
        if (pattern.kind == PatternKind::Alias) {
            if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
            if (pattern.alias_pattern->kind == PatternKind::Or) {
                fail(pattern.loc, "control-flow let patterns do not expand or-patterns yet; use match for alias-wrapped or-patterns");
            }
            IrMatchArm lowered_arm = lower_enum_case_pattern(*pattern.alias_pattern, enum_info, enum_value_type);
            apply_value_binding(lowered_arm, pattern.loc, pattern.alias_name, enum_match_value_type(pattern.loc, enum_value_type));
            return lowered_arm;
        }
        if (pattern.kind != PatternKind::EnumCase) {
            fail(pattern.loc, "control-flow let patterns require an enum case pattern");
        }

        std::string case_name = resolve_enum_case_name(pattern.case_name);
        auto case_found = enum_cases_.find(case_name);
        if (case_found == enum_cases_.end()) {
            fail(pattern.loc, "unknown enum case '" + pattern.case_name + "'");
        }
        EnumCaseInfo case_info = enum_case_for_match_value(pattern.loc, case_found->second, enum_value_type);
        require_enum_case_access(pattern.loc, case_info);
        if (case_info.enum_name != enum_info.name) {
            fail(pattern.loc, "enum case '" + pattern.case_name + "' does not belong to " + enum_info.name);
        }
        if (case_info.payloads.empty() && pattern.has_payload_pattern) {
            fail(pattern.loc, "enum case '" + pattern.case_name + "' has no payload");
        }
        if (!case_info.payloads.empty() && !pattern.has_payload_pattern) {
            fail(pattern.loc, "enum case '" + pattern.case_name + "' requires a payload pattern");
        }

        IrMatchArm lowered_arm;
        lowered_arm.loc = pattern.loc;
        lowered_arm.case_name = case_info.name;
        lowered_arm.enum_tag = case_info.tag;
        lower_enum_payload_pattern(pattern, case_info, lowered_arm);
        return lowered_arm;
    }

    const EnumInfo& require_enum_match_value(SourceLocation loc, const IrExpr& value) const {
        if (is_borrow_type(value.type)) {
            fail(loc, "borrow expression result must be passed directly to a call");
        }
        if (!is_value_enum_type(value.type)) {
            fail(loc, "pattern control flow requires an enum value, got " + type_name(value.type));
        }
        auto enum_found = enums_.find(value.type.name);
        if (enum_found == enums_.end()) {
            fail(loc, "unknown enum type '" + value.type.name + "'");
        }
        return enum_found->second;
    }

    IrMatchArm lower_match_arm_pattern(const Pattern& pattern,
                                       const EnumInfo& enum_info,
                                       const IrType& enum_value_type,
                                       EnumMatchCoverage& coverage) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_match_arm_pattern(effective_pattern, enum_info, enum_value_type, coverage);
        }
        if (coverage.has_wildcard) fail(pattern.loc, "unreachable match arm after wildcard");

        if (pattern.kind == PatternKind::Alias) {
            if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
            IrMatchArm lowered_arm = lower_match_arm_pattern(*pattern.alias_pattern, enum_info, enum_value_type, coverage);
            apply_value_binding(lowered_arm, pattern.loc, pattern.alias_name, enum_match_value_type(pattern.loc, enum_value_type));
            return lowered_arm;
        }

        IrMatchArm lowered_arm;
        lowered_arm.loc = pattern.loc;

        if (pattern.kind == PatternKind::Wildcard) {
            lowered_arm.wildcard = true;
            coverage.has_wildcard = true;
            return lowered_arm;
        }
        if (pattern.kind != PatternKind::EnumCase) {
            fail(pattern.loc, "enum match patterns must be enum cases or _");
        }

        std::string case_name = resolve_enum_case_name(pattern.case_name);
        auto case_found = enum_cases_.find(case_name);
        if (case_found == enum_cases_.end()) {
            fail(pattern.loc, "unknown enum case '" + pattern.case_name + "'");
        }
        EnumCaseInfo case_info = enum_case_for_match_value(pattern.loc, case_found->second, enum_value_type);
        require_enum_case_access(pattern.loc, case_info);
        if (case_info.enum_name != enum_info.name) {
            fail(pattern.loc,
                 "enum case '" + pattern.case_name + "' does not belong to " + enum_info.name);
        }
        if (case_info.payloads.empty() && pattern.has_payload_pattern) {
            fail(pattern.loc, "enum case '" + pattern.case_name + "' has no payload");
        }
        if (!case_info.payloads.empty() && !pattern.has_payload_pattern) {
            fail(pattern.loc, "enum case '" + pattern.case_name + "' requires a payload pattern");
        }

        lowered_arm.case_name = case_info.name;
        lowered_arm.enum_tag = case_info.tag;
        bool covers_case = lower_enum_payload_pattern(pattern, case_info, lowered_arm);
        note_enum_match_coverage(pattern, case_info, lowered_arm, covers_case, coverage);
        return lowered_arm;
    }

    std::vector<IrMatchArm> lower_match_arm_patterns(const Pattern& pattern,
                                                     const EnumInfo& enum_info,
                                                     const IrType& enum_value_type,
                                                     EnumMatchCoverage& coverage) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_match_arm_patterns(effective_pattern, enum_info, enum_value_type, coverage);
        }
        std::vector<Pattern> alternatives = expand_or_pattern_alternatives(pattern);
        if (pattern_contains_or(pattern)) {
            require_same_or_pattern_bindings(pattern.loc, alternatives, enum_match_value_type(pattern.loc, enum_value_type));
        }

        std::vector<IrMatchArm> arms;
        for (const auto& alternative : alternatives) {
            arms.push_back(lower_match_arm_pattern(alternative, enum_info, enum_value_type, coverage));
        }
        return arms;
    }

    void note_enum_match_coverage(const Pattern& pattern,
                                  const EnumCaseInfo& case_info,
                                  const IrMatchArm& lowered_arm,
                                  bool covers_case,
                                  EnumMatchCoverage& coverage) const {
        bool bool_payload_value = false;
        bool bool_payload_literal = enum_bool_payload_literal_value(
            pattern,
            case_info.payloads,
            bool_payload_value
        );
        EnumCoverageResult result = ari::note_enum_match_coverage(
            coverage,
            lowered_arm,
            covers_case,
            bool_payload_literal,
            bool_payload_value
        );
        if (result == EnumCoverageResult::DuplicateCase) {
            fail(pattern.loc, "duplicate match arm for enum case '" + pattern.case_name + "'");
        }
        if (result == EnumCoverageResult::DuplicatePayloadPattern) {
            fail(pattern.loc, "duplicate match arm for enum payload pattern '" + pattern.case_name + "'");
        }
    }

    using PatternBindingSignature = std::map<std::string, IrType>;

    static void add_pattern_binding(SourceLocation loc,
                                    PatternBindingSignature& bindings,
                                    const std::string& name,
                                    const IrType& type) {
        auto inserted = bindings.emplace(name, type);
        if (!inserted.second) {
            fail(loc, "duplicate binding '" + name + "' in pattern");
        }
    }

    void collect_pattern_bindings(const Pattern& pattern,
                                  const IrType& type,
                                  PatternBindingSignature& bindings) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            collect_pattern_bindings(effective_pattern, type, bindings);
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
                return;
            case PatternKind::Binding:
                add_pattern_binding(pattern.loc, bindings, pattern.payload_name, type);
                return;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                add_pattern_binding(pattern.loc, bindings, pattern.alias_name, type);
                collect_pattern_bindings(*pattern.alias_pattern, type, bindings);
                return;
            case PatternKind::Or:
                fail(pattern.loc, "internal error: or-pattern should be expanded before binding collection");
            case PatternKind::Tuple:
            case PatternKind::Array: {
                if (pattern.kind == PatternKind::Array && is_runtime_sequence_pattern_subject(type)) {
                    const IrType& element_type = runtime_sequence_element_type(pattern.loc, type);
                    if (!pattern.rest_alias_name.empty()) {
                        add_pattern_binding(
                            pattern.rest_alias_loc,
                            bindings,
                            pattern.rest_alias_name,
                            make_prelude_slice_type(pattern.rest_alias_loc, element_type)
                        );
                    }
                    for (const auto& element : pattern.elements) {
                        collect_pattern_bindings(element, element_type, bindings);
                    }
                    return;
                }
                IrPrimitiveKind expected = pattern.kind == PatternKind::Array
                    ? IrPrimitiveKind::Array
                    : IrPrimitiveKind::Tuple;
                if (type.primitive != expected) return;
                const std::vector<IrType>& fields = aggregate_field_types(type);
                require_tuple_pattern_arity(pattern, type, fields);
                for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
                    std::size_t field_index = tuple_pattern_field_index(pattern, fields.size(), i);
                    collect_pattern_bindings(pattern.elements[i], fields[field_index], bindings);
                }
                return;
            }
            case PatternKind::Struct: {
                if (type.primitive != IrPrimitiveKind::Struct) return;
                require_struct_match_pattern_type(pattern.loc, pattern.case_name, type);
                if (!pattern.has_rest && pattern.field_names.size() != type.field_names.size()) {
                    fail(pattern.loc, "struct match pattern must mention all fields or use '..'");
                }
                std::set<std::string> seen_fields;
                for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
                    if (!seen_fields.insert(pattern.field_names[i]).second) {
                        fail(pattern.elements[i].loc, "duplicate field '" + pattern.field_names[i] + "' in struct match pattern");
                    }
                    std::size_t field_index = struct_field_index(pattern.elements[i].loc, type, pattern.field_names[i]);
                    collect_pattern_bindings(pattern.elements[i], type.field_types[field_index], bindings);
                }
                return;
            }
            case PatternKind::EnumCase:
                break;
        }

        if (type.primitive == IrPrimitiveKind::Struct) {
            const StructInfo& info = require_struct_match_pattern_type(pattern.loc, pattern.case_name, type);
            if (!info.tuple_struct) return;
            if (!pattern.has_payload_pattern || !pattern.payload_pattern) return;
            const std::vector<IrType>& fields = aggregate_field_types(type);
            if (pattern.payload_pattern->kind == PatternKind::Tuple) {
                require_tuple_pattern_arity(*pattern.payload_pattern, type, fields);
                for (std::size_t i = 0; i < pattern.payload_pattern->elements.size(); ++i) {
                    std::size_t field_index = tuple_pattern_field_index(*pattern.payload_pattern, fields.size(), i);
                    collect_pattern_bindings(pattern.payload_pattern->elements[i], fields[field_index], bindings);
                }
                return;
            }
            if (fields.size() == 1) collect_pattern_bindings(*pattern.payload_pattern, fields[0], bindings);
            return;
        }

        if (is_value_enum_type(type)) {
            auto enum_found = enums_.find(type.name);
            if (enum_found == enums_.end()) fail(pattern.loc, "unknown enum type '" + type.name + "'");
            std::string case_name = resolve_enum_case_name(pattern.case_name);
            auto case_found = enum_cases_.find(case_name);
            if (case_found == enum_cases_.end()) fail(pattern.loc, "unknown enum case '" + pattern.case_name + "'");
            EnumCaseInfo case_info = enum_case_for_match_value(pattern.loc, case_found->second, type);
            require_enum_case_access(pattern.loc, case_info);
            if (case_info.enum_name != enum_found->second.name) {
                fail(pattern.loc, "enum case '" + pattern.case_name + "' does not belong to " + enum_found->second.name);
            }
            if (case_info.payloads.empty()) return;
            if (!pattern.has_payload_pattern || !pattern.payload_pattern) {
                fail(pattern.loc, "enum case '" + pattern.case_name + "' requires a payload pattern");
            }
            if (case_info.payloads.size() == 1) {
                const Pattern& payload = *pattern.payload_pattern;
                if (payload.kind == PatternKind::Tuple && payload.has_rest && payload.elements.empty()) return;
                collect_pattern_bindings(payload, case_info.payloads[0], bindings);
                return;
            }
            const Pattern& payload = *pattern.payload_pattern;
            if (payload.kind != PatternKind::Tuple) return;
            require_tuple_pattern_arity(payload, case_info.enum_type, case_info.payloads);
            std::size_t suffix_count = payload.has_rest ? payload.elements.size() - payload.rest_index : 0;
            for (std::size_t i = 0; i < payload.elements.size(); ++i) {
                std::size_t payload_index = i;
                if (payload.has_rest && i >= payload.rest_index) {
                    payload_index = case_info.payloads.size() - suffix_count + (i - payload.rest_index);
                }
                collect_pattern_bindings(payload.elements[i], case_info.payloads[payload_index], bindings);
            }
            return;
        }

        ConstantValue constant_pattern;
        if (try_constant_pattern_value(pattern, constant_pattern)) return;
    }

    std::set<std::string> require_same_or_pattern_bindings(SourceLocation loc,
                                                           const std::vector<Pattern>& alternatives,
                                                           const IrType& type) {
        if (alternatives.size() <= 1) return {};

        std::vector<PatternBindingSignature> signatures;
        signatures.reserve(alternatives.size());
        for (const auto& alternative : alternatives) {
            PatternBindingSignature signature;
            collect_pattern_bindings(alternative, type, signature);
            signatures.push_back(std::move(signature));
        }

        const PatternBindingSignature& expected = signatures[0];
        for (std::size_t i = 1; i < signatures.size(); ++i) {
            const PatternBindingSignature& actual = signatures[i];
            if (actual.size() != expected.size()) {
                fail(loc, "or-pattern alternatives must bind the same names");
            }
            for (const auto& [name, expected_type] : expected) {
                auto found = actual.find(name);
                if (found == actual.end()) {
                    fail(loc, "or-pattern alternatives must bind the same names");
                }
                if (!same_type(expected_type, found->second)) {
                    fail(loc,
                         "or-pattern binding '" + name + "' has inconsistent types: " +
                             type_name(expected_type) + " and " + type_name(found->second));
                }
            }
        }

        std::set<std::string> names;
        for (const auto& [name, type] : expected) {
            (void)type;
            names.insert(name);
        }
        return names;
    }

    bool lower_enum_payload_pattern(const Pattern& pattern,
                                    const EnumCaseInfo& case_info,
                                    IrMatchArm& lowered_arm) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_enum_payload_pattern(effective_pattern, case_info, lowered_arm);
        }
        if (case_info.payloads.empty()) return true;
        if (!pattern.payload_pattern) {
            fail(pattern.loc, "enum case '" + pattern.case_name + "' requires a payload pattern");
        }

        const Pattern& payload = *pattern.payload_pattern;
        bool aggregate_layout = has_aggregate_enum_layout(case_info.enum_type);
        if (case_info.payloads.size() > 1) {
            if (payload.kind != PatternKind::Tuple) {
                fail(payload.loc, "multi-payload enum case patterns must use positional payload patterns");
            }
            require_tuple_pattern_arity(payload, case_info.enum_type, case_info.payloads);
            bool covers_case = true;
            std::size_t suffix_count = payload.has_rest ? payload.elements.size() - payload.rest_index : 0;
            for (std::size_t i = 0; i < payload.elements.size(); ++i) {
                std::size_t payload_index = i;
                if (payload.has_rest && i >= payload.rest_index) {
                    payload_index = case_info.payloads.size() - suffix_count + (i - payload.rest_index);
                }
                covers_case = lower_enum_payload_slot_pattern(
                    payload.elements[i],
                    case_info.payloads[payload_index],
                    case_info,
                    lowered_arm,
                    static_cast<std::uint32_t>(payload_index),
                    aggregate_layout
                ) && covers_case;
            }
            return covers_case;
        }

        if (payload.kind == PatternKind::Tuple && payload.has_rest && payload.elements.empty()) {
            return true;
        }
        return lower_enum_payload_slot_pattern(payload, case_info.payloads[0], case_info, lowered_arm, 0, aggregate_layout);
    }

    bool lower_enum_payload_slot_pattern(const Pattern& payload,
                                         const IrType& payload_type,
                                         const EnumCaseInfo& case_info,
                                         IrMatchArm& lowered_arm,
                                         std::uint32_t payload_index,
                                         bool aggregate_layout) {
        const Pattern& effective_payload = expanded_pattern(payload);
        if (&effective_payload != &payload) {
            return lower_enum_payload_slot_pattern(
                effective_payload, payload_type, case_info, lowered_arm, payload_index, aggregate_layout);
        }
        switch (payload.kind) {
            case PatternKind::Binding:
                if (aggregate_layout && is_value_enum_type(payload_type) &&
                    try_lower_nested_zero_payload_case(payload.loc, payload.payload_name, payload_type, lowered_arm, payload_index)) {
                    return false;
                }
                add_payload_binding(lowered_arm, payload_index, payload.payload_name, payload_type);
                return true;
            case PatternKind::Wildcard:
                return true;
            case PatternKind::IntegerLiteral:
                if (aggregate_layout) {
                    lower_aggregate_enum_payload_integer_literal(payload, payload_type, lowered_arm, payload_index);
                    return false;
                }
                lowered_arm.has_literal = true;
                lowered_arm.literal_int = encode_enum_payload_integer_literal(payload, payload_type, case_info.tag);
                lowered_arm.literal_negative = false;
                return false;
            case PatternKind::BoolLiteral:
                if (aggregate_layout) {
                    lower_aggregate_enum_payload_bool_literal(payload.loc, payload.bool_value, payload_type, lowered_arm, payload_index);
                    return false;
                }
                if (payload_type.qualifier != TypeQualifier::Value ||
                    payload_type.primitive != IrPrimitiveKind::Bool) {
                    fail(payload.loc, "bool payload patterns require a bool enum payload");
                }
                lowered_arm.has_literal = true;
                lowered_arm.literal_int = ((payload.bool_value ? 1ULL : 0ULL) << 32) | case_info.tag;
                lowered_arm.literal_negative = false;
                return false;
            case PatternKind::Alias:
                lower_alias_payload_pattern(payload, payload_type, case_info, lowered_arm, payload_index, aggregate_layout);
                return payload.alias_pattern && payload.alias_pattern->kind == PatternKind::Wildcard;
            case PatternKind::Range:
                if (aggregate_layout) {
                    lower_enum_payload_range_pattern(payload, payload_type, lowered_arm, payload_index, false);
                    return false;
                }
                lower_enum_payload_range_pattern(payload, payload_type, lowered_arm, payload_index, true);
                return false;
            case PatternKind::Or:
                fail(payload.loc, "or-pattern enum payloads outside match arms are planned but are not supported yet");
            case PatternKind::EnumCase: {
                if (aggregate_layout && is_value_enum_type(payload_type)) {
                    lower_nested_enum_payload_pattern(payload, payload_type, lowered_arm, payload_index);
                    return false;
                }
                ConstantValue constant_pattern;
                if (try_constant_pattern_value(payload, constant_pattern)) {
                    if (aggregate_layout) {
                        lower_aggregate_enum_payload_constant_pattern(payload.loc, constant_pattern, payload_type, lowered_arm, payload_index);
                        return false;
                    }
                    lower_compact_enum_payload_constant_pattern(payload.loc, constant_pattern, payload_type, case_info.tag, lowered_arm);
                    return false;
                }
                fail(payload.loc, "nested enum payload patterns are planned but are not supported yet");
            }
            case PatternKind::Tuple:
                if (payload.has_rest && payload.elements.empty()) return true;
                fail(payload.loc, "nested tuple enum payload patterns are planned but are not supported yet");
            case PatternKind::Array:
                fail(payload.loc, "array enum payload patterns are planned after aggregate enum payload layout");
            case PatternKind::Struct:
                fail(payload.loc, "struct enum payload patterns are planned after aggregate enum payload layout");
        }
        fail(payload.loc, "unsupported enum payload pattern");
    }

    void lower_alias_payload_pattern(const Pattern& payload,
                                     const IrType& payload_type,
                                     const EnumCaseInfo& case_info,
                                     IrMatchArm& lowered_arm,
                                     std::uint32_t payload_index,
                                     bool aggregate_layout) {
        const Pattern& effective_payload = expanded_pattern(payload);
        if (&effective_payload != &payload) {
            lower_alias_payload_pattern(effective_payload, payload_type, case_info, lowered_arm, payload_index, aggregate_layout);
            return;
        }
        if (!payload.alias_pattern) fail(payload.loc, "missing aliased payload pattern");
        if (pattern_has_binding(*payload.alias_pattern)) {
            fail(payload.loc, "alias payload patterns cannot contain another binding yet");
        }
        add_payload_binding(lowered_arm, payload_index, payload.alias_name, payload_type);

        const Pattern& aliased = *payload.alias_pattern;
        switch (aliased.kind) {
            case PatternKind::Wildcard:
                return;
            case PatternKind::IntegerLiteral:
                if (aggregate_layout) {
                    lower_aggregate_enum_payload_integer_literal(aliased, payload_type, lowered_arm, payload_index);
                    return;
                }
                lowered_arm.has_literal = true;
                lowered_arm.literal_int = encode_enum_payload_integer_literal(aliased, payload_type, case_info.tag);
                lowered_arm.literal_negative = false;
                return;
            case PatternKind::BoolLiteral:
                if (aggregate_layout) {
                    lower_aggregate_enum_payload_bool_literal(aliased.loc, aliased.bool_value, payload_type, lowered_arm, payload_index);
                    return;
                }
                if (payload_type.qualifier != TypeQualifier::Value ||
                    payload_type.primitive != IrPrimitiveKind::Bool) {
                    fail(aliased.loc, "bool payload patterns require a bool enum payload");
                }
                lowered_arm.has_literal = true;
                lowered_arm.literal_int = ((aliased.bool_value ? 1ULL : 0ULL) << 32) | case_info.tag;
                lowered_arm.literal_negative = false;
                return;
            case PatternKind::Range:
                if (aggregate_layout) {
                    lower_enum_payload_range_pattern(aliased, payload_type, lowered_arm, payload_index, false);
                    return;
                }
                lower_enum_payload_range_pattern(aliased, payload_type, lowered_arm, payload_index, true);
                return;
            case PatternKind::Or:
                fail(aliased.loc, "or-pattern enum payloads outside match arms are planned but are not supported yet");
            case PatternKind::EnumCase: {
                if (aggregate_layout && is_value_enum_type(payload_type)) {
                    lower_nested_enum_payload_pattern(aliased, payload_type, lowered_arm, payload_index);
                    return;
                }
                ConstantValue constant_pattern;
                if (try_constant_pattern_value(aliased, constant_pattern)) {
                    if (aggregate_layout) {
                        lower_aggregate_enum_payload_constant_pattern(aliased.loc, constant_pattern, payload_type, lowered_arm, payload_index);
                        return;
                    }
                    lower_compact_enum_payload_constant_pattern(aliased.loc, constant_pattern, payload_type, case_info.tag, lowered_arm);
                    return;
                }
                fail(aliased.loc, "nested enum payload patterns are planned but are not supported yet");
            }
            case PatternKind::Tuple:
                if (aliased.has_rest && aliased.elements.empty()) return;
                fail(aliased.loc, "nested tuple enum payload patterns are planned but are not supported yet");
            case PatternKind::Array:
                fail(aliased.loc, "array enum payload patterns are planned after aggregate enum payload layout");
            case PatternKind::Struct:
                fail(aliased.loc, "struct enum payload patterns are planned after aggregate enum payload layout");
            case PatternKind::Binding:
            case PatternKind::Alias:
                fail(aliased.loc, "alias payload patterns cannot contain another binding yet");
        }
        fail(aliased.loc, "unsupported aliased enum payload pattern");
    }

    static void set_aggregate_enum_payload_literal_condition(IrMatchArm& lowered_arm,
                                                             std::uint32_t payload_index,
                                                             std::uint64_t payload_bits) {
        lowered_arm.payload_literal_conditions.push_back(
            IrPayloadLiteralCondition::integer(payload_index, payload_bits));
    }

    static void lower_aggregate_enum_payload_bool_literal(SourceLocation loc,
                                                          bool value,
                                                          const IrType& payload_type,
                                                          IrMatchArm& lowered_arm,
                                                          std::uint32_t payload_index) {
        if (payload_type.qualifier != TypeQualifier::Value ||
            payload_type.primitive != IrPrimitiveKind::Bool) {
            fail(loc, "bool payload patterns require a bool enum payload");
        }
        lowered_arm.payload_literal_conditions.push_back(
            IrPayloadLiteralCondition::boolean(payload_index, value));
    }

    static void lower_aggregate_enum_payload_integer_literal(const Pattern& pattern,
                                                             const IrType& payload_type,
                                                             IrMatchArm& lowered_arm,
                                                             std::uint32_t payload_index) {
        if (!is_value_integer_type(payload_type)) {
            fail(pattern.loc, "integer payload patterns require an integer enum payload");
        }

        IrExpr literal;
        literal.kind = IrExprKind::Integer;
        literal.loc = pattern.loc;
        literal.int_value = pattern.int_value;
        literal.int_negative = pattern.int_negative;
        literal.type = pattern.literal_suffix.empty()
            ? payload_type
            : integer_literal_suffix_type(pattern.literal_suffix, pattern.loc);
        if (!integer_literal_fits(literal, literal.type)) {
            fail(pattern.loc, "integer literal " + integer_literal_name(literal) +
                              " is out of range for " + type_name(literal.type));
        }
        require_assignable(pattern.loc, payload_type, literal.type);

        set_aggregate_enum_payload_literal_condition(
            lowered_arm,
            payload_index,
            integer_literal_payload_bits(pattern.int_value, pattern.int_negative, payload_type)
        );
    }

    static IrExpr make_payload_range_endpoint_literal(SourceLocation loc,
                                                      std::uint64_t value,
                                                      bool negative,
                                                      const std::string& suffix,
                                                      const IrType& payload_type) {
        IrExpr literal;
        literal.kind = IrExprKind::Integer;
        literal.loc = loc;
        literal.int_value = value;
        literal.int_negative = negative;
        literal.type = suffix.empty()
            ? payload_type
            : integer_literal_suffix_type(suffix, loc);
        if (!integer_literal_fits(literal, literal.type)) {
            fail(loc, "integer literal " + integer_literal_name(literal) +
                      " is out of range for " + type_name(literal.type));
        }
        require_assignable(loc, payload_type, literal.type);
        return literal;
    }

    static void lower_enum_payload_range_pattern(const Pattern& pattern,
                                                 const IrType& payload_type,
                                                 IrMatchArm& lowered_arm,
                                                 std::uint32_t payload_index,
                                                 bool compact_enum_payload) {
        if (!is_value_integer_type(payload_type)) {
            fail(pattern.loc, "range payload patterns require an integer enum payload");
        }
        if (compact_enum_payload && payload_index != 0) {
            fail(pattern.loc, "compact enum payload ranges require a single payload slot");
        }
        make_payload_range_endpoint_literal(
            pattern.loc,
            pattern.int_value,
            pattern.int_negative,
            pattern.literal_suffix,
            payload_type
        );
        make_payload_range_endpoint_literal(
            pattern.loc,
            pattern.range_end_value,
            pattern.range_end_negative,
            pattern.range_end_suffix,
            payload_type
        );
        if (!range_start_le_end(pattern, payload_type)) {
            fail(pattern.loc, "range pattern start must be <= end");
        }

        lowered_arm.payload_range_conditions.push_back(IrPayloadRangeCondition{
            payload_index,
            pattern.int_value,
            pattern.int_negative,
            pattern.range_end_value,
            pattern.range_end_negative,
            pattern.range_inclusive,
            is_unsigned_integer_primitive(payload_type.primitive),
            payload_type,
            compact_enum_payload
        });
    }

    void lower_nested_enum_payload_pattern(const Pattern& pattern,
                                           const IrType& payload_type,
                                           IrMatchArm& lowered_arm,
                                           std::uint32_t payload_index) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_nested_enum_payload_pattern(effective_pattern, payload_type, lowered_arm, payload_index);
            return;
        }
        std::string case_name = resolve_enum_case_name(pattern.case_name);
        auto case_found = enum_cases_.find(case_name);
        if (case_found == enum_cases_.end()) {
            fail(pattern.loc, "unknown nested enum case '" + pattern.case_name + "'");
        }
        EnumCaseInfo case_info = enum_case_for_match_value(pattern.loc, case_found->second, payload_type);
        require_enum_case_access(pattern.loc, case_info);
        if (case_info.enum_name != payload_type.name) {
            fail(pattern.loc,
                 "enum case '" + pattern.case_name + "' does not belong to payload enum " + payload_type.name);
        }
        if (case_info.payloads.empty() && pattern.has_payload_pattern) {
            fail(pattern.loc, "enum case '" + pattern.case_name + "' has no payload");
        }
        if (!case_info.payloads.empty() && !pattern.has_payload_pattern) {
            fail(pattern.loc, "enum case '" + pattern.case_name + "' requires a payload pattern");
        }
        IrPayloadEnumCondition tag_condition;
        tag_condition.index = payload_index;
        tag_condition.enum_type = payload_type;
        tag_condition.tag = case_info.tag;

        if (case_info.payloads.empty()) {
            lowered_arm.payload_enum_conditions.push_back(std::move(tag_condition));
            return;
        }

        if (case_info.payloads.size() == 1) {
            IrPayloadEnumCondition condition = tag_condition;
            lower_nested_enum_payload_slot_pattern(
                *pattern.payload_pattern,
                case_info.payloads[0],
                payload_type,
                lowered_arm,
                payload_index,
                0,
                condition
            );
            lowered_arm.payload_enum_conditions.push_back(std::move(condition));
            return;
        }

        const Pattern& payload = *pattern.payload_pattern;
        if (payload.kind != PatternKind::Tuple) {
            fail(payload.loc, "nested multi-payload enum case patterns must use positional payload patterns");
        }
        require_tuple_pattern_arity(payload, case_info.enum_type, case_info.payloads);

        lowered_arm.payload_enum_conditions.push_back(tag_condition);
        std::size_t suffix_count = payload.has_rest ? payload.elements.size() - payload.rest_index : 0;
        for (std::size_t i = 0; i < payload.elements.size(); ++i) {
            std::size_t nested_payload_index = i;
            if (payload.has_rest && i >= payload.rest_index) {
                nested_payload_index = case_info.payloads.size() - suffix_count + (i - payload.rest_index);
            }

            IrPayloadEnumCondition condition = tag_condition;
            condition.nested_payload_index = static_cast<std::uint32_t>(nested_payload_index);
            lower_nested_enum_payload_slot_pattern(
                payload.elements[i],
                case_info.payloads[nested_payload_index],
                payload_type,
                lowered_arm,
                payload_index,
                static_cast<std::uint32_t>(nested_payload_index),
                condition
            );
            if (condition.has_payload_literal || condition.has_payload_range) {
                lowered_arm.payload_enum_conditions.push_back(std::move(condition));
            }
        }
    }

    bool try_lower_nested_zero_payload_case(SourceLocation loc,
                                            const std::string& name,
                                            const IrType& payload_type,
                                            IrMatchArm& lowered_arm,
                                            std::uint32_t payload_index) {
        std::string case_name = resolve_enum_case_name(name);
        auto case_found = enum_cases_.find(case_name);
        if (case_found == enum_cases_.end()) return false;
        EnumCaseInfo case_info = enum_case_for_match_value(loc, case_found->second, payload_type);
        if (case_info.enum_name != payload_type.name || !case_info.payloads.empty()) return false;
        require_enum_case_access(loc, case_info);

        IrPayloadEnumCondition condition;
        condition.index = payload_index;
        condition.enum_type = payload_type;
        condition.tag = case_info.tag;
        lowered_arm.payload_enum_conditions.push_back(std::move(condition));
        return true;
    }

    void lower_nested_enum_payload_slot_pattern(const Pattern& pattern,
                                                const IrType& nested_payload_type,
                                                const IrType& nested_enum_type,
                                                IrMatchArm& lowered_arm,
                                                std::uint32_t payload_index,
                                                std::uint32_t nested_payload_index,
                                                IrPayloadEnumCondition& condition) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_nested_enum_payload_slot_pattern(
                effective_pattern,
                nested_payload_type,
                nested_enum_type,
                lowered_arm,
                payload_index,
                nested_payload_index,
                condition
            );
            return;
        }
        condition.nested_payload_index = nested_payload_index;
        switch (pattern.kind) {
            case PatternKind::Binding:
                add_compact_enum_payload_binding(
                    lowered_arm, payload_index, pattern.payload_name, nested_payload_type, nested_enum_type, nested_payload_index);
                return;
            case PatternKind::Wildcard:
                return;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased payload pattern");
                if (pattern_has_binding(*pattern.alias_pattern)) {
                    fail(pattern.loc, "alias payload patterns cannot contain another binding yet");
                }
                add_compact_enum_payload_binding(
                    lowered_arm, payload_index, pattern.alias_name, nested_payload_type, nested_enum_type, nested_payload_index);
                lower_nested_enum_payload_slot_pattern(
                    *pattern.alias_pattern,
                    nested_payload_type,
                    nested_enum_type,
                    lowered_arm,
                    payload_index,
                    nested_payload_index,
                    condition
                );
                return;
            case PatternKind::IntegerLiteral:
                set_nested_enum_payload_integer_literal(pattern, nested_payload_type, condition);
                return;
            case PatternKind::BoolLiteral:
                set_nested_enum_payload_bool_literal(pattern.loc, pattern.bool_value, nested_payload_type, condition);
                return;
            case PatternKind::Range:
                set_nested_enum_payload_range(pattern, nested_payload_type, condition);
                return;
            case PatternKind::EnumCase: {
                ConstantValue constant_pattern;
                if (try_constant_pattern_value(pattern, constant_pattern)) {
                    set_nested_enum_payload_constant_literal(pattern.loc, constant_pattern, nested_payload_type, condition);
                    return;
                }
                fail(pattern.loc, "nested enum payload subpatterns deeper than one compact enum are planned but are not supported yet");
            }
            case PatternKind::Or:
                fail(pattern.loc, "or-pattern enum payloads outside match arms are planned but are not supported yet");
            case PatternKind::Tuple:
                if (pattern.has_rest && pattern.elements.empty()) return;
                fail(pattern.loc, "nested tuple enum payload patterns are planned but are not supported yet");
            case PatternKind::Array:
                fail(pattern.loc, "array enum payload patterns are planned after aggregate enum payload layout");
            case PatternKind::Struct:
                fail(pattern.loc, "struct enum payload patterns are planned after aggregate enum payload layout");
        }
        fail(pattern.loc, "unsupported nested enum payload pattern");
    }

    static void set_nested_enum_payload_integer_literal(const Pattern& pattern,
                                                        const IrType& nested_payload_type,
                                                        IrPayloadEnumCondition& condition) {
        if (!is_value_integer_type(nested_payload_type)) {
            fail(pattern.loc, "integer nested enum payload patterns require an integer enum payload");
        }
        make_payload_range_endpoint_literal(
            pattern.loc,
            pattern.int_value,
            pattern.int_negative,
            pattern.literal_suffix,
            nested_payload_type
        );
        condition.has_payload_literal = true;
        condition.payload_literal.integer = pattern.int_value;
        condition.payload_literal_negative = pattern.int_negative;
        condition.payload_literal_is_bool = false;
        condition.payload_type = nested_payload_type;
    }

    static void set_nested_enum_payload_bool_literal(SourceLocation loc,
                                                     bool value,
                                                     const IrType& nested_payload_type,
                                                     IrPayloadEnumCondition& condition) {
        if (nested_payload_type.qualifier != TypeQualifier::Value ||
            nested_payload_type.primitive != IrPrimitiveKind::Bool) {
            fail(loc, "bool nested enum payload patterns require a bool enum payload");
        }
        condition.has_payload_literal = true;
        condition.payload_literal.boolean = value;
        condition.payload_literal_negative = false;
        condition.payload_literal_is_bool = true;
        condition.payload_type = nested_payload_type;
    }

    static void set_nested_enum_payload_range(const Pattern& pattern,
                                              const IrType& nested_payload_type,
                                              IrPayloadEnumCondition& condition) {
        if (!is_value_integer_type(nested_payload_type)) {
            fail(pattern.loc, "range nested enum payload patterns require an integer enum payload");
        }
        make_payload_range_endpoint_literal(
            pattern.loc,
            pattern.int_value,
            pattern.int_negative,
            pattern.literal_suffix,
            nested_payload_type
        );
        make_payload_range_endpoint_literal(
            pattern.loc,
            pattern.range_end_value,
            pattern.range_end_negative,
            pattern.range_end_suffix,
            nested_payload_type
        );
        if (!range_start_le_end(pattern, nested_payload_type)) {
            fail(pattern.loc, "range pattern start must be <= end");
        }
        condition.has_payload_range = true;
        condition.range_start_int = pattern.int_value;
        condition.range_start_negative = pattern.int_negative;
        condition.range_end_int = pattern.range_end_value;
        condition.range_end_negative = pattern.range_end_negative;
        condition.range_inclusive = pattern.range_inclusive;
        condition.range_is_unsigned = is_unsigned_integer_primitive(nested_payload_type.primitive);
        condition.payload_type = nested_payload_type;
    }

    static std::uint64_t encode_enum_payload_integer_literal(const Pattern& pattern,
                                                            const IrType& payload_type,
                                                            std::uint32_t tag) {
        if (!is_value_integer_type(payload_type)) {
            fail(pattern.loc, "integer payload patterns require an integer enum payload");
        }

        IrExpr literal;
        literal.kind = IrExprKind::Integer;
        literal.loc = pattern.loc;
        literal.int_value = pattern.int_value;
        literal.int_negative = pattern.int_negative;
        literal.type = pattern.literal_suffix.empty()
            ? payload_type
            : integer_literal_suffix_type(pattern.literal_suffix, pattern.loc);
        if (!integer_literal_fits(literal, literal.type)) {
            fail(pattern.loc, "integer literal " + integer_literal_name(literal) +
                              " is out of range for " + type_name(literal.type));
        }
        require_assignable(pattern.loc, payload_type, literal.type);

        std::uint64_t payload_bits = integer_literal_payload_bits(
            pattern.int_value,
            pattern.int_negative,
            payload_type
        );
        return (payload_bits << 32) | tag;
    }

    static std::uint64_t integer_literal_payload_bits(std::uint64_t value,
                                                      bool negative,
                                                      const IrType& payload_type) {
        std::uint64_t raw = negative ? 0 - value : value;
        unsigned width = integer_payload_width(payload_type.primitive);
        if (width == 64) return raw;
        std::uint64_t mask = (1ULL << width) - 1;
        raw &= mask;
        if (is_signed_integer_primitive(payload_type.primitive) && (raw & (1ULL << (width - 1)))) {
            raw |= ~mask;
        }
        return raw;
    }

    static unsigned integer_payload_width(IrPrimitiveKind primitive) {
        switch (primitive) {
            case IrPrimitiveKind::I8:
            case IrPrimitiveKind::U8:
                return 8;
            case IrPrimitiveKind::I16:
            case IrPrimitiveKind::U16:
                return 16;
            case IrPrimitiveKind::I32:
            case IrPrimitiveKind::U32:
                return 32;
            case IrPrimitiveKind::I64:
            case IrPrimitiveKind::U64:
                return 64;
            default:
                return 64;
        }
    }

    static void require_match_exhaustive(SourceLocation loc,
                                         const EnumInfo& enum_info,
                                         const EnumMatchCoverage& coverage) {
        std::string message = enum_match_exhaustiveness_error(
            enum_info.name,
            enum_info.case_names.size(),
            coverage
        );
        if (!message.empty()) fail(loc, message);
    }

    static IrType enum_match_value_type(SourceLocation loc, const IrType& enum_value_type) {
        IrType type = enum_value_type;
        type.loc = loc;
        return type;
    }

    static void apply_value_binding(IrMatchArm& arm,
                                    SourceLocation loc,
                                    const std::string& name,
                                    const IrType& type) {
        arm.has_value_binding = true;
        arm.value_name = name;
        arm.value_type = type;
        arm.loc = loc;
    }

    static void add_payload_binding(IrMatchArm& arm,
                                    std::uint32_t index,
                                    const std::string& name,
                                    const IrType& type) {
        IrPayloadBinding binding;
        binding.index = index;
        binding.name = name;
        binding.type = type;
        arm.payload_bindings.push_back(binding);
        if (!arm.has_payload_binding) {
            arm.has_payload_binding = true;
            arm.payload_index = index;
            arm.payload_name = name;
            arm.payload_type = type;
        }
    }

    static void add_compact_enum_payload_binding(IrMatchArm& arm,
                                                 std::uint32_t index,
                                                 const std::string& name,
                                                 const IrType& type,
                                                 const IrType& enum_type,
                                                 std::uint32_t nested_payload_index = 0) {
        IrPayloadBinding binding;
        binding.index = index;
        binding.name = name;
        binding.type = type;
        binding.compact_enum_payload = true;
        binding.compact_enum_type = enum_type;
        binding.compact_enum_payload_index = nested_payload_index;
        arm.payload_bindings.push_back(binding);
        if (!arm.has_payload_binding) {
            arm.has_payload_binding = true;
            arm.payload_index = index;
            arm.payload_name = name;
            arm.payload_type = type;
        }
    }

    template <typename Arm>
    void declare_match_arm_bindings(const Arm& arm) {
        if (arm.has_value_binding) {
            declare_local(arm.loc, arm.value_name, arm.value_type, false);
        }
        if (!arm.payload_bindings.empty()) {
            for (const auto& binding : arm.payload_bindings) {
                declare_local(arm.loc, binding.name, binding.type, false);
            }
        } else if (arm.has_payload_binding) {
            declare_local(arm.loc, arm.payload_name, arm.payload_type, false);
        }
    }

    IrMatchArm lower_scalar_constant_match_arm_pattern(const Pattern& pattern,
                                                       const ConstantValue& value,
                                                       const IrType& match_type,
                                                       ScalarMatchCoverage& coverage) {
        IrMatchArm lowered_arm = make_scalar_constant_match_arm(pattern.loc, value, match_type);

        if (match_type.qualifier == TypeQualifier::Value && match_type.primitive == IrPrimitiveKind::Bool) {
            std::string key = value.bool_value ? "true" : "false";
            if (coverage.covered_patterns.count(key)) warn_scalar_match_shadow(pattern.loc);
            coverage.covered_patterns.insert(key);
            return lowered_arm;
        }

        std::uint64_t point = integer_pattern_order_value(value.int_value, value.int_negative, match_type);
        if (integer_interval_is_fully_covered(coverage, point, point)) {
            warn_scalar_match_shadow(pattern.loc);
        }
        note_integer_coverage(coverage, match_type, value.int_value, value.int_negative);
        return lowered_arm;
    }

    IrMatchArm lower_scalar_match_arm_pattern(const Pattern& pattern,
                                              const IrType& match_type,
                                              ScalarMatchCoverage& coverage) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_scalar_match_arm_pattern(effective_pattern, match_type, coverage);
        }
        if (coverage.has_wildcard) fail(pattern.loc, "unreachable match arm after wildcard");

        if (pattern.kind == PatternKind::Alias) {
            if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
            IrMatchArm lowered_arm = lower_scalar_match_arm_pattern(*pattern.alias_pattern, match_type, coverage);
            apply_value_binding(lowered_arm, pattern.loc, pattern.alias_name, match_type);
            return lowered_arm;
        }

        IrMatchArm lowered_arm;
        lowered_arm.loc = pattern.loc;
        if (pattern.kind == PatternKind::Wildcard) {
            lowered_arm.wildcard = true;
            coverage.has_wildcard = true;
            return lowered_arm;
        }
        if (pattern.kind == PatternKind::Binding) {
            lowered_arm.wildcard = true;
            coverage.has_wildcard = true;
            apply_value_binding(lowered_arm, pattern.loc, pattern.payload_name, match_type);
            return lowered_arm;
        }

        ConstantValue constant_pattern;
        if (try_constant_pattern_value(pattern, constant_pattern)) {
            return lower_scalar_constant_match_arm_pattern(pattern, constant_pattern, match_type, coverage);
        }

        if (match_type.qualifier == TypeQualifier::Value && match_type.primitive == IrPrimitiveKind::Bool) {
            if (pattern.kind != PatternKind::BoolLiteral) {
                fail(pattern.loc, "bool match patterns must be true, false, or _");
            }
            std::string key = pattern.bool_value ? "true" : "false";
            if (coverage.covered_patterns.count(key)) warn_scalar_match_shadow(pattern.loc);
            coverage.covered_patterns.insert(key);
            lowered_arm.has_literal = true;
            lowered_arm.literal_is_bool = true;
            lowered_arm.literal_bool = pattern.bool_value;
            return lowered_arm;
        }

        if (!is_value_integer_type(match_type)) {
            fail(pattern.loc, "literal match patterns require integer or bool match values");
        }
        if (pattern.kind == PatternKind::Range) {
            return lower_integer_range_match_arm_pattern(pattern, match_type, coverage);
        }
        if (pattern.kind != PatternKind::IntegerLiteral) {
            fail(pattern.loc, "integer match patterns must be integer literals or _");
        }

        IrExpr literal;
        literal.kind = IrExprKind::Integer;
        literal.loc = pattern.loc;
        literal.int_value = pattern.int_value;
        literal.int_negative = pattern.int_negative;
        literal.type = pattern.literal_suffix.empty()
            ? match_type
            : integer_literal_suffix_type(pattern.literal_suffix, pattern.loc);
        if (!integer_literal_fits(literal, literal.type)) {
            fail(pattern.loc, "integer literal " + integer_literal_name(literal) +
                              " is out of range for " + type_name(literal.type));
        }
        require_assignable(pattern.loc, match_type, literal.type);

        std::string key = integer_literal_name(literal);
        std::uint64_t point = integer_pattern_order_value(pattern.int_value, pattern.int_negative, match_type);
        if (integer_interval_is_fully_covered(coverage, point, point)) {
            warn_scalar_match_shadow(pattern.loc);
        }
        coverage.covered_patterns.insert(key);
        note_integer_coverage(coverage, match_type, pattern.int_value, pattern.int_negative);
        lowered_arm.has_literal = true;
        lowered_arm.literal_int = pattern.int_value;
        lowered_arm.literal_negative = pattern.int_negative;
        return lowered_arm;
    }

    IrMatchArm lower_integer_range_match_arm_pattern(const Pattern& pattern,
                                                     const IrType& match_type,
                                                     ScalarMatchCoverage& coverage) {
        IrExpr start = make_pattern_integer_literal(
            pattern.loc, pattern.int_value, pattern.int_negative, pattern.literal_suffix, match_type);
        IrExpr end = make_pattern_integer_literal(
            pattern.loc, pattern.range_end_value, pattern.range_end_negative, pattern.range_end_suffix, match_type);
        if (!integer_literal_fits(start, start.type)) {
            fail(pattern.loc, "integer literal " + integer_literal_name(start) +
                              " is out of range for " + type_name(start.type));
        }
        if (!integer_literal_fits(end, end.type)) {
            fail(pattern.loc, "integer literal " + integer_literal_name(end) +
                              " is out of range for " + type_name(end.type));
        }
        require_assignable(pattern.loc, match_type, start.type);
        require_assignable(pattern.loc, match_type, end.type);
        if (!range_start_le_end(pattern, match_type)) {
            fail(pattern.loc, "range pattern start must be <= end");
        }

        std::uint64_t covered_start = 0;
        std::uint64_t covered_end = 0;
        if (integer_range_coverage_interval(pattern, match_type, covered_start, covered_end) &&
            integer_interval_is_fully_covered(coverage, covered_start, covered_end)) {
            warn_scalar_match_shadow(pattern.loc);
        }
        note_integer_range_coverage(coverage, match_type, pattern);

        IrMatchArm lowered_arm;
        lowered_arm.loc = pattern.loc;
        lowered_arm.has_range = true;
        lowered_arm.range_start_int = pattern.int_value;
        lowered_arm.range_start_negative = pattern.int_negative;
        lowered_arm.range_end_int = pattern.range_end_value;
        lowered_arm.range_end_negative = pattern.range_end_negative;
        lowered_arm.range_inclusive = pattern.range_inclusive;
        lowered_arm.range_is_unsigned = is_unsigned_integer_primitive(match_type.primitive);
        return lowered_arm;
    }

    static void require_scalar_match_exhaustive(SourceLocation loc,
                                                const IrType& match_type,
                                                const ScalarMatchCoverage& coverage) {
        std::string message = scalar_match_exhaustiveness_error(match_type, coverage);
        if (!message.empty()) fail(loc, message);
    }

    std::vector<IrMatchArm> lower_scalar_match_arm_patterns(const Pattern& pattern,
                                                            const IrType& match_type,
                                                            ScalarMatchCoverage& coverage) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_scalar_match_arm_patterns(effective_pattern, match_type, coverage);
        }
        std::vector<Pattern> alternatives = expand_or_pattern_alternatives(pattern);
        if (pattern_contains_or(pattern)) {
            require_same_or_pattern_bindings(pattern.loc, alternatives, match_type);
        }

        std::vector<IrMatchArm> arms;
        for (const auto& alternative : alternatives) {
            arms.push_back(lower_scalar_match_arm_pattern(alternative, match_type, coverage));
        }
        return arms;
    }

    IrExprPtr make_pattern_integer_expr(const Pattern& pattern, const IrType& expected) const {
        IrExpr literal = make_pattern_integer_literal(
            pattern.loc,
            pattern.int_value,
            pattern.int_negative,
            pattern.literal_suffix,
            expected
        );
        if (!integer_literal_fits(literal, literal.type)) {
            fail(pattern.loc, "integer literal " + integer_literal_name(literal) +
                              " is out of range for " + type_name(literal.type));
        }
        require_assignable(pattern.loc, expected, literal.type);

        auto expr = std::make_unique<IrExpr>();
        *expr = std::move(literal);
        return expr;
    }

    IrExprPtr make_pattern_range_endpoint_expr(SourceLocation loc,
                                               std::uint64_t value,
                                               bool negative,
                                               const std::string& suffix,
                                               const IrType& expected) const {
        IrExpr literal = make_pattern_integer_literal(loc, value, negative, suffix, expected);
        if (!integer_literal_fits(literal, literal.type)) {
            fail(loc, "integer literal " + integer_literal_name(literal) +
                      " is out of range for " + type_name(literal.type));
        }
        require_assignable(loc, expected, literal.type);

        auto expr = std::make_unique<IrExpr>();
        *expr = std::move(literal);
        return expr;
    }

    IrExprPtr combine_tuple_match_conditions(SourceLocation loc, IrBinaryOp op, IrExprPtr left, IrExprPtr right) const {
        if (!left) return right;
        if (!right) return left;
        return make_bool_binary_expr(loc, op, std::move(left), std::move(right));
    }

    static void require_tuple_pattern_arity(const Pattern& pattern,
                                            const IrType& source_type,
                                            const std::vector<IrType>& fields) {
        const char* pattern_name = pattern.kind == PatternKind::Array ? "array" : "tuple";
        if (!pattern.rest_alias_name.empty()) {
            fail(pattern.rest_alias_loc,
                 std::string(pattern_name) +
                     " rest bindings currently require a Vec[T] or Slice[T] value");
        }
        if (pattern.has_rest) {
            if (pattern.elements.size() > fields.size()) {
                fail(pattern.loc,
                     std::string(pattern_name) + " match pattern has " + std::to_string(pattern.elements.size()) +
                     " non-rest elements but value has " + std::to_string(fields.size()));
            }
            return;
        }
        if (pattern.elements.size() != fields.size()) {
            fail(pattern.loc,
                 std::string(pattern_name) + " match pattern has " + std::to_string(pattern.elements.size()) +
                 " elements but value has " + std::to_string(fields.size()));
        }
        (void)source_type;
    }

    IrExprPtr lower_tuple_element_match_condition(const Pattern& pattern,
                                                  const std::string& source_name,
                                                  const IrType& source_type,
                                                  std::size_t field_index,
                                                  std::vector<IrStmtPtr>& prelude) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_tuple_element_match_condition(effective_pattern, source_name, source_type, field_index, prelude);
        }
        const std::vector<IrType>& fields = aggregate_field_types(source_type);
        const IrType& field_type = fields[field_index];

        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::Binding:
                return nullptr;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                return lower_tuple_element_match_condition(
                    *pattern.alias_pattern,
                    source_name,
                    source_type,
                    field_index,
                    prelude
                );
            case PatternKind::IntegerLiteral:
                if (!is_value_integer_type(field_type)) {
                    fail(pattern.loc, "integer tuple patterns require an integer tuple field");
                }
                return make_bool_binary_expr(
                    pattern.loc,
                    IrBinaryOp::Eq,
                    make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                    make_pattern_integer_expr(pattern, field_type)
                );
            case PatternKind::BoolLiteral:
                if (field_type.qualifier != TypeQualifier::Value ||
                    field_type.primitive != IrPrimitiveKind::Bool) {
                    fail(pattern.loc, "bool tuple patterns require a bool tuple field");
                }
                return make_bool_binary_expr(
                    pattern.loc,
                    IrBinaryOp::Eq,
                    make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                    make_bool_literal_expr(pattern.loc, pattern.bool_value)
                );
            case PatternKind::Range: {
                if (!is_value_integer_type(field_type)) {
                    fail(pattern.loc, "range tuple patterns require an integer tuple field");
                }
                if (!range_start_le_end(pattern, field_type)) {
                    fail(pattern.loc, "range pattern start must be <= end");
                }
                IrExprPtr lower = make_bool_binary_expr(
                    pattern.loc,
                    IrBinaryOp::Ge,
                    make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                    make_pattern_range_endpoint_expr(
                        pattern.loc,
                        pattern.int_value,
                        pattern.int_negative,
                        pattern.literal_suffix,
                        field_type
                    )
                );
                IrExprPtr upper = make_bool_binary_expr(
                    pattern.loc,
                    pattern.range_inclusive ? IrBinaryOp::Le : IrBinaryOp::Lt,
                    make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                    make_pattern_range_endpoint_expr(
                        pattern.loc,
                        pattern.range_end_value,
                        pattern.range_end_negative,
                        pattern.range_end_suffix,
                        field_type
                    )
                );
                return combine_tuple_match_conditions(pattern.loc, IrBinaryOp::LogicalAnd, std::move(lower), std::move(upper));
            }
            case PatternKind::Or: {
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                IrExprPtr condition;
                for (const auto& alternative : pattern.alternatives) {
                    IrExprPtr alternative_condition = lower_tuple_element_match_condition(
                        alternative,
                        source_name,
                        source_type,
                        field_index,
                        prelude
                    );
                    if (!alternative_condition) return nullptr;
                    condition = combine_tuple_match_conditions(
                        pattern.loc,
                        IrBinaryOp::LogicalOr,
                        std::move(condition),
                        std::move(alternative_condition)
                    );
                }
                return condition;
            }
            case PatternKind::EnumCase:
                {
                    ConstantValue constant_pattern;
                    if (try_constant_pattern_value(pattern, constant_pattern)) {
                        if (field_type.qualifier == TypeQualifier::Value &&
                            field_type.primitive == IrPrimitiveKind::Bool) {
                            if (!constant_pattern.is_bool) {
                                fail(pattern.loc, "bool tuple constant pattern must have type bool");
                            }
                        } else if (is_value_integer_type(field_type)) {
                            if (constant_pattern.is_bool || !is_value_integer_type(constant_pattern.type)) {
                                fail(pattern.loc, "integer tuple constant pattern must have an integer type");
                            }
                            require_assignable(pattern.loc, field_type, constant_pattern.type);
                        } else {
                            fail(pattern.loc, "constant tuple patterns require integer or bool fields");
                        }
                        return make_bool_binary_expr(
                            pattern.loc,
                            IrBinaryOp::Eq,
                            make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                            make_constant_expr(pattern.loc, constant_pattern)
                        );
                    }
                }
                if (field_type.primitive != IrPrimitiveKind::Struct) {
                    fail(pattern.loc, "tuple-struct tuple element patterns require a tuple-struct field");
                }
                {
                    std::string nested_name = make_hidden_local("$match_struct");
                    declare_local(pattern.loc, nested_name, field_type, false);
                    prelude.push_back(make_ir_var_decl(
                        pattern.loc,
                        nested_name,
                        field_type,
                        make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                        false
                    ));
                    return lower_struct_match_pattern_condition(pattern, nested_name, field_type, prelude);
                }
            case PatternKind::Tuple: {
                if (field_type.primitive != IrPrimitiveKind::Tuple) {
                    fail(pattern.loc, "nested tuple pattern requires a tuple field, got " + type_name(field_type));
                }
                std::string nested_name = make_hidden_local("$match_tuple");
                declare_local(pattern.loc, nested_name, field_type, false);
                prelude.push_back(make_ir_var_decl(
                    pattern.loc,
                    nested_name,
                    field_type,
                    make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                    false
                ));
                return lower_tuple_match_pattern_condition(pattern, nested_name, field_type, prelude);
            }
            case PatternKind::Array: {
                if (field_type.primitive != IrPrimitiveKind::Array) {
                    if (is_runtime_sequence_pattern_subject(field_type)) {
                        std::string nested_name = make_hidden_local("$match_array");
                        declare_local(pattern.loc, nested_name, field_type, false);
                        prelude.push_back(make_ir_var_decl(
                            pattern.loc,
                            nested_name,
                            field_type,
                            make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                            false
                        ));
                        return lower_runtime_sequence_match_pattern_condition(
                            pattern, nested_name, field_type, prelude);
                    }
                    fail(pattern.loc, "nested array pattern requires an array field, got " + type_name(field_type));
                }
                std::string nested_name = make_hidden_local("$match_array");
                declare_local(pattern.loc, nested_name, field_type, false);
                prelude.push_back(make_ir_var_decl(
                    pattern.loc,
                    nested_name,
                    field_type,
                    make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                    false
                ));
                return lower_tuple_match_pattern_condition(pattern, nested_name, field_type, prelude);
            }
            case PatternKind::Struct: {
                if (field_type.primitive != IrPrimitiveKind::Struct) {
                    fail(pattern.loc, "struct tuple element patterns require a struct field");
                }
                std::string nested_name = make_hidden_local("$match_struct");
                declare_local(pattern.loc, nested_name, field_type, false);
                prelude.push_back(make_ir_var_decl(
                    pattern.loc,
                    nested_name,
                    field_type,
                    make_tuple_index_expr(pattern.loc, source_name, source_type, field_index),
                    false
                ));
                return lower_struct_match_pattern_condition(pattern, nested_name, field_type, prelude);
            }
        }
        fail(pattern.loc, "unsupported tuple element pattern");
    }

    IrExprPtr lower_positional_product_match_pattern_condition(const Pattern& pattern,
                                                               const std::string& source_name,
                                                               const IrType& source_type,
                                                               std::vector<IrStmtPtr>& prelude) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_positional_product_match_pattern_condition(effective_pattern, source_name, source_type, prelude);
        }
        const std::vector<IrType>& fields = aggregate_field_types(source_type);
        require_tuple_pattern_arity(pattern, source_type, fields);

        IrExprPtr condition;
        for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
            std::size_t field_index = tuple_pattern_field_index(pattern, fields.size(), i);
            IrExprPtr field_condition = lower_tuple_element_match_condition(
                pattern.elements[i],
                source_name,
                source_type,
                field_index,
                prelude
            );
            condition = combine_tuple_match_conditions(
                pattern.elements[i].loc,
                IrBinaryOp::LogicalAnd,
                std::move(condition),
                std::move(field_condition)
            );
        }
        return condition;
    }

    IrExprPtr lower_tuple_match_pattern_condition(const Pattern& pattern,
                                                  const std::string& source_name,
                                                  const IrType& source_type,
                                                  std::vector<IrStmtPtr>& prelude) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_tuple_match_pattern_condition(effective_pattern, source_name, source_type, prelude);
        }
        if (is_runtime_sequence_pattern_subject(source_type)) {
            return lower_runtime_sequence_match_pattern_condition(pattern, source_name, source_type, prelude);
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::Binding:
                return nullptr;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                return lower_tuple_match_pattern_condition(*pattern.alias_pattern, source_name, source_type, prelude);
            case PatternKind::Or: {
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                IrExprPtr condition;
                for (const auto& alternative : pattern.alternatives) {
                    IrExprPtr alternative_condition = lower_tuple_match_pattern_condition(
                        alternative,
                        source_name,
                        source_type,
                        prelude
                    );
                    if (!alternative_condition) return nullptr;
                    condition = combine_tuple_match_conditions(
                        pattern.loc,
                        IrBinaryOp::LogicalOr,
                        std::move(condition),
                        std::move(alternative_condition)
                    );
                }
                return condition;
            }
            case PatternKind::Tuple:
            case PatternKind::Array:
                break;
            default:
                fail(pattern.loc, "tuple and array match patterns must use matching positional patterns or _");
        }

        bool array_pattern = pattern.kind == PatternKind::Array;
        IrPrimitiveKind expected_primitive = array_pattern ? IrPrimitiveKind::Array : IrPrimitiveKind::Tuple;
        const char* pattern_name = array_pattern ? "array" : "tuple";
        if (source_type.primitive != expected_primitive) {
            fail(pattern.loc, std::string(pattern_name) + " match pattern requires a " +
                              pattern_name + " value, got " + type_name(source_type));
        }

        return lower_positional_product_match_pattern_condition(pattern, source_name, source_type, prelude);
    }

    const StructInfo& require_struct_match_pattern_type(SourceLocation loc,
                                                        const std::string& name,
                                                        const IrType& source_type) const {
        if (source_type.primitive != IrPrimitiveKind::Struct) {
            fail(loc, "struct match pattern requires a struct value, got " + type_name(source_type));
        }
        std::string struct_name = resolve_struct_type_name(name);
        auto struct_found = structs_.find(struct_name);
        if (struct_found == structs_.end()) {
            fail(loc, "unknown struct '" + name + "' in match pattern");
        }
        require_struct_access(loc, struct_found->second);
        if (struct_name != source_type.name) {
            fail(loc, "struct match pattern type '" + struct_name + "' does not match value type " + type_name(source_type));
        }
        return struct_found->second;
    }

    IrExprPtr lower_tuple_struct_match_pattern_condition(const Pattern& pattern,
                                                         const std::string& source_name,
                                                         const IrType& source_type,
                                                         std::vector<IrStmtPtr>& prelude) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_tuple_struct_match_pattern_condition(effective_pattern, source_name, source_type, prelude);
        }
        const StructInfo& info = require_struct_match_pattern_type(pattern.loc, pattern.case_name, source_type);
        if (!info.tuple_struct) {
            fail(pattern.loc, "tuple-struct pattern requires a tuple struct");
        }
        if (!pattern.has_payload_pattern || !pattern.payload_pattern) {
            fail(pattern.loc, "tuple-struct pattern '" + pattern.case_name + "' requires positional fields");
        }
        const std::vector<IrType>& fields = aggregate_field_types(source_type);
        const Pattern& payload = *pattern.payload_pattern;
        if (payload.kind == PatternKind::Tuple) {
            return lower_positional_product_match_pattern_condition(payload, source_name, source_type, prelude);
        }
        if (fields.size() != 1) {
            fail(payload.loc,
                 "tuple-struct pattern for '" + info.name + "' has 1 field but value has " +
                     std::to_string(fields.size()));
        }
        return lower_tuple_element_match_condition(payload, source_name, source_type, 0, prelude);
    }

    IrExprPtr lower_struct_match_pattern_condition(const Pattern& pattern,
                                                   const std::string& source_name,
                                                   const IrType& source_type,
                                                   std::vector<IrStmtPtr>& prelude) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_struct_match_pattern_condition(effective_pattern, source_name, source_type, prelude);
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::Binding:
                return nullptr;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                return lower_struct_match_pattern_condition(*pattern.alias_pattern, source_name, source_type, prelude);
            case PatternKind::Or: {
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                IrExprPtr condition;
                for (const auto& alternative : pattern.alternatives) {
                    IrExprPtr alternative_condition = lower_struct_match_pattern_condition(
                        alternative,
                        source_name,
                        source_type,
                        prelude
                    );
                    if (!alternative_condition) return nullptr;
                    condition = combine_tuple_match_conditions(
                        pattern.loc,
                        IrBinaryOp::LogicalOr,
                        std::move(condition),
                        std::move(alternative_condition)
                    );
                }
                return condition;
            }
            case PatternKind::EnumCase:
                return lower_tuple_struct_match_pattern_condition(pattern, source_name, source_type, prelude);
            case PatternKind::Struct:
                break;
            default:
                fail(pattern.loc, "struct match patterns must be struct patterns, tuple-struct patterns, or _");
        }

        require_struct_match_pattern_type(pattern.loc, pattern.case_name, source_type);
        if (pattern.field_names.size() != pattern.elements.size()) {
            throw CompileError("internal error: struct match pattern field/value arity mismatch");
        }
        if (!pattern.has_rest && pattern.field_names.size() != source_type.field_names.size()) {
            fail(pattern.loc, "struct match pattern must mention all fields or use '..'");
        }

        std::set<std::string> seen_fields;
        IrExprPtr condition;
        for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
            const std::string& field_name = pattern.field_names[i];
            if (!seen_fields.insert(field_name).second) {
                fail(pattern.elements[i].loc, "duplicate field '" + field_name + "' in struct match pattern");
            }
            std::size_t field_index = struct_field_index(pattern.elements[i].loc, source_type, field_name);
            IrExprPtr field_condition = lower_tuple_element_match_condition(
                pattern.elements[i],
                source_name,
                source_type,
                field_index,
                prelude
            );
            condition = combine_tuple_match_conditions(
                pattern.elements[i].loc,
                IrBinaryOp::LogicalAnd,
                std::move(condition),
                std::move(field_condition)
            );
        }
        return condition;
    }

    IrExprPtr lower_product_match_pattern_condition(const Pattern& pattern,
                                                    const std::string& source_name,
                                                    const IrType& source_type,
                                                    std::vector<IrStmtPtr>& prelude) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            return lower_product_match_pattern_condition(effective_pattern, source_name, source_type, prelude);
        }
        if (is_runtime_sequence_pattern_subject(source_type)) {
            return lower_runtime_sequence_match_pattern_condition(pattern, source_name, source_type, prelude);
        }
        if (source_type.primitive == IrPrimitiveKind::Tuple) {
            return lower_tuple_match_pattern_condition(pattern, source_name, source_type, prelude);
        }
        if (source_type.primitive == IrPrimitiveKind::Array) {
            return lower_tuple_match_pattern_condition(pattern, source_name, source_type, prelude);
        }
        if (source_type.primitive == IrPrimitiveKind::Struct) {
            return lower_struct_match_pattern_condition(pattern, source_name, source_type, prelude);
        }
        fail(pattern.loc, "aggregate match pattern requires tuple, array, or struct value");
    }

    void initialize_product_match_coverage(const IrType& subject_type, ProductMatchCoverage& coverage) {
        if (!coverage.checked_finite_universe) {
            coverage.checked_finite_universe = true;
            std::vector<std::string> universe;
            if (finite_product_coverage_domain(subject_type, universe)) {
                coverage.has_finite_universe = true;
                coverage.universe_size = universe.size();
            }
        }
        if (!coverage.checked_symbolic_universe) {
            coverage.checked_symbolic_universe = true;
            ProductRect universe;
            if (symbolic_product_coverage_domain(subject_type, universe)) {
                coverage.has_symbolic_universe = true;
                coverage.symbolic_universe = std::move(universe);
            }
        }
    }

    ProductPatternCoverageHooks product_pattern_coverage_hooks() {
        ProductPatternCoverageHooks hooks;
        hooks.try_constant_pattern_value = [this](const Pattern& pattern, ConstantValue& value) {
            return try_constant_pattern_value(pattern, value);
        };
        hooks.tuple_struct_pattern_matches = [this](SourceLocation loc,
                                                    const std::string& name,
                                                    const IrType& type) {
            const StructInfo& info = require_struct_match_pattern_type(loc, name, type);
            return info.tuple_struct;
        };
        hooks.require_struct_pattern_matches = [this](SourceLocation loc,
                                                      const std::string& name,
                                                      const IrType& type) {
            (void)require_struct_match_pattern_type(loc, name, type);
        };
        hooks.struct_field_index = [this](SourceLocation loc,
                                          const IrType& type,
                                          const std::string& field_name) {
            return struct_field_index(loc, type, field_name);
        };
        return hooks;
    }

    ForPatternValidationHooks for_pattern_validation_hooks() {
        ForPatternValidationHooks hooks;
        hooks.require_struct_pattern_type = [this](SourceLocation loc,
                                                   const std::string& name,
                                                   const IrType& type) {
            const StructInfo& info = require_struct_match_pattern_type(loc, name, type);
            ForPatternStructInfo out;
            out.name = info.name;
            out.tuple_struct = info.tuple_struct;
            return out;
        };
        hooks.struct_field_index = [this](SourceLocation loc,
                                          const IrType& type,
                                          const std::string& field_name) {
            return struct_field_index(loc, type, field_name);
        };
        return hooks;
    }

    void note_product_match_coverage(const Pattern& pattern,
                                     const IrType& subject_type,
                                     ProductMatchCoverage& coverage) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            note_product_match_coverage(effective_pattern, subject_type, coverage);
            return;
        }
        initialize_product_match_coverage(subject_type, coverage);
        ProductPatternCoverageHooks hooks = product_pattern_coverage_hooks();
        bool finite_handled = false;
        if (coverage.has_finite_universe) {
            std::vector<std::string> values;
            if (finite_product_pattern_values(pattern, subject_type, hooks, values)) {
                finite_handled = true;
                if (note_finite_product_match_coverage(coverage, values)) {
                    warn_aggregate_match_shadow(pattern.loc);
                }
            }
        }

        if (!coverage.has_symbolic_universe) return;
        std::vector<ProductRect> rects;
        if (!symbolic_product_pattern_rects(pattern, subject_type, hooks, rects)) return;
        if (note_symbolic_product_match_coverage(coverage, rects, finite_handled)) {
            warn_aggregate_match_shadow(pattern.loc);
        }
    }

    void lower_tuple_match_value_bindings(const Pattern& pattern,
                                          const IrType& value_type,
                                          IrExprPtr value,
                                          std::vector<IrStmtPtr>& statements,
                                          bool mutable_binding = false) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_tuple_match_value_bindings(effective_pattern, value_type, std::move(value), statements, mutable_binding);
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
                return;
            case PatternKind::EnumCase:
                if (value_type.primitive == IrPrimitiveKind::Struct) {
                    std::string nested_name = make_hidden_local("$pattern");
                    declare_local(pattern.loc, nested_name, value_type, false);
                    statements.push_back(make_ir_var_decl(pattern.loc, nested_name, value_type, std::move(value), false));
                    lower_struct_match_pattern_bindings_from_local(pattern, nested_name, value_type, statements, mutable_binding);
                }
                return;
            case PatternKind::Binding:
                lower_binding_pattern_value(pattern, value_type, std::move(value), mutable_binding, statements);
                return;
            case PatternKind::Alias: {
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                if (is_borrow_type(value_type)) {
                    fail(pattern.loc, "borrow pattern bindings are not supported yet; pass ref values directly to calls");
                }
                if (is_owner_type(value_type)) {
                    fail(pattern.loc, "owning pattern bindings are planned after ownership through aggregates is implemented");
                }
                declare_local(pattern.loc, pattern.alias_name, value_type, mutable_binding);
                statements.push_back(make_ir_var_decl(pattern.loc, pattern.alias_name, value_type, std::move(value), mutable_binding));
                lower_product_match_pattern_bindings_from_local(
                    *pattern.alias_pattern,
                    pattern.alias_name,
                    value_type,
                    statements,
                    mutable_binding
                );
                return;
            }
            case PatternKind::Tuple: {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(pattern.loc, nested_name, value_type, false);
                statements.push_back(make_ir_var_decl(pattern.loc, nested_name, value_type, std::move(value), false));
                lower_tuple_match_pattern_bindings_from_local(pattern, nested_name, value_type, statements, mutable_binding);
                return;
            }
            case PatternKind::Array: {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(pattern.loc, nested_name, value_type, false);
                statements.push_back(make_ir_var_decl(pattern.loc, nested_name, value_type, std::move(value), false));
                if (is_runtime_sequence_pattern_subject(value_type)) {
                    lower_runtime_sequence_match_pattern_bindings_from_local(
                        pattern,
                        nested_name,
                        value_type,
                        statements,
                        mutable_binding
                    );
                    return;
                }
                lower_tuple_match_pattern_bindings_from_local(pattern, nested_name, value_type, statements, mutable_binding);
                return;
            }
            case PatternKind::Struct: {
                std::string nested_name = make_hidden_local("$pattern");
                declare_local(pattern.loc, nested_name, value_type, false);
                statements.push_back(make_ir_var_decl(pattern.loc, nested_name, value_type, std::move(value), false));
                lower_struct_match_pattern_bindings_from_local(pattern, nested_name, value_type, statements, mutable_binding);
                return;
            }
            case PatternKind::Or:
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                return;
        }
        fail(pattern.loc, "unsupported tuple match binding pattern");
    }

    void lower_product_match_pattern_bindings_from_local(const Pattern& pattern,
                                                         const std::string& source_name,
                                                         const IrType& source_type,
                                                         std::vector<IrStmtPtr>& statements,
                                                         bool mutable_binding = false) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_product_match_pattern_bindings_from_local(
                effective_pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        if (is_runtime_sequence_pattern_subject(source_type)) {
            lower_runtime_sequence_match_pattern_bindings_from_local(
                pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        if (source_type.primitive == IrPrimitiveKind::Tuple) {
            lower_tuple_match_pattern_bindings_from_local(pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        if (source_type.primitive == IrPrimitiveKind::Array) {
            lower_tuple_match_pattern_bindings_from_local(pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        if (source_type.primitive == IrPrimitiveKind::Struct) {
            lower_struct_match_pattern_bindings_from_local(pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
            case PatternKind::EnumCase:
            case PatternKind::Struct:
            case PatternKind::Tuple:
            case PatternKind::Array:
                return;
            case PatternKind::Binding:
                lower_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                return;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                declare_local(pattern.loc, pattern.alias_name, source_type, mutable_binding);
                statements.push_back(make_ir_var_decl(
                    pattern.loc,
                    pattern.alias_name,
                    source_type,
                    make_local_lvalue_expr(pattern.loc, source_name, source_type),
                    mutable_binding
                ));
                lower_product_match_pattern_bindings_from_local(
                    *pattern.alias_pattern,
                    source_name,
                    source_type,
                    statements,
                    mutable_binding
                );
                return;
            case PatternKind::Or:
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                return;
        }
    }

    void lower_positional_product_match_pattern_bindings_from_local(const Pattern& pattern,
                                                                    const std::string& source_name,
                                                                    const IrType& source_type,
                                                                    std::vector<IrStmtPtr>& statements,
                                                                    bool mutable_binding = false) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_positional_product_match_pattern_bindings_from_local(
                effective_pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        const std::vector<IrType>& fields = aggregate_field_types(source_type);
        require_tuple_pattern_arity(pattern, source_type, fields);

        for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
            const Pattern& item = pattern.elements[i];
            if (item.kind == PatternKind::Wildcard) continue;
            std::size_t field_index = tuple_pattern_field_index(pattern, fields.size(), i);
            lower_tuple_match_value_bindings(
                item,
                fields[field_index],
                make_tuple_index_expr(item.loc, source_name, source_type, field_index),
                statements,
                mutable_binding
            );
        }
    }

    void lower_tuple_match_pattern_bindings_from_local(const Pattern& pattern,
                                                       const std::string& source_name,
                                                       const IrType& source_type,
                                                       std::vector<IrStmtPtr>& statements,
                                                       bool mutable_binding = false) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_tuple_match_pattern_bindings_from_local(
                effective_pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        if (is_runtime_sequence_pattern_subject(source_type)) {
            lower_runtime_sequence_match_pattern_bindings_from_local(
                pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
                return;
            case PatternKind::Binding:
                lower_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                return;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                declare_local(pattern.loc, pattern.alias_name, source_type, mutable_binding);
                statements.push_back(make_ir_var_decl(
                    pattern.loc,
                    pattern.alias_name,
                    source_type,
                    make_local_lvalue_expr(pattern.loc, source_name, source_type),
                    mutable_binding
                ));
                lower_product_match_pattern_bindings_from_local(
                    *pattern.alias_pattern,
                    source_name,
                    source_type,
                    statements,
                    mutable_binding
                );
                return;
            case PatternKind::Or:
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                return;
            case PatternKind::EnumCase:
            case PatternKind::Struct:
                if (source_type.primitive == IrPrimitiveKind::Struct) {
                    lower_struct_match_pattern_bindings_from_local(pattern, source_name, source_type, statements, mutable_binding);
                    return;
                }
                return;
            case PatternKind::Tuple:
            case PatternKind::Array:
                break;
        }

        bool array_pattern = pattern.kind == PatternKind::Array;
        IrPrimitiveKind expected_primitive = array_pattern ? IrPrimitiveKind::Array : IrPrimitiveKind::Tuple;
        const char* pattern_name = array_pattern ? "array" : "tuple";
        if (source_type.primitive != expected_primitive) {
            fail(pattern.loc, std::string(pattern_name) + " match pattern requires a " +
                              pattern_name + " value, got " + type_name(source_type));
        }
        lower_positional_product_match_pattern_bindings_from_local(pattern, source_name, source_type, statements, mutable_binding);
    }

    void lower_tuple_struct_match_pattern_bindings_from_local(const Pattern& pattern,
                                                              const std::string& source_name,
                                                              const IrType& source_type,
                                                              std::vector<IrStmtPtr>& statements,
                                                              bool mutable_binding = false) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_tuple_struct_match_pattern_bindings_from_local(
                effective_pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        const StructInfo& info = require_struct_match_pattern_type(pattern.loc, pattern.case_name, source_type);
        if (!info.tuple_struct) {
            fail(pattern.loc, "tuple-struct pattern requires a tuple struct");
        }
        if (!pattern.has_payload_pattern || !pattern.payload_pattern) {
            fail(pattern.loc, "tuple-struct pattern '" + pattern.case_name + "' requires positional fields");
        }
        const std::vector<IrType>& fields = aggregate_field_types(source_type);
        const Pattern& payload = *pattern.payload_pattern;
        if (payload.kind == PatternKind::Tuple) {
            lower_positional_product_match_pattern_bindings_from_local(payload, source_name, source_type, statements, mutable_binding);
            return;
        }
        if (fields.size() != 1) {
            fail(payload.loc,
                 "tuple-struct pattern for '" + info.name + "' has 1 field but value has " +
                     std::to_string(fields.size()));
        }
        lower_tuple_match_value_bindings(
            payload,
            fields[0],
            make_tuple_index_expr(payload.loc, source_name, source_type, 0),
            statements,
            mutable_binding
        );
    }

    void lower_struct_match_pattern_bindings_from_local(const Pattern& pattern,
                                                        const std::string& source_name,
                                                        const IrType& source_type,
                                                        std::vector<IrStmtPtr>& statements,
                                                        bool mutable_binding = false) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_struct_match_pattern_bindings_from_local(
                effective_pattern, source_name, source_type, statements, mutable_binding);
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::IntegerLiteral:
            case PatternKind::BoolLiteral:
            case PatternKind::Range:
                return;
            case PatternKind::Binding:
                lower_binding_pattern_from_local(pattern, source_name, source_type, mutable_binding, statements);
                return;
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased pattern");
                declare_local(pattern.loc, pattern.alias_name, source_type, mutable_binding);
                statements.push_back(make_ir_var_decl(
                    pattern.loc,
                    pattern.alias_name,
                    source_type,
                    make_local_lvalue_expr(pattern.loc, source_name, source_type),
                    mutable_binding
                ));
                lower_struct_match_pattern_bindings_from_local(
                    *pattern.alias_pattern,
                    source_name,
                    source_type,
                    statements,
                    mutable_binding
                );
                return;
            case PatternKind::Or:
                if (pattern_has_binding(pattern)) {
                    fail(pattern.loc, "or-pattern bindings are planned but are not supported yet");
                }
                return;
            case PatternKind::EnumCase:
                lower_tuple_struct_match_pattern_bindings_from_local(pattern, source_name, source_type, statements, mutable_binding);
                return;
            case PatternKind::Tuple:
                fail(pattern.loc, "tuple patterns cannot match named struct values; use the tuple-struct name");
            case PatternKind::Array:
                fail(pattern.loc, "array patterns cannot match named struct values");
            case PatternKind::Struct:
                break;
        }

        require_struct_match_pattern_type(pattern.loc, pattern.case_name, source_type);
        if (pattern.field_names.size() != pattern.elements.size()) {
            throw CompileError("internal error: struct match pattern field/value arity mismatch");
        }
        if (!pattern.has_rest && pattern.field_names.size() != source_type.field_names.size()) {
            fail(pattern.loc, "struct match pattern must mention all fields or use '..'");
        }

        std::set<std::string> seen_fields;
        for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
            const std::string& field_name = pattern.field_names[i];
            if (!seen_fields.insert(field_name).second) {
                fail(pattern.elements[i].loc, "duplicate field '" + field_name + "' in struct match pattern");
            }
            std::size_t field_index = struct_field_index(pattern.elements[i].loc, source_type, field_name);
            lower_tuple_match_value_bindings(
                pattern.elements[i],
                source_type.field_types[field_index],
                make_tuple_index_expr(pattern.elements[i].loc, source_name, source_type, field_index),
                statements,
                mutable_binding
            );
        }
    }

    void require_tuple_match_exhaustive(SourceLocation loc,
                                        const IrType& match_type,
                                        const ProductMatchCoverage& coverage) const {
        if (product_match_coverage_is_exhaustive(coverage)) return;
        std::string kind = match_type.primitive == IrPrimitiveKind::Struct
            ? "struct"
            : (match_type.primitive == IrPrimitiveKind::Array ? "array" : "tuple");
        std::string message = kind + " match must cover every supported product case or include an irrefutable arm such as _ or a binding-only pattern";
        std::set<std::string> tuple_struct_names;
        for (const auto& [name, info] : structs_) {
            if (info.tuple_struct) tuple_struct_names.insert(name);
        }
        std::string missing = product_missing_case_hint(match_type, coverage, tuple_struct_names);
        if (!missing.empty()) {
            message += "; missing case such as " + missing;
        }
        fail(loc, message);
    }

    IrExprPtr make_default_value_expr(SourceLocation loc, const IrType& type) const {
        if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
            return make_bool_literal_expr(loc, false);
        }
        if (has_aggregate_enum_layout(type)) {
            auto value = std::make_unique<IrExpr>();
            value->kind = IrExprKind::EnumConstruct;
            value->loc = loc;
            value->type = type;
            set_ir_expr_enum_name(*value, type.name);
            set_ir_expr_enum_result_payload(*value, 0, false);
            return value;
        }
        if (is_value_integer_type(type) ||
            type.primitive == IrPrimitiveKind::Enum) {
            return make_integer_zero(loc, type);
        }
        if (is_raw_pointer_type(type)) {
            return make_null_literal_expr(loc, type);
        }
        if (type.primitive == IrPrimitiveKind::F32 ||
            type.primitive == IrPrimitiveKind::F64 ||
            type.primitive == IrPrimitiveKind::F128) {
            return make_float_literal_expr(loc, type, 0.0);
        }
        if (type.primitive == IrPrimitiveKind::String) {
            return make_string_literal_expr(loc, type);
        }
        if (type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            type.primitive == IrPrimitiveKind::Struct) {
            std::vector<IrExprPtr> elements;
            for (const auto& field_type : aggregate_field_types(type)) {
                elements.push_back(make_default_value_expr(loc, field_type));
            }
            return make_ir_tuple_expr(loc, type, std::move(elements));
        }
        fail(loc, "internal error: cannot build unreachable match fallback for " + type_name(type));
    }

    IrExprPtr make_unreachable_match_fallback_expr(SourceLocation loc, const IrType& result_type) {
        std::vector<IrStmtPtr> body;
        body.push_back(make_panic_stmt(loc));
        return make_tuple_match_block_value(
            loc,
            result_type,
            std::move(body),
            make_default_value_expr(loc, result_type)
        );
    }

    IrStmtPtr make_bool_assignment_stmt(SourceLocation loc, const std::string& name, bool value) const {
        auto stmt = std::make_unique<IrStmt>();
        stmt->kind = IrStmtKind::Assign;
        stmt->loc = loc;
        set_ir_stmt_assign_name(*stmt, name);
        set_ir_stmt_assign_rhs(*stmt, make_bool_literal_expr(loc, value));
        return stmt;
    }

    std::vector<IrMatchArm> lower_if_let_enum_pattern_arms(
        const Pattern& pattern,
        const EnumInfo& enum_info,
        const IrType& enum_value_type
    ) {
        EnumMatchCoverage coverage;
        std::vector<IrMatchArm> arms = lower_match_arm_patterns(pattern, enum_info, enum_value_type, coverage);
        if (arms.empty()) {
            fail(pattern.loc, "if-let pattern did not lower to a match arm");
        }
        for (const auto& arm : arms) {
            if (arm.wildcard) {
                fail(pattern.loc, "if-let requires a refutable enum-case pattern");
            }
        }
        return arms;
    }

    Flow check_if_let(const Stmt& stmt, IrStmt& lowered) {
        IrExprPtr match_value = check_expr(*stmt.condition);
        if (is_aggregate_type(match_value->type) ||
            is_runtime_sequence_pattern_subject(match_value->type)) {
            return check_aggregate_if_let(stmt, lowered, std::move(match_value));
        }

        lowered.kind = IrStmtKind::Block;
        const Pattern& condition_pattern = expanded_pattern(*stmt.condition_pattern);
        IrType enum_value_type = match_value->type;
        const EnumInfo& enum_info = require_enum_match_value(stmt.loc, *match_value);
        std::vector<IrMatchArm> then_arms = lower_if_let_enum_pattern_arms(
            condition_pattern,
            enum_info,
            enum_value_type
        );

        std::string subject_name = make_hidden_local("$iflet_enum");
        declare_local(stmt.loc, subject_name, enum_value_type, false);
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(
            stmt.loc,
            subject_name,
            enum_value_type,
            std::move(match_value),
            false
        ));

        IrType matched_type = bool_type(stmt.loc);
        std::string matched_name = make_hidden_local("$iflet_matched");
        declare_local(stmt.loc, matched_name, matched_type, true);
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(
            stmt.loc,
            matched_name,
            matched_type,
            make_bool_literal_expr(stmt.loc, false),
            true
        ));

        auto pattern_match = std::make_unique<IrStmt>();
        pattern_match->kind = IrStmtKind::Match;
        pattern_match->loc = stmt.loc;
        pattern_match->match_value = make_local_lvalue_expr(stmt.loc, subject_name, enum_value_type);
        IrStmtMatchArms& match_arms = ensure_ir_stmt_match_arms(*pattern_match);
        for (auto& arm : then_arms) {
            arm.body.push_back(make_bool_assignment_stmt(arm.loc, matched_name, true));
            match_arms.push_back(std::move(arm));
        }
        IrMatchArm no_match;
        no_match.loc = stmt.loc;
        no_match.wildcard = true;
        match_arms.push_back(std::move(no_match));
        ir_stmt_statements(lowered).push_back(std::move(pattern_match));

        StateSnapshot branch_input = snapshot_states();
        push_scope();
        declare_match_arm_bindings(ir_stmt_match_arms(*ir_stmt_statements(lowered).back()).front());
        CheckedStatements then_checked = check_statements(stmt_then_body(stmt), false);
        std::vector<IrStmtPtr> then_body = std::move(then_checked.statements);
        if (then_checked.flow == Flow::Returns) discard_scope();
        else if (then_checked.flow == Flow::Stops) discard_scope();
        else {
            append_current_scope_auto_destroy_cleanup(stmt.loc, then_body);
            pop_scope();
        }
        StateSnapshot then_state = snapshot_states();

        restore_states(branch_input);

        auto if_stmt = std::make_unique<IrStmt>();
        if_stmt->kind = IrStmtKind::If;
        if_stmt->loc = stmt.loc;
        if_stmt->condition = make_local_lvalue_expr(stmt.loc, matched_name, matched_type);
        set_ir_stmt_then_body(*if_stmt, std::move(then_body));

        if (stmt_else_body(stmt).empty()) {
            if (then_checked.flow == Flow::Continues) {
                require_same_states(stmt.loc, branch_input, then_state, "changes ownership state in if-let without else");
                restore_merged_states(branch_input, then_state);
            } else {
                restore_states(branch_input);
            }
            ir_stmt_statements(lowered).push_back(std::move(if_stmt));
            return Flow::Continues;
        }

        CheckedStatements else_checked = check_statements(stmt_else_body(stmt), true);
        set_ir_stmt_else_body(*if_stmt, std::move(else_checked.statements));
        StateSnapshot else_state = snapshot_states();

        ir_stmt_statements(lowered).push_back(std::move(if_stmt));

        if (then_checked.flow != Flow::Continues && else_checked.flow != Flow::Continues) {
            restore_states(branch_input);
            if (then_checked.flow == Flow::Returns && else_checked.flow == Flow::Returns) return Flow::Returns;
            return Flow::Stops;
        }
        if (then_checked.flow != Flow::Continues) {
            restore_states(else_state);
            return Flow::Continues;
        }
        if (else_checked.flow != Flow::Continues) {
            restore_states(then_state);
            return Flow::Continues;
        }

        require_same_states(stmt.loc, then_state, else_state, "has incompatible ownership states after if-let branches");
        restore_merged_states(then_state, else_state);
        return Flow::Continues;
    }

    Flow check_aggregate_if_let(const Stmt& stmt, IrStmt& lowered, IrExprPtr subject) {
        IrType subject_type = subject->type;
        const Pattern& condition_pattern = expanded_pattern(*stmt.condition_pattern);
        lowered.kind = IrStmtKind::Block;

        std::string subject_name = make_hidden_local(
            subject_type.primitive == IrPrimitiveKind::Struct ? "$iflet_struct" : "$iflet_tuple");
        declare_local(stmt.loc, subject_name, subject_type, false);
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(stmt.loc, subject_name, subject_type, std::move(subject), false));

        std::vector<Pattern> alternatives = expand_or_pattern_alternatives(condition_pattern);
        std::set<std::string> reusable_names;
        if (pattern_contains_or(condition_pattern)) {
            reusable_names = require_same_or_pattern_bindings(condition_pattern.loc, alternatives, subject_type);
        }

        bool has_irrefutable_alternative = false;
        StateSnapshot branch_input = snapshot_states();
        StateSnapshot continuing_state;
        bool has_continuing_state = false;
        bool all_return = true;
        bool all_non_continuing = true;
        std::vector<TupleCheckedStmtArm> checked_arms;
        checked_arms.reserve(alternatives.size() + 1);

        for (std::size_t i = 0; i < alternatives.size(); ++i) {
            const Pattern& pattern = alternatives[i];
            TupleCheckedStmtArm lowered_arm;
            lowered_arm.loc = pattern.loc;

            std::vector<IrStmtPtr> condition_prelude;
            lowered_arm.condition = lower_product_match_pattern_condition(
                pattern,
                subject_name,
                subject_type,
                condition_prelude
            );
            if (!lowered_arm.condition) {
                has_irrefutable_alternative = true;
                if (!stmt_else_body(stmt).empty()) {
                    fail(condition_pattern.loc, "irrefutable if-let aggregate pattern cannot have else");
                }
            }
            for (auto& statement : condition_prelude) {
                ir_stmt_statements(lowered).push_back(std::move(statement));
            }

            restore_states(branch_input);
            push_scope();
            local_scopes_.set_reusable_pattern_bindings(i == 0 ? std::set<std::string>{} : reusable_names);
            lower_product_match_pattern_bindings_from_local(pattern, subject_name, subject_type, lowered_arm.body);
            local_scopes_.clear_reusable_pattern_bindings();
            CheckedStatements then_checked = check_statements(stmt_then_body(stmt), false);
            for (auto& statement : then_checked.statements) {
                lowered_arm.body.push_back(std::move(statement));
            }
            if (then_checked.flow == Flow::Returns) discard_scope();
            else if (then_checked.flow == Flow::Stops) discard_scope();
            else {
                append_current_scope_auto_destroy_cleanup(stmt.loc, lowered_arm.body);
                pop_scope();
            }
            StateSnapshot then_state = snapshot_states();

            if (then_checked.flow == Flow::Continues) {
                all_return = false;
                all_non_continuing = false;
                if (!has_continuing_state) {
                    continuing_state = then_state;
                    has_continuing_state = true;
                } else {
                    require_same_states(
                        stmt.loc,
                        continuing_state,
                        then_state,
                        "has incompatible ownership states after if-let pattern alternatives"
                    );
                    merge_zone_generations_into(continuing_state, then_state);
                }
            } else if (then_checked.flow == Flow::Stops) {
                all_return = false;
            }

            checked_arms.push_back(std::move(lowered_arm));
        }

        if (has_irrefutable_alternative && !stmt_else_body(stmt).empty()) {
            fail(condition_pattern.loc, "irrefutable if-let aggregate pattern cannot have else");
        }

        if (!stmt_else_body(stmt).empty()) {
            restore_states(branch_input);
            CheckedStatements else_checked = check_statements(stmt_else_body(stmt), true);
            TupleCheckedStmtArm fallback;
            fallback.loc = stmt.loc;
            fallback.body = std::move(else_checked.statements);
            checked_arms.push_back(std::move(fallback));
            StateSnapshot else_state = snapshot_states();

            if (else_checked.flow == Flow::Continues) {
                all_return = false;
                all_non_continuing = false;
                if (!has_continuing_state) {
                    continuing_state = else_state;
                    has_continuing_state = true;
                } else {
                    require_same_states(
                        stmt.loc,
                        continuing_state,
                        else_state,
                        "has incompatible ownership states after if-let branches"
                    );
                    merge_zone_generations_into(continuing_state, else_state);
                }
            } else if (else_checked.flow == Flow::Stops) {
                all_return = false;
            }

            std::vector<IrStmtPtr> chain = build_tuple_match_if_chain(checked_arms);
            for (auto& statement : chain) {
                ir_stmt_statements(lowered).push_back(std::move(statement));
            }

            if (all_non_continuing) {
                restore_states(branch_input);
                return all_return ? Flow::Returns : Flow::Stops;
            }
            restore_states(continuing_state);
            return Flow::Continues;
        }

        std::vector<IrStmtPtr> chain = build_tuple_match_if_chain(checked_arms);
        for (auto& statement : chain) {
            ir_stmt_statements(lowered).push_back(std::move(statement));
        }

        if (has_irrefutable_alternative) {
            if (all_non_continuing) {
                restore_states(branch_input);
                return all_return ? Flow::Returns : Flow::Stops;
            }
            restore_states(continuing_state);
            return Flow::Continues;
        }

        if (has_continuing_state) {
            require_same_states(stmt.loc, branch_input, continuing_state, "changes ownership state in if-let without else");
            restore_merged_states(branch_input, continuing_state);
        } else {
            restore_states(branch_input);
        }
        return Flow::Continues;
    }

    Flow check_tuple_match(const Stmt& stmt, IrStmt& lowered) {
        IrExprPtr subject = std::move(lowered.match_value);
        IrType subject_type = subject->type;
        const bool runtime_sequence_subject = is_runtime_sequence_pattern_subject(subject_type);
        if (!is_aggregate_type(subject_type) && !runtime_sequence_subject) {
            fail(stmt.loc, "aggregate match requires a tuple, array, or struct value, got " + type_name(subject_type));
        }

        lowered.kind = IrStmtKind::Block;
        std::string subject_name = make_hidden_local(
            subject_type.primitive == IrPrimitiveKind::Struct ? "$match_struct" : "$match_tuple");
        declare_local(stmt.loc, subject_name, subject_type, false);
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(stmt.loc, subject_name, subject_type, std::move(subject), false));

        StateSnapshot branch_input = snapshot_states();
        StateSnapshot continuing_state;
        bool has_continuing_state = false;
        bool all_return = true;
        bool all_non_continuing = true;
        ProductMatchCoverage coverage;
        std::vector<TupleCheckedStmtArm> checked_arms;

        const StmtMatchArms& source_arms = stmt_match_arms(stmt);
        for (const auto& arm : source_arms) {
            if (coverage.has_irrefutable_arm) {
                fail(arm.pattern.loc, "unreachable match arm after irrefutable pattern");
            }

            std::vector<Pattern> alternatives = expand_or_pattern_alternatives(arm.pattern);
            std::set<std::string> reusable_names;
            if (pattern_contains_or(arm.pattern)) {
                reusable_names = require_same_or_pattern_bindings(arm.pattern.loc, alternatives, subject_type);
            }

            std::size_t alternative_index = 0;
            for (const auto& pattern : alternatives) {
                TupleCheckedStmtArm lowered_arm;
                lowered_arm.loc = arm.loc;
                std::vector<IrStmtPtr> condition_prelude;
                lowered_arm.condition = lower_product_match_pattern_condition(
                    pattern,
                    subject_name,
                    subject_type,
                    condition_prelude
                );
                for (auto& statement : condition_prelude) {
                    ir_stmt_statements(lowered).push_back(std::move(statement));
                }
                if (runtime_sequence_subject) {
                    if (!lowered_arm.condition) coverage.has_irrefutable_arm = true;
                } else {
                    note_product_match_coverage(pattern, subject_type, coverage);
                    if (!lowered_arm.condition) coverage.has_irrefutable_arm = true;
                }

                restore_states(branch_input);
                push_scope();
                local_scopes_.set_reusable_pattern_bindings(alternative_index == 0 ? std::set<std::string>{} : reusable_names);
                lower_product_match_pattern_bindings_from_local(pattern, subject_name, subject_type, lowered_arm.body);
                local_scopes_.clear_reusable_pattern_bindings();
                CheckedStatements checked = check_statements(arm.body, false);
                for (auto& statement : checked.statements) {
                    lowered_arm.body.push_back(std::move(statement));
                }
                if (checked.flow == Flow::Returns) discard_scope();
                else if (checked.flow == Flow::Stops) discard_scope();
                else {
                    append_current_scope_auto_destroy_cleanup(arm.loc, lowered_arm.body);
                    pop_scope();
                }

                StateSnapshot branch_state = snapshot_states();
                if (checked.flow == Flow::Continues) {
                    all_return = false;
                    all_non_continuing = false;
                    if (!has_continuing_state) {
                        continuing_state = branch_state;
                        has_continuing_state = true;
                    } else {
                        require_same_states(arm.loc, continuing_state, branch_state,
                                            "has incompatible ownership states after match arms");
                        merge_zone_generations_into(continuing_state, branch_state);
                    }
                } else if (checked.flow == Flow::Stops) {
                    all_return = false;
                }

                checked_arms.push_back(std::move(lowered_arm));
                ++alternative_index;
            }
        }

        if (runtime_sequence_subject) {
            if (!coverage.has_irrefutable_arm) {
                fail(stmt.loc,
                     runtime_sequence_pattern_subject_name(subject_type) +
                         " match must include _ or [..] fallback");
            }
        } else {
            require_tuple_match_exhaustive(stmt.loc, subject_type, coverage);
        }
        std::vector<IrStmtPtr> chain = build_tuple_match_if_chain(checked_arms);
        for (auto& statement : chain) {
            ir_stmt_statements(lowered).push_back(std::move(statement));
        }

        if (all_non_continuing) {
            restore_states(branch_input);
            return all_return ? Flow::Returns : Flow::Stops;
        }
        restore_states(continuing_state);
        return Flow::Continues;
    }

    Flow check_match(const Stmt& stmt, IrStmt& lowered) {
        const StmtMatchArms& source_arms = stmt_match_arms(stmt);
        if (source_arms.empty()) fail(stmt.loc, "match must have at least one arm");

        lowered.match_value = check_expr(*stmt.match_value);
        if (is_borrow_type(lowered.match_value->type)) {
            fail(stmt.loc, "borrow expression result must be passed directly to a call");
        }
        if (!is_value_enum_type(lowered.match_value->type)) {
            if (is_value_integer_type(lowered.match_value->type) ||
                (lowered.match_value->type.qualifier == TypeQualifier::Value &&
                 lowered.match_value->type.primitive == IrPrimitiveKind::Bool)) {
                return check_scalar_match(stmt, lowered);
            }
            if (is_aggregate_type(lowered.match_value->type) ||
                is_runtime_sequence_pattern_subject(lowered.match_value->type)) {
                return check_tuple_match(stmt, lowered);
            }
            fail(stmt.loc, "match value must be an enum, integer, bool, tuple, array, or struct, got " + type_name(lowered.match_value->type));
        }

        auto enum_found = enums_.find(lowered.match_value->type.name);
        if (enum_found == enums_.end()) {
            fail(stmt.loc, "unknown enum type '" + lowered.match_value->type.name + "'");
        }
        const EnumInfo& enum_info = enum_found->second;
        IrType enum_value_type = lowered.match_value->type;

        StateSnapshot branch_input = snapshot_states();
        StateSnapshot continuing_state;
        bool has_continuing_state = false;
        bool all_return = true;
        bool all_non_continuing = true;
        EnumMatchCoverage coverage;

        IrStmtMatchArms& lowered_arms = ensure_ir_stmt_match_arms(lowered);
        for (const auto& arm : source_arms) {
            std::set<std::string> reusable_names;
            if (pattern_contains_or(arm.pattern)) {
                std::vector<Pattern> alternatives = expand_or_pattern_alternatives(arm.pattern);
                reusable_names = require_same_or_pattern_bindings(
                    arm.pattern.loc,
                    alternatives,
                    enum_match_value_type(arm.pattern.loc, enum_value_type)
                );
            }
            std::vector<IrMatchArm> lowered_patterns = lower_match_arm_patterns(
                arm.pattern,
                enum_info,
                enum_value_type,
                coverage);
            std::size_t alternative_index = 0;
            for (auto& lowered_arm : lowered_patterns) {
                restore_states(branch_input);
                push_scope();
                local_scopes_.set_reusable_pattern_bindings(alternative_index == 0 ? std::set<std::string>{} : reusable_names);
                declare_match_arm_bindings(lowered_arm);
                local_scopes_.clear_reusable_pattern_bindings();

                CheckedStatements checked = check_statements(arm.body, false);
                lowered_arm.body = std::move(checked.statements);
                if (checked.flow == Flow::Returns) discard_scope();
                else if (checked.flow == Flow::Stops) discard_scope();
                else {
                    append_current_scope_auto_destroy_cleanup(arm.loc, lowered_arm.body);
                    pop_scope();
                }

                StateSnapshot branch_state = snapshot_states();
                if (checked.flow == Flow::Continues) {
                    all_return = false;
                    all_non_continuing = false;
                    if (!has_continuing_state) {
                        continuing_state = branch_state;
                        has_continuing_state = true;
                    } else {
                        require_same_states(arm.loc, continuing_state, branch_state,
                                            "has incompatible ownership states after match arms");
                        merge_zone_generations_into(continuing_state, branch_state);
                    }
                } else if (checked.flow == Flow::Stops) {
                    all_return = false;
                }

                lowered_arms.push_back(std::move(lowered_arm));
                ++alternative_index;
            }
        }

        require_match_exhaustive(stmt.loc, enum_info, coverage);

        if (all_non_continuing) {
            restore_states(branch_input);
            return all_return ? Flow::Returns : Flow::Stops;
        }
        restore_states(continuing_state);
        return Flow::Continues;
    }

    Flow check_scalar_match(const Stmt& stmt, IrStmt& lowered) {
        const StmtMatchArms& source_arms = stmt_match_arms(stmt);
        StateSnapshot branch_input = snapshot_states();
        StateSnapshot continuing_state;
        bool has_continuing_state = false;
        bool all_return = true;
        bool all_non_continuing = true;
        ScalarMatchCoverage coverage;

        IrStmtMatchArms& lowered_arms = ensure_ir_stmt_match_arms(lowered);
        for (const auto& arm : source_arms) {
            std::set<std::string> reusable_names;
            if (pattern_contains_or(arm.pattern)) {
                std::vector<Pattern> alternatives = expand_or_pattern_alternatives(arm.pattern);
                reusable_names = require_same_or_pattern_bindings(
                    arm.pattern.loc,
                    alternatives,
                    lowered.match_value->type
                );
            }
            std::vector<IrMatchArm> lowered_patterns = lower_scalar_match_arm_patterns(
                arm.pattern, lowered.match_value->type, coverage);
            std::size_t alternative_index = 0;
            for (auto& lowered_arm : lowered_patterns) {
                restore_states(branch_input);
                push_scope();
                local_scopes_.set_reusable_pattern_bindings(alternative_index == 0 ? std::set<std::string>{} : reusable_names);
                declare_match_arm_bindings(lowered_arm);
                local_scopes_.clear_reusable_pattern_bindings();
                CheckedStatements checked = check_statements(arm.body, false);
                lowered_arm.body = std::move(checked.statements);
                if (checked.flow == Flow::Returns) discard_scope();
                else if (checked.flow == Flow::Stops) discard_scope();
                else {
                    append_current_scope_auto_destroy_cleanup(arm.loc, lowered_arm.body);
                    pop_scope();
                }

                StateSnapshot branch_state = snapshot_states();
                if (checked.flow == Flow::Continues) {
                    all_return = false;
                    all_non_continuing = false;
                    if (!has_continuing_state) {
                        continuing_state = branch_state;
                        has_continuing_state = true;
                    } else {
                        require_same_states(arm.loc, continuing_state, branch_state,
                                            "has incompatible ownership states after match arms");
                        merge_zone_generations_into(continuing_state, branch_state);
                    }
                } else if (checked.flow == Flow::Stops) {
                    all_return = false;
                }

                lowered_arms.push_back(std::move(lowered_arm));
                ++alternative_index;
            }
        }

        require_scalar_match_exhaustive(stmt.loc, lowered.match_value->type, coverage);
        if (all_non_continuing) {
            restore_states(branch_input);
            return all_return ? Flow::Returns : Flow::Stops;
        }
        restore_states(continuing_state);
        return Flow::Continues;
    }

    Flow check_while(const Stmt& stmt, IrStmt& lowered) {
        lowered.condition = check_expr(*stmt.condition);
        coerce_condition_to_bool(stmt.loc, lowered.condition);
        const std::optional<bool> known_condition = known_bool_condition_value(*lowered.condition);
        const bool literal_true_condition = known_condition.value_or(false);
        StateSnapshot loop_input = snapshot_states();
        LocalScopeStack::NameState loop_name_state = local_scopes_.snapshot_name_state();
        const std::string& label = stmt_label(stmt);
        const std::vector<StmtPtr>& body_source = stmt_loop_body(stmt);

        LoopInfo loop;
        loop.label = label;
        push_loop(stmt.loc, loop);
        CheckedStatements body = check_statements(body_source, true);
        set_ir_stmt_loop_body(lowered, std::move(body.statements));
        StateSnapshot loop_body_state = snapshot_states();
        LoopInfo loop_state = loops_.back();
        loops_.pop_back();

        if (known_condition && !*known_condition) {
            restore_states(loop_input);
            set_ir_stmt_label(lowered, label);
            return Flow::Continues;
        }

        LoopBodyRecheck recheck{&body_source, true, std::move(loop_name_state)};
        restore_states(checked_loop_exit_state(
            stmt.loc,
            loop_input,
            loop_body_state,
            loop_state,
            body.flow,
            !literal_true_condition,
            &recheck
        ));
        set_ir_stmt_label(lowered, label);
        if (literal_true_condition && body.flow == Flow::Returns) return Flow::Returns;
        if (literal_true_condition && literal_true_loop_never_falls_through(loop_state, body.flow)) {
            return Flow::Stops;
        }
        return Flow::Continues;
    }

    void check_while_let(const Stmt& stmt, IrStmt& lowered) {
        IrExprPtr match_value = check_expr(*stmt.condition);
        if (is_aggregate_type(match_value->type) ||
            is_runtime_sequence_pattern_subject(match_value->type)) {
            check_aggregate_while_let(stmt, lowered, std::move(match_value));
            return;
        }

        lowered.kind = IrStmtKind::WhileLet;
        const std::string& label = stmt_label(stmt);
        set_ir_stmt_label(lowered, label);
        lowered.match_value = std::move(match_value);
        const Pattern& condition_pattern = expanded_pattern(*stmt.condition_pattern);
        const EnumInfo& enum_info = require_enum_match_value(stmt.loc, *lowered.match_value);
        IrType enum_value_type = lowered.match_value->type;
        EnumMatchCoverage coverage;
        std::vector<IrMatchArm> pattern_arms = lower_match_arm_patterns(
            condition_pattern,
            enum_info,
            enum_value_type,
            coverage
        );
        if (pattern_arms.empty()) {
            fail(condition_pattern.loc, "while-let pattern did not lower to a match arm");
        }
        for (const auto& arm : pattern_arms) {
            if (arm.wildcard) {
                fail(condition_pattern.loc, "while-let requires a refutable enum-case pattern");
            }
        }
        StateSnapshot loop_input = snapshot_states();

        LoopInfo loop;
        loop.label = label;
        push_loop(stmt.loc, loop);
        push_scope();
        declare_match_arm_bindings(pattern_arms.front());
        CheckedStatements body = check_statements(stmt_loop_body(stmt), false);
        set_ir_stmt_loop_body(lowered, std::move(body.statements));
        if (body.flow == Flow::Returns) discard_scope();
        else if (body.flow == Flow::Stops) discard_scope();
        else {
            append_current_scope_auto_destroy_cleanup(stmt.loc, ir_stmt_loop_body(lowered));
            pop_scope();
        }
        StateSnapshot loop_body_state = snapshot_states();
        LoopInfo loop_state = loops_.back();
        loops_.pop_back();

        restore_states(checked_loop_exit_state(stmt.loc, loop_input, loop_body_state, loop_state, body.flow));
        IrStmtMatchArms& lowered_arms = ensure_ir_stmt_match_arms(lowered);
        for (auto& arm : pattern_arms) {
            lowered_arms.push_back(std::move(arm));
        }
    }

    static IrStmtPtr make_unlabeled_break_stmt(SourceLocation loc) {
        auto stmt = std::make_unique<IrStmt>();
        stmt->kind = IrStmtKind::Break;
        stmt->loc = loc;
        return stmt;
    }

    void check_aggregate_while_let(const Stmt& stmt, IrStmt& lowered, IrExprPtr subject) {
        IrType subject_type = subject->type;
        const Pattern& condition_pattern = expanded_pattern(*stmt.condition_pattern);
        lowered.kind = IrStmtKind::While;
        const std::string& label = stmt_label(stmt);
        set_ir_stmt_label(lowered, label);
        lowered.condition = make_bool_literal_expr(stmt.loc, true);

        std::string subject_name = make_hidden_local(
            subject_type.primitive == IrPrimitiveKind::Struct ? "$whilelet_struct" : "$whilelet_tuple");
        declare_local(stmt.loc, subject_name, subject_type, false);
        ir_stmt_loop_body(lowered).push_back(make_ir_var_decl(stmt.loc, subject_name, subject_type, std::move(subject), false));

        std::vector<Pattern> alternatives = expand_or_pattern_alternatives(condition_pattern);
        std::set<std::string> reusable_names;
        if (pattern_contains_or(condition_pattern)) {
            reusable_names = require_same_or_pattern_bindings(condition_pattern.loc, alternatives, subject_type);
        }

        bool has_irrefutable_alternative = false;
        std::vector<TupleCheckedStmtArm> checked_arms;
        checked_arms.reserve(alternatives.size() + 1);

        for (const auto& pattern : alternatives) {
            TupleCheckedStmtArm arm;
            arm.loc = pattern.loc;
            std::vector<IrStmtPtr> condition_prelude;
            arm.condition = lower_product_match_pattern_condition(
                pattern,
                subject_name,
                subject_type,
                condition_prelude
            );
            if (!arm.condition) has_irrefutable_alternative = true;
            for (auto& statement : condition_prelude) {
                ir_stmt_loop_body(lowered).push_back(std::move(statement));
            }
            checked_arms.push_back(std::move(arm));
        }

        StateSnapshot loop_input = snapshot_states();
        LoopInfo loop;
        loop.label = label;
        push_loop(stmt.loc, loop);

        for (std::size_t i = 0; i < alternatives.size(); ++i) {
            restore_states(loop_input);
            push_scope();
            local_scopes_.set_reusable_pattern_bindings(i == 0 ? std::set<std::string>{} : reusable_names);
            lower_product_match_pattern_bindings_from_local(
                alternatives[i],
                subject_name,
                subject_type,
                checked_arms[i].body
            );
            local_scopes_.clear_reusable_pattern_bindings();

            CheckedStatements body = check_statements(stmt_loop_body(stmt), false);
            for (auto& statement : body.statements) {
                checked_arms[i].body.push_back(std::move(statement));
            }
            if (body.flow == Flow::Returns) discard_scope();
            else if (body.flow == Flow::Stops) discard_scope();
            else {
                append_current_scope_auto_destroy_cleanup(stmt.loc, checked_arms[i].body);
                pop_scope();
            }

            StateSnapshot loop_body_state = snapshot_states();
            if (body.flow == Flow::Continues) {
                require_same_states(
                    stmt.loc,
                    loop_input,
                    loop_body_state,
                    "cannot change ownership state inside loop yet"
                );
                restore_merged_states(loop_input, loop_body_state);
            } else {
                restore_states(loop_input);
            }
        }
        LoopInfo loop_state = loops_.back();
        loops_.pop_back();

        if (!has_irrefutable_alternative) {
            TupleCheckedStmtArm fallback;
            fallback.loc = stmt.loc;
            fallback.body.push_back(make_unlabeled_break_stmt(stmt.loc));
            checked_arms.push_back(std::move(fallback));
        }

        std::vector<IrStmtPtr> chain = build_tuple_match_if_chain(checked_arms);
        for (auto& statement : chain) {
            ir_stmt_loop_body(lowered).push_back(std::move(statement));
        }

        StateSnapshot exit_state = loop_input;
        merge_continue_states(stmt.loc, exit_state, loop_state);
        merge_break_exit_states(stmt.loc, exit_state, loop_state, "has incompatible ownership states after loop exits");
        restore_states(exit_state);
    }

    void bind_irrefutable_non_iterator_for_item(SourceLocation loc,
                                                const std::string& item_name,
                                                const IrType& item_type,
                                                std::string* loop_binding_name,
                                                IrExprPtr* item_expr,
                                                std::vector<IrStmtPtr>& pattern_prelude) {
        declare_local(loc, item_name, item_type, false);
        if (loop_binding_name) {
            *loop_binding_name = item_name;
        }
        if (item_expr && *item_expr) {
            pattern_prelude.push_back(
                make_ir_var_decl(loc, item_name, item_type, std::move(*item_expr), false)
            );
        }
    }

    void lower_irrefutable_non_iterator_for_head(const Pattern& pattern,
                                                 const IrType& item_type,
                                                 std::string* loop_binding_name,
                                                 IrExprPtr* item_expr,
                                                 std::vector<IrStmtPtr>& pattern_prelude,
                                                 bool materialize_wildcard_aggregate) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            lower_irrefutable_non_iterator_for_head(
                effective_pattern,
                item_type,
                loop_binding_name,
                item_expr,
                pattern_prelude,
                materialize_wildcard_aggregate
            );
            return;
        }
        if (pattern.kind == PatternKind::Binding) {
            bind_irrefutable_non_iterator_for_item(
                pattern.loc,
                pattern.payload_name,
                item_type,
                loop_binding_name,
                item_expr,
                pattern_prelude
            );
            return;
        }

        if (pattern.kind == PatternKind::Wildcard) {
            if (materialize_wildcard_aggregate && is_aggregate_type(item_type)) {
                bind_irrefutable_non_iterator_for_item(
                    pattern.loc,
                    make_hidden_local("$for_item"),
                    item_type,
                    loop_binding_name,
                    item_expr,
                    pattern_prelude
                );
            }
            return;
        }

        require_irrefutable_non_iterator_for_pattern(pattern, item_type, for_pattern_validation_hooks());
        std::string item_name = make_hidden_local("$for_item");
        bind_irrefutable_non_iterator_for_item(
            pattern.loc,
            item_name,
            item_type,
            loop_binding_name,
            item_expr,
            pattern_prelude
        );
        lower_product_match_pattern_bindings_from_local(
            pattern,
            item_name,
            item_type,
            pattern_prelude
        );
    }

    void require_supported_for_iterator_pattern(const Pattern& pattern, const IrType& value_type) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) {
            require_supported_for_iterator_pattern(effective_pattern, value_type);
            return;
        }
        switch (pattern.kind) {
            case PatternKind::Wildcard:
            case PatternKind::Binding:
                return;
            case PatternKind::Tuple:
            case PatternKind::Array:
            case PatternKind::Struct:
                require_irrefutable_non_iterator_for_pattern(pattern, value_type, for_pattern_validation_hooks());
                return;
            case PatternKind::EnumCase:
                if (value_type.primitive == IrPrimitiveKind::Struct) {
                    require_irrefutable_non_iterator_for_pattern(pattern, value_type, for_pattern_validation_hooks());
                    return;
                }
                if (is_value_enum_type(value_type)) return;
                fail(pattern.loc, "enum-case iterator for-loop pattern requires an enum item, got " + type_name(value_type));
            case PatternKind::Alias:
                if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased iterator for-loop pattern");
                require_supported_for_iterator_pattern(*pattern.alias_pattern, value_type);
                return;
            case PatternKind::IntegerLiteral:
                if (!is_value_integer_type(value_type)) {
                    fail(pattern.loc, "integer iterator for-loop pattern requires an integer item, got " + type_name(value_type));
                }
                return;
            case PatternKind::BoolLiteral:
                if (value_type.qualifier != TypeQualifier::Value || value_type.primitive != IrPrimitiveKind::Bool) {
                    fail(pattern.loc, "bool iterator for-loop pattern requires a bool item, got " + type_name(value_type));
                }
                return;
            case PatternKind::Range:
                if (!is_value_integer_type(value_type)) {
                    fail(pattern.loc, "range iterator for-loop pattern requires an integer item, got " + type_name(value_type));
                }
                return;
            case PatternKind::Or: {
                std::vector<Pattern> alternatives = expand_or_pattern_alternatives(pattern);
                require_same_or_pattern_bindings(pattern.loc, alternatives, value_type);
                for (const auto& alternative : alternatives) {
                    require_supported_for_iterator_pattern(alternative, value_type);
                }
                return;
            }
        }
    }

    bool iterator_binding_names_enum_case(const Pattern& pattern, const IrType& value_type) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) return iterator_binding_names_enum_case(effective_pattern, value_type);
        if (pattern.kind != PatternKind::Binding || !is_value_enum_type(value_type)) return false;
        std::string case_name = resolve_enum_case_name(pattern.payload_name);
        auto case_found = enum_cases_.find(case_name);
        if (case_found == enum_cases_.end()) return false;
        EnumCaseInfo case_info = enum_case_for_match_value(pattern.loc, case_found->second, value_type);
        if (case_info.enum_name != value_type.name) return false;
        require_enum_case_access(pattern.loc, case_info);
        return true;
    }

    Pattern normalize_iterator_item_pattern(const Pattern& pattern, const IrType& value_type) {
        const Pattern& effective_pattern = expanded_pattern(pattern);
        if (&effective_pattern != &pattern) return normalize_iterator_item_pattern(effective_pattern, value_type);
        if (iterator_binding_names_enum_case(pattern, value_type)) {
            Pattern enum_pattern;
            enum_pattern.kind = PatternKind::EnumCase;
            enum_pattern.case_name = pattern.payload_name;
            enum_pattern.loc = pattern.loc;
            return enum_pattern;
        }
        if (pattern.kind == PatternKind::Alias) {
            Pattern normalized = clone_pattern(pattern);
            if (!pattern.alias_pattern) return normalized;
            normalized.alias_pattern =
                std::make_unique<Pattern>(normalize_iterator_item_pattern(*pattern.alias_pattern, value_type));
            return normalized;
        }
        if (pattern.kind == PatternKind::Or) {
            Pattern normalized;
            normalized.kind = PatternKind::Or;
            normalized.loc = pattern.loc;
            normalized.alternatives.reserve(pattern.alternatives.size());
            for (const auto& alternative : pattern.alternatives) {
                normalized.alternatives.push_back(normalize_iterator_item_pattern(alternative, value_type));
            }
            return normalized;
        }
        return clone_pattern(pattern);
    }

    static Pattern make_iterator_some_pattern(const Pattern& item_pattern) {
        Pattern pattern;
        pattern.kind = PatternKind::EnumCase;
        pattern.case_name = "std::Some";
        pattern.loc = item_pattern.loc;
        pattern.has_payload_pattern = true;
        pattern.payload_pattern = std::make_unique<Pattern>(clone_pattern(item_pattern));
        return pattern;
    }

    static Pattern make_wildcard_pattern(SourceLocation loc) {
        Pattern pattern;
        pattern.kind = PatternKind::Wildcard;
        pattern.loc = loc;
        return pattern;
    }

    bool try_find_concrete_iterator_trait_impl(
        const std::string& trait_name,
        const IrType& self_type,
        IrType& item_type
    ) const {
        for (const auto& impl : trait_impl_coherence_) {
            if (!impl.generic_names.empty()) continue;
            if (impl.trait_name != trait_name) continue;
            if (impl.trait_args.size() != 1) continue;
            if (!same_type(impl.self_type, self_type)) continue;
            item_type = impl.trait_args[0];
            item_type.qualifier = TypeQualifier::Value;
            return true;
        }
        return false;
    }

    bool try_find_generic_iterator_trait_impl(
        const std::string& trait_name,
        const IrType& self_type,
        IrType& item_type
    ) const {
        for (const auto& impl : generic_trait_impls_) {
            if (impl.trait_name != trait_name) continue;
            if (impl.trait_args.size() != 1) continue;

            std::map<std::string, IrType> substitutions;
            if (!infer_generic_pattern_type(impl.self_type, self_type, impl.generic_names, substitutions)) continue;

            bool complete = true;
            for (const auto& generic_name : impl.generic_names) {
                if (!substitutions.count(generic_name)) {
                    complete = false;
                    break;
                }
            }
            if (!complete) continue;

            substitutions.emplace("Self", self_type);
            std::set<std::string> visiting;
            if (!impl_generic_bounds_satisfied(impl.generic_bounds, substitutions, visiting, nullptr)) continue;

            item_type = substitute_inferred_type(impl.trait_args[0], substitutions);
            item_type.qualifier = TypeQualifier::Value;
            return true;
        }
        return false;
    }

    bool try_find_for_iterator_trait_kind(
        const IrType& iterable_type,
        ForIteratorTraitKind kind,
        ForIteratorTraitMatch& match
    ) const {
        IrType self_type = iterable_type;
        self_type.qualifier = TypeQualifier::Value;

        for (const auto& trait_name : for_iterator_trait_candidates(kind)) {
            IrType item_type;
            if (try_find_concrete_iterator_trait_impl(trait_name, self_type, item_type) ||
                try_find_generic_iterator_trait_impl(trait_name, self_type, item_type)) {
                match.kind = kind;
                match.trait_name = trait_name;
                match.item_type = item_type;
                return true;
            }
        }
        return false;
    }

    bool try_find_for_iterator_trait(const IrType& iterable_type, ForIteratorTraitMatch& match) const {
        for (ForIteratorTraitKind kind : {ForIteratorTraitKind::IntoIterator, ForIteratorTraitKind::Iterator}) {
            if (try_find_for_iterator_trait_kind(iterable_type, kind, match)) return true;
        }
        return false;
    }

    IrExprPtr make_iterator_method_call(SourceLocation loc,
                                        const std::string& iterator_name,
                                        const std::string& method_name,
                                        bool borrow_mut_receiver = false) {
        Expr call;
        call.kind = ExprKind::MethodCall;
        call.loc = loc;
        call.name = method_name;
        if (borrow_mut_receiver) {
            ExprPtr receiver = make_ast_name_expr(loc, iterator_name);
            set_expr_operand(call, make_ast_borrow_expr(loc, std::move(receiver), true));
        } else {
            set_expr_operand(call, make_ast_name_expr(loc, iterator_name));
        }
        return check_expr(call);
    }

    void append_hidden_iterator_owner_cleanup(SourceLocation loc,
                                              const std::string& iterator_name,
                                              std::vector<IrStmtPtr>& statements) {
        LocalInfo* local = find_local_slot(iterator_name);
        if (!local || !local_has_live_owner(*local)) return;
        require_not_borrowed(loc, iterator_name, *local, "drop");
        DropValueFactory make_value = [this, loc, iterator_name, type = local->type]() {
            return make_local_lvalue_expr(loc, iterator_name, type);
        };
        append_drop_stmts_for_value(loc, local->type, make_value, statements, local);
        mark_all_local_owned_fields(*local, LocalState::Dropped);
        mark_local_dropped(*local);
    }

    void append_loop_exit_owner_cleanup(SourceLocation loc,
                                        const LoopInfo& loop,
                                        std::vector<IrStmtPtr>& statements) {
        for (const auto& name : loop.exit_cleanup_owner_names) {
            append_hidden_iterator_owner_cleanup(loc, name, statements);
        }
    }

    void append_active_loop_exit_owner_cleanup(SourceLocation loc,
                                               std::vector<IrStmtPtr>& statements) {
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            append_loop_exit_owner_cleanup(loc, *loop, statements);
        }
    }

    std::vector<IrStmtPtr> make_active_loop_exit_owner_cleanup_for_branch(SourceLocation loc) {
        StateSnapshot before_cleanup = snapshot_states();
        std::vector<IrStmtPtr> cleanup;
        append_active_loop_exit_owner_cleanup(loc, cleanup);
        restore_states(before_cleanup);
        return cleanup;
    }

    void append_outer_loop_exit_owner_cleanup_until(SourceLocation loc,
                                                   const LoopInfo& target,
                                                   std::vector<IrStmtPtr>& statements) {
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (&*loop == &target) return;
            append_loop_exit_owner_cleanup(loc, *loop, statements);
        }
    }

    void append_for_iterator_loop(
        const Stmt& stmt,
        IrStmt& lowered,
        IrExprPtr iterator,
        const IrType& item_type
    ) {
        const Pattern& for_pattern = expanded_pattern(*stmt.for_pattern);
        IrType iterator_type = iterator->type;
        const bool borrowed_iterator = is_borrow_type(iterator_type);
        const bool owning_iterator = is_owner_type(iterator_type);
        if (contains_borrow_type(iterator_type) && !borrowed_iterator) {
            fail(stmt.loc,
                 "Iterator[T] for-loop lowering currently requires a copyable iterator value, owning iterator value, or ref mut iterator value, got " +
                     type_name(iterator_type));
        }
        if (borrowed_iterator && iterator_type.qualifier != TypeQualifier::MutRef) {
            fail(stmt.loc,
                 "borrowed Iterator[T] for-loop values require ref mut, got " +
                     type_name(iterator_type));
        }
        if (is_owner_type(item_type) || contains_borrow_type(item_type)) {
            fail(for_pattern.loc,
                 "Iterator[T] for-loop item bindings currently require copyable non-borrow items, got " +
                     type_name(item_type));
        }
        if (stmt.for_pattern_filter &&
            (for_pattern.kind == PatternKind::Binding || for_pattern.kind == PatternKind::Wildcard)) {
            fail(for_pattern.loc, "for-let iterator filters require a refutable item pattern");
        }
        Pattern item_pattern = normalize_iterator_item_pattern(for_pattern, item_type);
        if (item_pattern.kind != PatternKind::Binding && item_pattern.kind != PatternKind::Wildcard) {
            require_supported_for_iterator_pattern(item_pattern, item_type);
        }

        push_scope();
        std::string iterator_name = make_hidden_local("$for_iter");
        declare_local(stmt.loc, iterator_name, iterator_type, true);
        if (borrowed_iterator && iterator->kind == IrExprKind::Borrow) {
            promote_temporary_borrow_to_named(stmt.loc, *iterator, iterator_name);
        }
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(stmt.loc, iterator_name, iterator_type, std::move(iterator), true));

        auto loop = std::make_unique<IrStmt>();
        loop->kind = IrStmtKind::WhileLet;
        loop->loc = stmt.loc;
        const std::string& label = stmt_label(stmt);
        set_ir_stmt_label(*loop, label);
        loop->while_let_continue_on_mismatch = stmt.for_pattern_filter;
        loop->match_value = make_iterator_method_call(stmt.loc, iterator_name, "next", owning_iterator);
        const EnumInfo& enum_info = require_enum_match_value(stmt.loc, *loop->match_value);
        IrType enum_value_type = loop->match_value->type;

        EnumMatchCoverage coverage;
        Pattern some_pattern = make_iterator_some_pattern(item_pattern);
        std::vector<IrMatchArm> pattern_arms = lower_match_arm_patterns(
            some_pattern,
            enum_info,
            enum_value_type,
            coverage
        );
        if (pattern_arms.empty()) {
            fail(for_pattern.loc, "iterator for-loop pattern did not lower to a match arm");
        }
        if (stmt.for_pattern_filter) {
            Pattern skip_pattern = make_iterator_some_pattern(make_wildcard_pattern(for_pattern.loc));
            std::vector<IrMatchArm> skip_arms = lower_match_arm_patterns(
                skip_pattern,
                enum_info,
                enum_value_type,
                coverage
            );
            if (skip_arms.size() != 1) {
                fail(for_pattern.loc, "iterator for-let fallback pattern did not lower to a single match arm");
            }
            pattern_arms.push_back(std::move(skip_arms.front()));
        }

        StateSnapshot loop_input = snapshot_states();
        LoopInfo loop_info;
        loop_info.label = label;
        if (owning_iterator) loop_info.exit_cleanup_owner_names.push_back(iterator_name);
        push_loop(stmt.loc, loop_info);
        push_scope();
        declare_match_arm_bindings(pattern_arms.front());
        CheckedStatements body = check_statements(stmt_loop_body(stmt), false);
        set_ir_stmt_loop_body(*loop, std::move(body.statements));
        if (body.flow == Flow::Returns) discard_scope();
        else if (body.flow == Flow::Stops) discard_scope();
        else {
            append_current_scope_auto_destroy_cleanup(stmt.loc, ir_stmt_loop_body(*loop));
            pop_scope();
        }
        StateSnapshot loop_body_state = snapshot_states();
        LoopInfo loop_state = loops_.back();
        loops_.pop_back();

        restore_states(checked_loop_exit_state(stmt.loc, loop_input, loop_body_state, loop_state, body.flow));
        IrStmtMatchArms& loop_arms = ensure_ir_stmt_match_arms(*loop);
        for (auto& arm : pattern_arms) {
            loop_arms.push_back(std::move(arm));
        }
        ir_stmt_statements(lowered).push_back(std::move(loop));
        append_hidden_iterator_owner_cleanup(stmt.loc, iterator_name, ir_stmt_statements(lowered));
        pop_scope();
    }

    void check_for_iterator(
        const Stmt& stmt,
        IrStmt& lowered,
        IrExprPtr iterable,
        const ForIteratorTraitMatch& iterator_match
    ) {
        lowered.kind = IrStmtKind::Block;
        if (iterator_match.kind == ForIteratorTraitKind::Iterator) {
            append_for_iterator_loop(stmt, lowered, std::move(iterable), iterator_match.item_type);
            return;
        }

        if (is_owner_type(iterable->type) || contains_borrow_type(iterable->type)) {
            fail(stmt.loc,
                 "IntoIterator[T] for-loop conversion currently requires a copyable non-borrow value, got " +
                     type_name(iterable->type));
        }
        std::string source_name = make_hidden_local("$for_into");
        IrType source_type = iterable->type;
        declare_local(stmt.loc, source_name, source_type, true);
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(stmt.loc, source_name, source_type, std::move(iterable), true));

        IrExprPtr iterator = make_iterator_method_call(stmt.loc, source_name, "into_iter");
        ForIteratorTraitMatch direct_iterator;
        if (!try_find_for_iterator_trait_kind(iterator->type, ForIteratorTraitKind::Iterator, direct_iterator)) {
            std::string trait_display =
                for_iterator_trait_display(iterator_match.trait_name, iterator_match.item_type);
            fail(stmt.loc,
                 "for over " + trait_display + " value of type " + type_name(source_type) +
                     " calls into_iter() but the result type " + type_name(iterator->type) +
                     " does not implement Iterator[" + type_name(iterator_match.item_type) + "]");
        }
        if (!same_type(direct_iterator.item_type, iterator_match.item_type)) {
            std::string trait_display =
                for_iterator_trait_display(iterator_match.trait_name, iterator_match.item_type);
            fail(stmt.loc,
                 "for over " + trait_display + " value of type " + type_name(source_type) +
                     " converts to Iterator[" + type_name(direct_iterator.item_type) + "]");
        }
        append_for_iterator_loop(stmt, lowered, std::move(iterator), direct_iterator.item_type);
    }

    void check_for(const Stmt& stmt, IrStmt& lowered) {
        const Pattern& for_pattern = expanded_pattern(*stmt.for_pattern);
        std::string range_call_name;
        bool is_range_call = false;
        if (stmt.for_iterable && stmt.for_iterable->kind == ExprKind::Call) {
            range_call_name = resolve_use_path(stmt.for_iterable->name);
            is_range_call =
                prelude_specials_available() &&
                source_std_generic_function_available(range_call_name) &&
                !unqualified_decl_shadows_prelude_name(stmt.for_iterable->name, range_call_name) &&
                is_prelude_range_function_name(range_call_name);
        }
        if (is_range_call) {
            if (stmt.for_pattern_filter) {
                fail(for_pattern.loc, "for-let filters currently require an Iterator[T] or IntoIterator[T] value");
            }
            check_for_range(stmt, lowered, range_call_name);
            return;
        }
        if (stmt.for_iterable && stmt.for_iterable->kind == ExprKind::Vector) {
            if (stmt.for_pattern_filter) {
                fail(for_pattern.loc, "for-let filters currently require an Iterator[T] or IntoIterator[T] value");
            }
            check_for_vector(stmt, lowered);
            return;
        }
        IrExprPtr iterable = check_expr(*stmt.for_iterable);
        if (is_prelude_range_type(iterable->type)) {
            if (stmt.for_pattern_filter) {
                fail(for_pattern.loc, "for-let filters currently require an Iterator[T] or IntoIterator[T] value");
            }
            check_for_range_value(stmt, lowered, std::move(iterable));
            return;
        }
        if (iterable->type.primitive == IrPrimitiveKind::Vector) {
            if (stmt.for_pattern_filter) {
                fail(for_pattern.loc, "for-let filters currently require an Iterator[T] or IntoIterator[T] value");
            }
            check_for_vector_value(stmt, lowered, std::move(iterable));
            return;
        }
        ForIteratorTraitMatch iterator_match;
        if (try_find_for_iterator_trait(iterable->type, iterator_match)) {
            check_for_iterator(stmt, lowered, std::move(iterable), iterator_match);
            return;
        }
        fail(stmt.loc,
             "for currently supports range values, range(start, end), range_inclusive(start, end), 0..end, 0..=end, list literals, stored local vector values, direct Iterator[T] values, or copyable IntoIterator[T] values");
    }

    void check_for_range(const Stmt& stmt, IrStmt& lowered, const std::string& call_name = "") {
        const Expr& call = *stmt.for_iterable;
        const std::string& range_name = call_name.empty() ? call.name : call_name;
        if (!is_prelude_range_function_name(range_name)) {
            fail(call.loc, "for currently supports range(start, end) iterator expressions");
        }
        IrExprPtr range = check_range_call(call, std::make_unique<IrExpr>(), nullptr, range_name);
        if (range->args.size() != 2) throw CompileError("internal error: range call did not lower to two bounds");
        bool inclusive = range->type.name == "RangeInclusive";
        IrExprPtr start = std::move(range->args[0]);
        IrExprPtr end = std::move(range->args[1]);
        finish_for_range(stmt, lowered, std::move(start), std::move(end), inclusive);
    }

    void check_for_range_value(const Stmt& stmt, IrStmt& lowered, IrExprPtr iterable) {
        IrType range_type = iterable->type;
        bool inclusive = range_type.name == "RangeInclusive";

        lowered.kind = IrStmtKind::Block;
        std::string range_name = make_hidden_local("$for_range");
        declare_local(stmt.loc, range_name, range_type, false);
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(stmt.loc, range_name, range_type, std::move(iterable), false));

        auto loop = std::make_unique<IrStmt>();
        loop->loc = stmt.loc;
        IrExprPtr start = make_tuple_index_expr(stmt.loc, range_name, range_type, 0);
        IrExprPtr end = make_tuple_index_expr(stmt.loc, range_name, range_type, 1);
        finish_for_range(stmt, *loop, std::move(start), std::move(end), inclusive);
        ir_stmt_statements(lowered).push_back(std::move(loop));
    }

    void finish_for_range(const Stmt& stmt, IrStmt& lowered, IrExprPtr start, IrExprPtr end, bool inclusive) {
        const Pattern& for_pattern = expanded_pattern(*stmt.for_pattern);
        IrType bound_type = start->type;
        lowered.kind = IrStmtKind::ForRange;
        const std::string& label = stmt_label(stmt);
        set_ir_stmt_label(lowered, label);
        IrStmtForPayload& for_loop = ensure_ir_stmt_for_payload(lowered);
        for_loop.inclusive = inclusive;
        for_loop.binding_type = bound_type;
        for_loop.start = std::move(start);
        for_loop.end = std::move(end);
        for_loop.index_name = make_hidden_local("$for_index");
        for_loop.end_name = make_hidden_local("$for_end");
        declare_local(stmt.loc, for_loop.index_name, bound_type, true);
        declare_local(stmt.loc, for_loop.end_name, bound_type, false);

        StateSnapshot loop_input = snapshot_states();
        LoopInfo loop;
        loop.label = label;
        push_loop(stmt.loc, loop);
        push_scope();
        std::vector<IrStmtPtr> pattern_prelude;
        lower_irrefutable_non_iterator_for_head(
            for_pattern,
            bound_type,
            &for_loop.binding_name,
            nullptr,
            pattern_prelude,
            false
        );
        CheckedStatements body = check_statements(stmt_loop_body(stmt), false);
        set_ir_stmt_loop_body(lowered, std::move(pattern_prelude));
        for (auto& body_stmt : body.statements) {
            ir_stmt_loop_body(lowered).push_back(std::move(body_stmt));
        }
        if (body.flow == Flow::Returns) discard_scope();
        else if (body.flow == Flow::Stops) discard_scope();
        else {
            append_current_scope_auto_destroy_cleanup(stmt.loc, ir_stmt_loop_body(lowered));
            pop_scope();
        }
        StateSnapshot loop_body_state = snapshot_states();
        LoopInfo loop_state = loops_.back();
        loops_.pop_back();

        restore_states(checked_loop_exit_state(stmt.loc, loop_input, loop_body_state, loop_state, body.flow));
    }

    void check_for_vector(const Stmt& stmt, IrStmt& lowered) {
        const Pattern& for_pattern = expanded_pattern(*stmt.for_pattern);
        IrExprPtr iterable = check_expr(*stmt.for_iterable);
        if (iterable->kind != IrExprKind::Vector || iterable->type.args.size() != 1) {
            fail(stmt.loc, "for currently supports list literal iterator expressions");
        }

        lowered.kind = IrStmtKind::ForVector;
        const std::string& label = stmt_label(stmt);
        set_ir_stmt_label(lowered, label);
        IrStmtForPayload& for_loop = ensure_ir_stmt_for_payload(lowered);
        for_loop.binding_type = iterable->type.args[0];
        for_loop.values = iterable->args.take();

        StateSnapshot loop_input = snapshot_states();
        LoopInfo loop;
        loop.label = label;
        push_loop(stmt.loc, loop);
        push_scope();
        std::vector<IrStmtPtr> pattern_prelude;
        lower_irrefutable_non_iterator_for_head(
            for_pattern,
            for_loop.binding_type,
            &for_loop.binding_name,
            nullptr,
            pattern_prelude,
            true
        );
        CheckedStatements body = check_statements(stmt_loop_body(stmt), false);
        set_ir_stmt_loop_body(lowered, std::move(pattern_prelude));
        for (auto& body_stmt : body.statements) {
            ir_stmt_loop_body(lowered).push_back(std::move(body_stmt));
        }
        if (body.flow == Flow::Returns) discard_scope();
        else if (body.flow == Flow::Stops) discard_scope();
        else {
            append_current_scope_auto_destroy_cleanup(stmt.loc, ir_stmt_loop_body(lowered));
            pop_scope();
        }
        StateSnapshot loop_body_state = snapshot_states();
        LoopInfo loop_state = loops_.back();
        loops_.pop_back();

        restore_states(checked_loop_exit_state(stmt.loc, loop_input, loop_body_state, loop_state, body.flow));
    }

    void check_for_vector_value(const Stmt& stmt, IrStmt& lowered, IrExprPtr iterable) {
        const Pattern& for_pattern = expanded_pattern(*stmt.for_pattern);
        if (iterable->type.args.size() != 1) {
            fail(stmt.loc, "for currently supports stored local vector values with a known length");
        }

        IrType vector_type = iterable->type;
        IrType element_type = vector_type.args[0];
        IrType i64 = i64_type(stmt.loc);
        VectorKnownLength current_length =
            vector_known_length_from_source_expr(vector_type, *stmt.for_iterable, *iterable);

        lowered.kind = IrStmtKind::Block;
        std::string vector_name = make_hidden_local("$for_vec");
        declare_local(stmt.loc, vector_name, vector_type, false);
        ir_stmt_statements(lowered).push_back(make_ir_var_decl(stmt.loc, vector_name, vector_type, std::move(iterable), false));

        auto loop = std::make_unique<IrStmt>();
        loop->kind = IrStmtKind::ForRange;
        loop->loc = stmt.loc;
        const std::string& label = stmt_label(stmt);
        set_ir_stmt_label(*loop, label);
        IrStmtForPayload& for_loop = ensure_ir_stmt_for_payload(*loop);
        for_loop.inclusive = false;
        for_loop.binding_type = i64;
        for_loop.start = make_integer_literal(stmt.loc, i64, 0);
        for_loop.end = make_local_vec_len_expr(
            stmt.loc,
            make_vec_local_lvalue(stmt.loc, vector_name, vector_type),
            current_length
        );
        for_loop.index_name = make_hidden_local("$for_index");
        for_loop.end_name = make_hidden_local("$for_end");
        declare_local(stmt.loc, for_loop.index_name, i64, true);
        declare_local(stmt.loc, for_loop.end_name, i64, false);

        StateSnapshot loop_input = snapshot_states();
        LoopInfo loop_info;
        loop_info.label = label;
        push_loop(stmt.loc, loop_info);
        push_scope();

        std::vector<IrStmtPtr> pattern_prelude;
        IrExprPtr element = make_vector_index_expr(stmt.loc, vector_name, vector_type, for_loop.index_name, i64);
        lower_irrefutable_non_iterator_for_head(
            for_pattern,
            element_type,
            nullptr,
            &element,
            pattern_prelude,
            true
        );

        CheckedStatements body = check_statements(stmt_loop_body(stmt), false);
        set_ir_stmt_loop_body(*loop, std::move(pattern_prelude));
        for (auto& body_stmt : body.statements) {
            ir_stmt_loop_body(*loop).push_back(std::move(body_stmt));
        }
        if (body.flow == Flow::Returns) discard_scope();
        else if (body.flow == Flow::Stops) discard_scope();
        else {
            append_current_scope_auto_destroy_cleanup(stmt.loc, ir_stmt_loop_body(*loop));
            pop_scope();
        }
        StateSnapshot loop_body_state = snapshot_states();
        LoopInfo loop_state = loops_.back();
        loops_.pop_back();

        restore_states(checked_loop_exit_state(stmt.loc, loop_input, loop_body_state, loop_state, body.flow));
        ir_stmt_statements(lowered).push_back(std::move(loop));
    }

    std::string make_hidden_local(const std::string& prefix) {
        return prefix + std::to_string(hidden_local_counter_++);
    }

    void push_loop(SourceLocation loc, LoopInfo loop) {
        if (!loop.label.empty()) {
            for (const auto& active : loops_) {
                if (active.label == loop.label) {
                    fail(loc, "duplicate active loop label '" + loop.label + "'");
                }
            }
        }
        loop.scope_depth = local_scopes_.size();
        loops_.push_back(loop);
    }

    void push_labeled_block(SourceLocation loc, const std::string& label) {
        for (const auto& active : loops_) {
            if (active.label == label) {
                fail(loc, "duplicate active label '" + label + "'");
            }
        }
        LoopInfo block;
        block.label = label;
        block.is_loop = false;
        block.scope_depth = local_scopes_.size();
        loops_.push_back(block);
    }

    void apply_init_while_update_state(SourceLocation loc, const LoopInfo& loop) {
        for (std::size_t i = 0; i < loop.names.size(); ++i) {
            const std::string& name = loop.names[i];
            LocalInfo& target = require_local_slot(loc, name);
            require_not_borrowed(loc, name, target, "assign to");
            if (is_owner_type(loop.types[i]) && local_has_live_owner(target)) {
                fail(loc,
                     "cannot overwrite owning init-while binding '" + name +
                         "' before it is moved or dropped");
            }
        }

        for (std::size_t i = 0; i < loop.names.size(); ++i) {
            if (!is_owner_type(loop.types[i])) continue;
            LocalInfo& target = local_slot_by_name(loop.names[i]);
            mark_local_alive(target);
            initialize_owned_field_states(target);
        }
    }

    std::vector<IrExprPtr> check_init_while_update_exprs(
        SourceLocation loc,
        const LoopInfo& loop,
        const std::vector<ExprPtr>& updates
    ) {
        if (updates.size() != loop.types.size()) {
            fail(loc, "next value count must match init binding count");
        }
        std::vector<IrExprPtr> lowered_updates;
        lowered_updates.reserve(updates.size());
        for (std::size_t i = 0; i < updates.size(); ++i) {
            IrExprPtr update = check_expr(*updates[i]);
            coerce_expr_to_expected(*update, loop.types[i]);
            require_assignable(loc, loop.types[i], update->type);
            lowered_updates.push_back(std::move(update));
        }
        return lowered_updates;
    }

    Flow check_init_while(const Stmt& stmt, IrStmt& lowered) {
        const std::string& label = stmt_label(stmt);
        LoopInfo loop;
        loop.supports_values = true;
        loop.label = label;
        set_ir_stmt_label(lowered, label);
        for (const auto& binding : stmt.init_bindings) {
            IrExprPtr init = check_expr(*binding.init);
            IrType declared = binding.has_type ? resolve_executable_type(binding.type) : init->type;
            coerce_expr_to_expected(*init, declared);
            require_assignable(binding.loc, declared, init->type);
            if (is_borrow_type(declared)) {
                fail(binding.loc, "borrow bindings are not supported in init-while yet");
            }
            declare_local(binding.loc, binding.name, declared, true);
            loop.names.push_back(binding.name);
            loop.types.push_back(declared);

            IrBinding ir_binding;
            ir_binding.name = binding.name;
            ir_binding.type = declared;
            ir_binding.init = std::move(init);
            ir_binding.mutable_binding = true;
            ir_binding.loc = binding.loc;
            lowered.init_bindings.push_back(std::move(ir_binding));
        }

        lowered.condition = check_expr(*stmt.condition);
        coerce_condition_to_bool(stmt.loc, lowered.condition);
        const std::optional<bool> known_condition = known_bool_condition_value(*lowered.condition);
        const bool literal_true_condition = known_condition.value_or(false);
        StateSnapshot loop_input = snapshot_states();

        push_loop(stmt.loc, loop);
        CheckedStatements body = check_statements(stmt_loop_body(stmt), true);
        set_ir_stmt_loop_body(lowered, std::move(body.statements));
        StateSnapshot loop_body_state = snapshot_states();
        StateSnapshot next_iteration_state = loop_input;
        if (known_condition && !*known_condition) {
            restore_states(loop_input);
            if (!stmt.updates.empty()) {
                lowered.updates = check_init_while_update_exprs(stmt.loc, loop, stmt.updates);
                restore_states(loop_input);
            }
            loops_.pop_back();
            restore_states(loop_input);
            return Flow::Continues;
        }
        if (body.flow != Flow::Continues && !stmt.updates.empty()) {
            restore_states(loop_input);
            lowered.updates = check_init_while_update_exprs(stmt.loc, loop, stmt.updates);
            restore_states(loop_input);
        }
        if (body.flow == Flow::Continues && stmt.updates.empty()) {
            require_same_states(stmt.loc, loop_input, loop_body_state, "cannot change ownership state inside loop yet");
            merge_existing_zone_generations_into(next_iteration_state, loop_body_state);
        } else if (body.flow == Flow::Continues) {
            require_same_states_except_loop_bindings(
                stmt.loc,
                loop_input,
                loop_body_state,
                loop,
                "cannot change ownership state inside loop except updated init-while bindings"
            );
        }

        if (body.flow == Flow::Continues && !stmt.updates.empty()) {
            lowered.updates = check_init_while_update_exprs(stmt.loc, loop, stmt.updates);
            apply_init_while_update_state(stmt.loc, loop);
            StateSnapshot update_state = snapshot_states();
            require_same_states(stmt.loc, loop_input, update_state, "cannot change ownership state in loop updates yet");
            merge_existing_zone_generations_into(next_iteration_state, update_state);
        }
        LoopInfo loop_state = loops_.back();
        loops_.pop_back();
        const bool has_zero_iteration_exit = !literal_true_condition;
        StateSnapshot exit_state = loop_exit_base_state(loop_input, loop_state, has_zero_iteration_exit);
        if (has_zero_iteration_exit && body.flow == Flow::Continues) {
            merge_existing_zone_generations_into(exit_state, next_iteration_state);
        }
        merge_loop_continue_snapshots(
            stmt.loc,
            exit_state,
            std::move(next_iteration_state),
            loop_state,
            has_zero_iteration_exit
        );
        merge_break_exit_states(stmt.loc, exit_state, loop_state, "has incompatible ownership states after loop exits");
        restore_states(exit_state);
        if (literal_true_condition && body.flow == Flow::Returns) return Flow::Returns;
        if (literal_true_condition && literal_true_loop_never_falls_through(loop_state, body.flow)) {
            return Flow::Stops;
        }
        return Flow::Continues;
    }

    const LoopInfo& loop_for_break(SourceLocation loc, const std::string& label) const {
        if (label.empty()) {
            for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
                if (loop->is_loop) return *loop;
            }
            fail(loc, "break used outside a loop");
        }
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (loop->label == label) return *loop;
        }
        fail(loc, "unknown loop label '" + label + "'");
    }

    LoopInfo& mutable_loop_for_break(SourceLocation loc, const std::string& label) {
        if (label.empty()) {
            for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
                if (loop->is_loop) return *loop;
            }
            fail(loc, "break used outside a loop");
        }
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (loop->label == label) return *loop;
        }
        fail(loc, "unknown loop label '" + label + "'");
    }

    LoopInfo& mutable_loop_for_continue(SourceLocation loc) {
        for (auto loop = loops_.rbegin(); loop != loops_.rend(); ++loop) {
            if (loop->is_loop) return *loop;
        }
        fail(loc, "continue used outside a loop");
    }

    void check_break(const Stmt& stmt, IrStmt& lowered) {
        const std::string& break_label = stmt_break_label(stmt);
        const ExprPtr& break_value = stmt_break_value(stmt);
        LoopInfo& target = mutable_loop_for_break(stmt.loc, break_label);
        set_ir_stmt_break_label(lowered, break_label);
        if (!break_value) {
            if (target.supports_break_values) {
                fail(stmt.loc, "break from labeled block expression must provide a value");
            }
            std::vector<IrStmtPtr> cleanup;
            append_auto_destroy_zone_cleanup(stmt.loc, cleanup, target.scope_depth);
            append_outer_loop_exit_owner_cleanup_until(stmt.loc, target, cleanup);
            require_no_live_owners_before_scope_jump(stmt.loc, target.scope_depth, "break");
            target.break_state_snapshots.push_back(snapshot_states());
            if (!cleanup.empty()) {
                auto break_stmt = std::make_unique<IrStmt>();
                break_stmt->kind = IrStmtKind::Break;
                break_stmt->loc = stmt.loc;
                set_ir_stmt_break_label(*break_stmt, break_label);
                cleanup.push_back(std::move(break_stmt));
                lowered.kind = IrStmtKind::Block;
                set_ir_stmt_statements(lowered, std::move(cleanup));
            }
            return;
        }
        if (!target.supports_break_values) {
            fail(stmt.loc, "break values are only valid for labeled block expressions");
        }
        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = target.has_break_result_type
            ? check_expr_with_expected(*break_value, target.break_result_type)
            : check_expr(*break_value);
        std::optional<BorrowResultSource> borrow_source;
        if (is_borrow_type(value->type)) {
            borrow_source = expr_borrow_result_source(*value);
            if (!borrow_source) {
                fail(stmt.loc, "break value borrow result must come directly from ref, ref mut, or a compatible borrow control-flow result");
            }
            require_borrow_result_source_outlives_scope(
                stmt.loc,
                *borrow_source,
                target.scope_depth,
                "labeled block break value");
            release_temporary_borrows(borrow_mark);
        } else if (contains_borrow_type(value->type)) {
            fail(stmt.loc, "break values cannot contain borrow values yet");
        }
        if (is_void_value_type(value->type)) {
            fail(stmt.loc, "break value must produce a value");
        }
        if (target.has_break_result_type) {
            widen_vector_result_storage(target.break_result_type, *value);
            coerce_expr_to_expected(*value, target.break_result_type);
            require_assignable(stmt.loc, target.break_result_type, value->type);
        } else {
            target.break_result_type = value->type;
            target.has_break_result_type = true;
        }
        if (borrow_source) {
            require_same_borrow_result_source(
                stmt.loc,
                "labeled block expression",
                target.break_borrow_source,
                *borrow_source);
        }
        std::vector<IrStmtPtr> cleanup;
        if (borrow_source) {
            require_zone_pointer_not_escape_temporary_scope(
                stmt.loc,
                *value,
                target.scope_depth,
                "labeled block break value");
            append_auto_destroy_zone_cleanup(stmt.loc, cleanup, target.scope_depth);
        } else {
            value = materialize_value_before_auto_destroy_cleanup(
                stmt.loc,
                std::move(value),
                cleanup,
                target.scope_depth,
                "$break",
                "labeled block break value"
            );
        }
        append_outer_loop_exit_owner_cleanup_until(stmt.loc, target, cleanup);
        require_no_live_owners_before_scope_jump(stmt.loc, target.scope_depth, "break");
        target.break_state_snapshots.push_back(snapshot_states());
        if (!cleanup.empty()) {
            auto break_stmt = std::make_unique<IrStmt>();
            break_stmt->kind = IrStmtKind::Break;
            break_stmt->loc = stmt.loc;
            set_ir_stmt_break_label(*break_stmt, break_label);
            set_ir_stmt_break_value(*break_stmt, std::move(value));
            cleanup.push_back(std::move(break_stmt));
            lowered.kind = IrStmtKind::Block;
            set_ir_stmt_statements(lowered, std::move(cleanup));
            return;
        }
        set_ir_stmt_break_value(lowered, std::move(value));
    }

    void check_continue(const Stmt& stmt, IrStmt& lowered) {
        LoopInfo& loop = mutable_loop_for_continue(stmt.loc);
        if (stmt.updates.empty()) {
            if (loop.supports_values && loop_has_owner_bindings(loop)) {
                fail(stmt.loc, "plain continue in owning init-while loops must provide update values");
            }
            std::vector<IrStmtPtr> cleanup;
            append_auto_destroy_zone_cleanup(stmt.loc, cleanup, loop.scope_depth);
            require_no_live_owners_before_scope_jump(stmt.loc, loop.scope_depth, "continue");
            loop.continue_state_snapshots.push_back(snapshot_states());
            if (!cleanup.empty()) {
                auto continue_stmt = std::make_unique<IrStmt>();
                continue_stmt->kind = IrStmtKind::Continue;
                continue_stmt->loc = stmt.loc;
                cleanup.push_back(std::move(continue_stmt));
                lowered.kind = IrStmtKind::Block;
                set_ir_stmt_statements(lowered, std::move(cleanup));
            }
            return;
        }
        if (!loop.supports_values) fail(stmt.loc, "continue values are only valid inside init-while loops");
        if (stmt.updates.size() != loop.types.size()) {
            fail(stmt.loc, "continue value count must match init binding count");
        }
        std::vector<IrExprPtr> updates;
        updates.reserve(stmt.updates.size());
        for (std::size_t i = 0; i < stmt.updates.size(); ++i) {
            IrExprPtr update = check_expr(*stmt.updates[i]);
            coerce_expr_to_expected(*update, loop.types[i]);
            require_assignable(stmt.loc, loop.types[i], update->type);
            updates.push_back(std::move(update));
        }
        apply_init_while_update_state(stmt.loc, loop);
        std::vector<IrStmtPtr> cleanup;
        materialize_values_before_auto_destroy_cleanup(
            stmt.loc,
            updates,
            cleanup,
            loop.scope_depth,
            "$continue",
            "continue value");
        require_no_live_owners_before_scope_jump(stmt.loc, loop.scope_depth, "continue");
        loop.continue_state_snapshots.push_back(snapshot_states());
        if (!cleanup.empty()) {
            auto continue_stmt = std::make_unique<IrStmt>();
            continue_stmt->kind = IrStmtKind::Continue;
            continue_stmt->loc = stmt.loc;
            continue_stmt->updates = std::move(updates);
            cleanup.push_back(std::move(continue_stmt));
            lowered.kind = IrStmtKind::Block;
            set_ir_stmt_statements(lowered, std::move(cleanup));
            return;
        }
        lowered.updates = std::move(updates);
    }

    static bool is_empty_vector_literal_expr(const Expr& expr) {
        return expr.kind == ExprKind::Vector && expr.args.empty();
    }

    IrExprPtr make_typed_empty_vector_expr(SourceLocation loc, const IrType& expected) const {
        const IrType& element = require_typed_empty_vector_element_type(loc, expected);
        require_plain_prelude_aggregate_element(loc, element, "vector");
        return make_empty_vector_literal_expr(loc, element);
    }

    IrExprPtr check_expr_with_expected(const Expr& expr, const IrType& expected) {
        if (is_empty_vector_literal_expr(expr)) {
            return make_typed_empty_vector_expr(expr.loc, expected);
        }
        if (expr.kind == ExprKind::Name) {
            std::string generic_name;
            if (const FunctionDecl* generic = find_generic_function(expr, generic_name)) {
                if (IrExprPtr ref = check_generic_function_ref_with_expected(expr, *generic, generic_name, expected)) {
                    return ref;
                }
            }
            if (is_borrow_type(expected)) {
                if (LocalInfo* local = find_local_slot(expr.name)) {
                    if (is_borrow_type(local->type)) {
                        require_can_borrow_path(
                            expr.loc,
                            expr.name,
                            *local,
                            "",
                            expected.qualifier == TypeQualifier::MutRef);
                    }
                }
            }
        }
        if (expr.kind == ExprKind::Call) {
            std::string range_name = resolve_use_path(expr.name);
            if (prelude_specials_available() &&
                source_std_generic_function_available(range_name) &&
                !unqualified_decl_shadows_prelude_name(expr.name, range_name) &&
                is_prelude_range_function_name(range_name) &&
                is_prelude_range_type(expected)) {
                return check_range_call(expr, std::make_unique<IrExpr>(), &expected, range_name);
            }
            auto lowered = std::make_unique<IrExpr>();
            lowered->loc = expr.loc;
            return check_call(expr, std::move(lowered), &expected);
        }
        if (expr.kind == ExprKind::If || expr.kind == ExprKind::Match || expr.kind == ExprKind::Block) {
            auto lowered = std::make_unique<IrExpr>();
            lowered->loc = expr.loc;
            if (expr.kind == ExprKind::If) return check_if_expr(expr, std::move(lowered), &expected);
            if (expr.kind == ExprKind::Match) return check_match_expr(expr, std::move(lowered), &expected);
            return check_block_expr(expr, std::move(lowered), &expected);
        }
        if (expr.kind == ExprKind::Tuple ||
            expr.kind == ExprKind::StructLiteral ||
            expr.kind == ExprKind::Vector) {
            auto lowered = std::make_unique<IrExpr>();
            lowered->loc = expr.loc;
            if (expr.kind == ExprKind::Tuple) return check_tuple(expr, std::move(lowered), &expected);
            if (expr.kind == ExprKind::StructLiteral) {
                return check_struct_literal(expr, std::move(lowered), &expected);
            }
            return check_vector(expr, std::move(lowered), &expected);
        }
        return check_expr(expr);
    }

    IrExprPtr check_expr(const Expr& expr) {
        auto lowered = std::make_unique<IrExpr>();
        lowered->loc = expr.loc;
        switch (expr.kind) {
            case ExprKind::Integer: {
                IrType literal_type = expr.literal_suffix.empty()
                    ? i64_type(expr.loc)
                    : integer_literal_suffix_type(expr.literal_suffix, expr.loc);
                IrExprPtr literal = make_integer_literal(expr.loc, literal_type, expr.int_value, expr.int_negative);
                if (!expr.literal_suffix.empty() && !integer_literal_fits(*literal, literal->type)) {
                    fail(expr.loc, "integer literal " + integer_literal_name(*literal) +
                                   " is out of range for " + type_name(literal->type));
                }
                return literal;
            }
            case ExprKind::Float: {
                IrType literal_type = expr.literal_suffix.empty()
                    ? primitive_type(IrPrimitiveKind::F64, "f64", expr.loc)
                    : float_literal_suffix_type(expr.literal_suffix, expr.loc);
                return make_float_literal_expr(expr.loc, literal_type, expr.float_value);
            }
            case ExprKind::String:
                return make_string_literal_expr(
                    expr.loc,
                    primitive_type(IrPrimitiveKind::String, "string", expr.loc),
                    expr.string_value
                );
            case ExprKind::Bool:
                return make_bool_literal_expr(expr.loc, expr.bool_value);
            case ExprKind::Null:
                return make_null_literal_expr(expr.loc, null_pointer_type(expr.loc));
            case ExprKind::Name: {
                LocalInfo* local_slot = find_local_slot(expr.name);
                if (!local_slot) {
                    ConstantValue constant_value;
                    if (resolve_constant_value(expr.loc, expr.name, constant_value)) {
                        return make_constant_expr(expr.loc, constant_value);
                    }
                    std::string case_name = resolve_enum_case_name(expr.name);
                    auto case_found = enum_cases_.find(case_name);
                    if (case_found != enum_cases_.end()) {
                        const EnumCaseInfo& info = case_found->second;
                        require_enum_case_access(expr.loc, info);
                        if (info.is_generic) {
                            fail(expr.loc,
                                 "generic enum case '" + expr.name +
                                     "' requires call syntax with type arguments");
                        }
                        if (!info.payloads.empty()) fail(expr.loc, "enum case '" + expr.name + "' requires a payload");
                        return make_enum_construct(expr.loc, info, {});
                    }
                    std::string function_name = resolve_function_name(expr.name);
                    auto function_found = functions_.find(function_name);
                    if (function_found != functions_.end()) {
                        const FunctionSig& sig = function_found->second;
                        if (sig.is_variadic) {
                            fail(expr.loc, "variadic extern function '" + expr.name + "' cannot be used as a function pointer value");
                        }
                        require_function_access(expr.loc, sig, function_name);
                        if (sig.deprecated) {
                            warn_deprecated_use(expr.loc, "function", function_name, sig.deprecated_message);
                        }
                        return make_function_ref_expr(expr.loc, function_name, function_pointer_type(sig, expr.loc));
                    }
                    std::string generic_function_name = resolve_generic_function_name(expr.name);
                    if (generic_functions_.count(generic_function_name)) {
                        fail(expr.loc, "generic function '" + expr.name + "' cannot be used as a function pointer yet");
                    }
                    fail(expr.loc, "unknown name '" + expr.name + "'");
                }
                LocalInfo& local = *local_slot;
                if (auto error = local_unavailable_binding_error(expr.name, local)) fail(expr.loc, *error);
                if (is_borrow_type(local.type) && local.borrow_sources_released) {
                    fail(expr.loc, "cannot use expired borrow binding '" + expr.name + "'");
                }
                require_can_read_borrow_path(expr.loc, expr.name, local, "");
                require_zone_pointer_valid(expr.loc, expr.name, local);
                copy_aggregate_borrow_sources_to_temporaries(expr.loc, expr.name, local);
                lowered->kind = IrExprKind::Local;
                set_ir_expr_name(*lowered, expr.name);
                lowered->type = local.type;
                if (is_owner_type(local.type)) {
                    if (is_auto_destroy_zone(local)) {
                        fail(expr.loc,
                             "temporary zone '" + expr.name +
                                 "' cannot be moved; it is destroyed automatically at function exit");
                    }
                    require_not_borrowed(expr.loc, expr.name, local, "move");
                    if (local_has_moved_or_dropped_owned_fields(local)) {
                        fail(expr.loc, "cannot move partially moved owning binding '" + expr.name + "'");
                    }
                    mark_local_moved(local);
                }
                return lowered;
            }
            case ExprKind::Borrow:
                return check_borrow(expr, std::move(lowered));
            case ExprKind::Unary:
                return check_unary(expr, std::move(lowered));
            case ExprKind::Cast:
                return check_cast(expr, std::move(lowered));
            case ExprKind::Try:
                return check_try(expr);
            case ExprKind::NullCoalesce:
                return check_null_coalesce(expr);
            case ExprKind::Tuple:
                return check_tuple(expr, std::move(lowered));
            case ExprKind::TupleIndex:
                return check_tuple_index(expr, std::move(lowered));
            case ExprKind::Index:
                return check_index(expr, std::move(lowered));
            case ExprKind::FieldAccess:
                return check_field_access(expr, std::move(lowered));
            case ExprKind::StructLiteral:
                return check_struct_literal(expr, std::move(lowered));
            case ExprKind::Vector:
                return check_vector(expr, std::move(lowered));
            case ExprKind::MacroCall:
                return check_macro_call(expr);
            case ExprKind::MethodCall:
                return check_method_call(expr, std::move(lowered));
            case ExprKind::Match:
                return check_match_expr(expr, std::move(lowered));
            case ExprKind::If:
                return check_if_expr(expr, std::move(lowered));
            case ExprKind::Block:
                return check_block_expr(expr, std::move(lowered));
            case ExprKind::Call:
                return check_call(expr, std::move(lowered));
            case ExprKind::Binary:
                return check_binary(expr, std::move(lowered));
        }
        lowered->type.primitive = IrPrimitiveKind::Unknown;
        return lowered;
    }

    IrExprPtr check_expr_maybe_expected(const Expr& expr, const IrType* expected) {
        return expected ? check_expr_with_expected(expr, *expected) : check_expr(expr);
    }

    const std::vector<IrType>& aggregate_field_types(const IrType& type) const {
        if (type.primitive == IrPrimitiveKind::Struct ||
            type.primitive == IrPrimitiveKind::Array) return type.field_types;
        return type.args;
    }

    std::size_t struct_field_index(SourceLocation loc, const IrType& type, const std::string& field_name) const {
        for (std::size_t i = 0; i < type.field_names.size(); ++i) {
            if (type.field_names[i] == field_name) return i;
        }
        fail(loc, "struct '" + type.name + "' has no field '" + field_name + "'");
    }

    bool is_struct_generic_name(const StructInfo& info, const std::string& name) const {
        return std::find(info.generic_names.begin(), info.generic_names.end(), name) != info.generic_names.end();
    }

    void record_inferred_struct_type_arg(
        SourceLocation loc,
        const StructInfo& info,
        const std::string& generic_name,
        const IrType& actual,
        std::map<std::string, IrType>& inferred
    ) const {
        if (!is_struct_generic_name(info, generic_name)) return;
        auto found = inferred.find(generic_name);
        if (found != inferred.end()) {
            if (!same_type(found->second, actual)) {
                fail(loc,
                     "conflicting inferred type for generic parameter '" + generic_name +
                         "': " + type_name(found->second) + " and " + type_name(actual));
            }
            return;
        }
        inferred.emplace(generic_name, actual);
    }

    bool infer_struct_type_arg_from_value_type(
        const TypeRef& pattern,
        const IrType& actual,
        const StructInfo& info,
        std::map<std::string, IrType>& inferred
    ) {
        if (pattern.args.empty() && is_struct_generic_name(info, pattern.name)) {
            IrType inferred_type = actual;
            if (pattern.qualifier != TypeQualifier::Value) {
                if (actual.qualifier != pattern.qualifier) return false;
                inferred_type.qualifier = TypeQualifier::Value;
            }
            record_inferred_struct_type_arg(pattern.loc, info, pattern.name, inferred_type, inferred);
            return true;
        }

        if (pattern.qualifier != actual.qualifier) return false;

        if (pattern.name == "Array") {
            if (actual.primitive != IrPrimitiveKind::Array ||
                pattern.array_size != actual.array_size ||
                pattern.args.size() != 1 ||
                actual.args.size() != 1) {
                return false;
            }
            return infer_struct_type_arg_from_value_type(pattern.args[0], actual.args[0], info, inferred);
        }

        if (pattern.name == "Tuple") {
            if (actual.primitive != IrPrimitiveKind::Tuple || pattern.args.size() != actual.args.size()) return false;
            for (std::size_t i = 0; i < pattern.args.size(); ++i) {
                if (!infer_struct_type_arg_from_value_type(pattern.args[i], actual.args[i], info, inferred)) return false;
            }
            return true;
        }

        if (pattern.name == "Vec" || pattern.name == "std::Vec" || pattern.name == "prelude::Vec") {
            if (actual.primitive != IrPrimitiveKind::Vector || pattern.args.size() != 1 || actual.args.size() != 1) {
                return false;
            }
            return infer_struct_type_arg_from_value_type(pattern.args[0], actual.args[0], info, inferred);
        }

        if (pattern.name == "fn") {
            if (actual.primitive != IrPrimitiveKind::Function ||
                pattern.array_size != actual.array_size ||
                pattern.args.size() != actual.args.size()) {
                return false;
            }
            for (std::size_t i = 0; i < pattern.args.size(); ++i) {
                if (!infer_struct_type_arg_from_value_type(pattern.args[i], actual.args[i], info, inferred)) return false;
            }
            return true;
        }

        std::string resolved_struct_name = resolve_struct_type_name(pattern.name);
        if (actual.primitive == IrPrimitiveKind::Struct && structs_.count(resolved_struct_name)) {
            if (actual.name != resolved_struct_name || pattern.args.size() != actual.args.size()) return false;
            for (std::size_t i = 0; i < pattern.args.size(); ++i) {
                if (!infer_struct_type_arg_from_value_type(pattern.args[i], actual.args[i], info, inferred)) return false;
            }
            return true;
        }

        std::string resolved_enum_name = resolve_enum_type_name(pattern.name);
        if (actual.primitive == IrPrimitiveKind::Enum && enums_.count(resolved_enum_name)) {
            if (actual.name != resolved_enum_name || pattern.args.size() != actual.args.size()) return false;
            for (std::size_t i = 0; i < pattern.args.size(); ++i) {
                if (!infer_struct_type_arg_from_value_type(pattern.args[i], actual.args[i], info, inferred)) return false;
            }
            return true;
        }

        IrPrimitiveKind primitive = IrPrimitiveKind::Unknown;
        std::string canonical;
        if (c_abi_type_alias(pattern.name, target_, primitive, canonical)) {
            return pattern.args.empty() && actual.primitive == primitive && actual.name == canonical;
        }

        if (!pattern.args.empty()) return false;
        TypeRef concrete = pattern;
        concrete.args.clear();
        IrType expected = resolve_executable_type(concrete);
        return same_type(expected, actual);
    }

    std::vector<IrType> infer_struct_type_args_from_values(
        SourceLocation loc,
        const StructInfo& info,
        const std::vector<IrExprPtr>& values
    ) {
        std::map<std::string, IrType> inferred;
        for (std::size_t i = 0; i < info.fields.size(); ++i) {
            if (i >= values.size()) break;
            infer_struct_type_arg_from_value_type(info.fields[i].type, values[i]->type, info, inferred);
        }

        std::vector<IrType> args;
        args.reserve(info.generic_names.size());
        for (const auto& generic_name : info.generic_names) {
            auto found = inferred.find(generic_name);
            if (found == inferred.end()) {
                fail(loc,
                     "cannot infer type argument '" + generic_name +
                         "' for struct '" + info.name + "'");
            }
            args.push_back(found->second);
        }
        return args;
    }

    IrType resolve_struct_literal_type(SourceLocation loc,
                                       const std::string& name,
                                       const std::vector<IrType>& explicit_type_args) {
        std::string struct_name = resolve_struct_type_name(name);
        auto struct_found = structs_.find(struct_name);
        if (struct_found == structs_.end()) {
            fail(loc, "unknown struct '" + name + "'");
        }
        const StructInfo& info = struct_found->second;
        require_struct_access(loc, info);
        if (info.deprecated) {
            warn_deprecated_use(loc, "struct", info.name, info.deprecated_message);
        }
        if (explicit_type_args.size() != info.generic_arity) {
            fail(loc,
                 "struct '" + info.name + "' expects " + std::to_string(info.generic_arity) +
                     " type argument" + (info.generic_arity == 1 ? "" : "s"));
        }

        IrType type;
        type.qualifier = TypeQualifier::Value;
        type.primitive = IrPrimitiveKind::Struct;
        type.name = info.name;
        type.loc = loc;

        std::map<std::string, IrType> substitutions;
        for (std::size_t i = 0; i < explicit_type_args.size(); ++i) {
            type.args.push_back(explicit_type_args[i]);
            substitutions.emplace(info.generic_names[i], explicit_type_args[i]);
        }

        std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
        for (const auto& item : substitutions) current_type_substitutions_.emplace(item.first, item.second);
        for (const auto& field : info.fields) {
            type.field_names.push_back(field.name);
            type.field_types.push_back(resolve_executable_type(field.type));
            type.field_mutable.push_back(field.mutable_field);
        }
        current_type_substitutions_ = std::move(previous_substitutions);
        return type;
    }

    IrExprPtr check_aggregate_access_operand(const Expr& expr) {
        if (expr.kind == ExprKind::Unary && expr.op == TokenKind::Star) {
            return check_pointer_deref_access_operand(expr);
        }
        if (expr.kind == ExprKind::Name) {
            LocalInfo* local_slot = find_local_slot(expr.name);
            if (local_slot && is_owner_type(local_slot->type)) {
                LocalInfo& local = *local_slot;
                if (auto error = local_unavailable_binding_error(expr.name, local)) fail(expr.loc, *error);
                require_can_read_borrow_path(expr.loc, expr.name, local, "");
                return make_local_lvalue_expr(expr.loc, expr.name, local.type);
            }
        }
        return check_expr(expr);
    }

    IrExprPtr check_tuple_index(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        TrackedAggregateAccess access;
        if (try_build_tracked_aggregate_access(expr, access)) {
            LocalInfo& local = local_slot_by_name(access.base_name);
            require_can_read_borrow_path(expr.loc, access.base_name, local, access.path);
            if (is_owner_type(access.type)) {
                mark_owned_field_moved(expr.loc, access.base_name, access.path);
            }
            activate_tracked_borrow_field_access(expr.loc, access, *access.expr);
            return std::move(access.expr);
        }

        IrExprPtr operand = check_aggregate_access_operand(*expr_operand(expr));
        if (operand->type.primitive != IrPrimitiveKind::Tuple &&
            operand->type.primitive != IrPrimitiveKind::Struct) {
            fail(expr.loc, "tuple index access requires tuple or tuple struct value, got " + type_name(operand->type));
        }
        const std::vector<IrType>& fields = aggregate_field_types(operand->type);
        if (expr.tuple_index >= fields.size()) {
            fail(expr.loc,
                 "tuple index " + std::to_string(expr.tuple_index) +
                 " is out of range for " + type_name(operand->type));
        }
        if (is_owner_type(operand->type) && is_owner_type(fields[static_cast<std::size_t>(expr.tuple_index)])) {
            if (!operand || operand->kind != IrExprKind::Local) {
                fail(expr.loc, "moving owned fields out of temporary aggregate values is not supported; bind the aggregate first");
            }
            mark_owned_field_moved(expr.loc, ir_expr_name(*operand), std::to_string(expr.tuple_index));
        }
        return make_tuple_index_expr(
            expr.loc,
            std::move(operand),
            static_cast<std::size_t>(expr.tuple_index)
        );
    }

    IrExprPtr check_index(const Expr& expr, IrExprPtr lowered) {
        TrackedAggregateAccess access;
        if (try_build_tracked_aggregate_access(expr, access)) {
            LocalInfo& local = local_slot_by_name(access.base_name);
            require_can_read_borrow_path(expr.loc, access.base_name, local, access.path);
            if (is_owner_type(access.type)) {
                mark_owned_field_moved(expr.loc, access.base_name, access.path);
            }
            activate_tracked_borrow_field_access(expr.loc, access, *access.expr);
            return std::move(access.expr);
        }

        IrExprPtr operand = check_aggregate_access_operand(*expr_operand(expr));
        IrExprPtr index = check_expr(*expr_right(expr));
        const bool slice_index = is_prelude_slice_type(operand->type);
        if (slice_index && is_prelude_range_type(index->type)) {
            return check_slice_range_index(expr, std::move(lowered), std::move(operand), std::move(index));
        }
        if (!is_value_integer_type(index->type)) {
            fail(expr.loc, "index expression must be an integer, got " + type_name(index->type));
        }
        if (!slice_index &&
            (operand->type.primitive != IrPrimitiveKind::Vector || operand->type.args.size() != 1) &&
            (operand->type.primitive != IrPrimitiveKind::Array || operand->type.args.size() != 1)) {
            fail(expr.loc, "index access requires an array, vector, or Slice value, got " + type_name(operand->type));
        }
        if (is_owner_type(operand->type)) {
            fail(expr.loc, "moving owning aggregate elements out of temporary values is not supported; bind the aggregate first");
        }

        if (slice_index) {
            if (index->kind == IrExprKind::Integer && index->int_negative) {
                fail(expr.loc, "Slice index must be non-negative");
            }
            const IrType element_type = operand->type.args[0];
            require_slice_element_materializable(expr.loc, element_type, "Slice indexing");
            return make_ir_index_expr(expr.loc, std::move(operand), std::move(index));
        }

        if (operand->kind == IrExprKind::Vector) {
            std::string label = operand->type.primitive == IrPrimitiveKind::Array ? "array" : "vector";
            if (index->kind != IrExprKind::Integer || index->int_negative) {
                fail(expr.loc,
                     label + " literal index must be a non-negative integer literal; bind the " +
                         label + " before dynamic indexing");
            }
            if (index->int_value >= operand->args.size()) {
                fail(expr.loc,
                     label + " literal index " + std::to_string(index->int_value) +
                     " is out of range for " + std::to_string(operand->args.size()) + " elements");
            }
            return std::move(operand->args[static_cast<std::size_t>(index->int_value)]);
        }

        if (operand->type.primitive == IrPrimitiveKind::Vector) {
            VectorKnownLength current_length =
                vector_known_length_from_source_expr(operand->type, *expr_operand(expr), *operand);
            if (index->kind == IrExprKind::Integer) {
                require_vector_index_in_known_bounds(
                    expr.loc,
                    StaticIntegerValue{index->int_value, index->int_negative},
                    current_length
                );
                if (index->int_value >= operand->type.array_size) {
                    fail(expr.loc,
                         "vector index " + std::to_string(index->int_value) +
                         " is out of range for " + std::to_string(operand->type.array_size) + " elements");
                }
            } else {
                require_vector_index_known_non_empty(expr.loc, current_length);
            }
        }

        if (operand->type.primitive == IrPrimitiveKind::Array) {
            if (index->kind == IrExprKind::Integer) {
                if (index->int_negative) {
                    fail(expr.loc, "array index must be non-negative");
                }
                if (index->int_value >= operand->type.array_size) {
                    fail(expr.loc,
                         "array index " + std::to_string(index->int_value) +
                         " is out of range for " + std::to_string(operand->type.array_size) + " elements");
                }
                return make_tuple_index_expr(
                    expr.loc,
                    std::move(operand),
                    static_cast<std::size_t>(index->int_value)
                );
            }

            return make_ir_index_expr(expr.loc, std::move(operand), std::move(index));
        }

        return make_ir_index_expr(expr.loc, std::move(operand), std::move(index));
    }

    IrExprPtr check_slice_range_index(const Expr& expr,
                                      IrExprPtr lowered,
                                      IrExprPtr operand,
                                      IrExprPtr range) {
        if (range->kind != IrExprKind::Tuple || range->args.size() != 2) {
            fail(expr.loc, "Slice range indexing currently expects direct range syntax like view[start..end]");
        }
        if (range->type.args.empty() || !is_value_integer_type(range->type.args[0])) {
            fail(expr.loc, "Slice range bounds must be integers");
        }
        const IrType element_type = operand->type.args[0];
        require_slice_element_materializable(expr.loc, element_type, "Slice range indexing");

        lowered->kind = IrExprKind::SliceRange;
        lowered->loc = expr.loc;
        lowered->type = operand->type;
        lowered->bool_value = range->type.name == "RangeInclusive";
        set_ir_expr_operand(*lowered, std::move(operand));
        set_ir_expr_left(*lowered, std::move(range->args[0]));
        set_ir_expr_right(*lowered, std::move(range->args[1]));
        return lowered;
    }

    IrExprPtr check_field_access(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        TrackedAggregateAccess access;
        if (try_build_tracked_aggregate_access(expr, access)) {
            LocalInfo& local = local_slot_by_name(access.base_name);
            require_can_read_borrow_path(expr.loc, access.base_name, local, access.path);
            if (is_owner_type(access.type)) {
                mark_owned_field_moved(expr.loc, access.base_name, access.path);
            }
            activate_tracked_borrow_field_access(expr.loc, access, *access.expr);
            return std::move(access.expr);
        }

        IrExprPtr operand = check_aggregate_access_operand(*expr_operand(expr));
        if (operand->type.primitive != IrPrimitiveKind::Struct) {
            fail(expr.loc, "field access requires a struct value, got " + type_name(operand->type));
        }
        std::size_t index = struct_field_index(expr.loc, operand->type, expr.name);
        if (is_owner_type(operand->type) && is_owner_type(operand->type.field_types[index])) {
            if (!operand || operand->kind != IrExprKind::Local) {
                fail(expr.loc, "moving owned fields out of temporary aggregate values is not supported; bind the aggregate first");
            }
            mark_owned_field_moved(expr.loc, ir_expr_name(*operand), std::to_string(index));
        }
        return make_tuple_index_expr(expr.loc, std::move(operand), index);
    }

    IrExprPtr check_tuple(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        (void)lowered;
        if (expr.args.size() == 1) fail(expr.loc, "single-element tuple literals are not supported");
        IrType tuple_type = primitive_type(IrPrimitiveKind::Tuple, "Tuple", expr.loc);
        std::vector<IrExprPtr> elements;
        elements.reserve(expr.args.size());
        bool use_expected_type = expected &&
            expected->qualifier == TypeQualifier::Value &&
            expected->primitive == IrPrimitiveKind::Tuple &&
            expected->args.size() == expr.args.size();
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            const auto& item = expr.args[i];
            const IrType* item_expected = tuple_literal_expected_element_type(expected, expr.args.size(), i);
            std::size_t item_borrow_mark = temporary_borrow_mark();
            IrExprPtr value = check_expr_maybe_expected(*item, item_expected);
            if (item_expected) {
                coerce_expr_to_expected(*value, *item_expected);
                require_assignable(item->loc, *item_expected, value->type);
            }
            require_plain_prelude_aggregate_element(expr.loc, value->type, "tuple");
            require_no_zone_pointer_escape(item->loc, *value, "tuple literal");
            prefix_temporary_borrow_targets(item_borrow_mark, std::to_string(i));
            tuple_type.args.push_back(value->type);
            elements.push_back(std::move(value));
        }
        if (use_expected_type) {
            tuple_type = *expected;
        }
        return make_ir_tuple_expr(expr.loc, std::move(tuple_type), std::move(elements));
    }

    IrExprPtr check_struct_literal(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        (void)lowered;
        std::string struct_name = resolve_struct_type_name(expr.name);
        auto struct_found = structs_.find(struct_name);
        if (struct_found == structs_.end()) {
            fail(expr.loc, "unknown struct '" + expr.name + "'");
        }
        const StructInfo& info = struct_found->second;
        require_struct_access(expr.loc, info);
        if (info.deprecated) {
            warn_deprecated_use(expr.loc, "struct", info.name, info.deprecated_message);
        }
        const ExprFieldNames& field_names = expr_field_names(expr);
        if (info.fields.size() != field_names.size()) {
            fail(expr.loc,
                 "struct literal for '" + info.name + "' expects " +
                     std::to_string(info.fields.size()) + " field" +
                     (info.fields.size() == 1 ? "" : "s"));
        }

        std::map<std::string, const Expr*> values;
        for (std::size_t i = 0; i < field_names.size(); ++i) {
            const std::string& field_name = field_names[i];
            if (!values.emplace(field_name, expr.args[i].get()).second) {
                fail(expr.loc, "duplicate field '" + field_name + "' in struct literal");
            }
        }

        std::vector<IrType> explicit_type_args;
        const ExprTypeArgs& ast_type_args = expr_type_args(expr);
        explicit_type_args.reserve(ast_type_args.size());
        for (const auto& type_arg : ast_type_args) {
            explicit_type_args.push_back(resolve_executable_type(type_arg));
        }

        bool has_struct_type = !explicit_type_args.empty() || info.generic_arity == 0;
        IrType struct_type;
        if (has_struct_type) {
            struct_type = resolve_struct_literal_type(expr.loc, expr.name, explicit_type_args);
        } else if (expected_type_matches_struct_literal(
                       expected,
                       info.name,
                       info.generic_arity,
                       info.fields.size())) {
            struct_type = *expected;
            has_struct_type = true;
        }

        std::vector<IrExprPtr> lowered_values;
        lowered_values.reserve(info.fields.size());
        for (std::size_t i = 0; i < info.fields.size(); ++i) {
            const auto& field = info.fields[i];
            auto found = values.find(field.name);
            if (found == values.end()) {
                fail(expr.loc, "missing field '" + field.name + "' in struct literal for '" + info.name + "'");
            }
            const IrType* field_expected = has_struct_type ? &struct_type.field_types[i] : nullptr;
            std::size_t field_borrow_mark = temporary_borrow_mark();
            lowered_values.push_back(check_expr_maybe_expected(*found->second, field_expected));
            prefix_temporary_borrow_targets(field_borrow_mark, std::to_string(i));
        }

        if (!has_struct_type) {
            explicit_type_args = infer_struct_type_args_from_values(expr.loc, info, lowered_values);
            struct_type = resolve_struct_literal_type(expr.loc, expr.name, explicit_type_args);
        }

        std::vector<IrExprPtr> elements;
        elements.reserve(struct_type.field_names.size());
        std::optional<std::size_t> std_zone_handle_source_field =
            std_vec_zone_handle_source_field_index(struct_type);
        if (!std_zone_handle_source_field) {
            std_zone_handle_source_field = std_box_zone_handle_source_field_index(struct_type);
        }
        if (!std_zone_handle_source_field) {
            std_zone_handle_source_field = std_string_zone_handle_source_field_index(struct_type);
        }
        for (std::size_t i = 0; i < struct_type.field_names.size(); ++i) {
            IrExprPtr value = std::move(lowered_values[i]);
            coerce_expr_to_expected(*value, struct_type.field_types[i]);
            require_assignable(expr.loc, struct_type.field_types[i], value->type);
            require_plain_prelude_aggregate_element(expr.loc, value->type, "struct");
            if (!std_zone_handle_source_field || i != *std_zone_handle_source_field) {
                require_no_zone_pointer_escape(value->loc, *value, "struct literal");
            }
            elements.push_back(std::move(value));
        }
        return make_ir_tuple_expr(expr.loc, std::move(struct_type), std::move(elements));
    }

    IrExprPtr check_struct_constructor_call(const Expr& expr,
                                            const StructInfo& info,
                                            IrExprPtr lowered,
                                            const IrType* expected = nullptr) {
        (void)lowered;
        if (!info.tuple_struct) {
            fail(expr.loc, "named struct '" + info.name + "' must be constructed with field literal syntax");
        }
        if (expr.args.size() != info.fields.size()) {
            fail(expr.loc,
                 "tuple struct '" + info.name + "' expects " + std::to_string(info.fields.size()) +
                     " value" + (info.fields.size() == 1 ? "" : "s"));
        }
        std::vector<IrType> explicit_type_args;
        const ExprTypeArgs& ast_type_args = expr_type_args(expr);
        explicit_type_args.reserve(ast_type_args.size());
        for (const auto& type_arg : ast_type_args) {
            explicit_type_args.push_back(resolve_executable_type(type_arg));
        }

        bool has_struct_type = !explicit_type_args.empty() || info.generic_arity == 0;
        IrType struct_type;
        if (has_struct_type) {
            struct_type = resolve_struct_literal_type(expr.loc, info.name, explicit_type_args);
        } else if (expected_type_matches_struct_literal(
                       expected,
                       info.name,
                       info.generic_arity,
                       info.fields.size())) {
            struct_type = *expected;
            has_struct_type = true;
        }

        std::vector<IrExprPtr> lowered_values;
        lowered_values.reserve(expr.args.size());
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            const IrType* field_expected = has_struct_type ? &struct_type.field_types[i] : nullptr;
            std::size_t field_borrow_mark = temporary_borrow_mark();
            lowered_values.push_back(check_expr_maybe_expected(*expr.args[i], field_expected));
            prefix_temporary_borrow_targets(field_borrow_mark, std::to_string(i));
        }
        if (!has_struct_type) {
            explicit_type_args = infer_struct_type_args_from_values(expr.loc, info, lowered_values);
            struct_type = resolve_struct_literal_type(expr.loc, info.name, explicit_type_args);
        }

        std::vector<IrExprPtr> elements;
        elements.reserve(expr.args.size());
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            IrExprPtr value = std::move(lowered_values[i]);
            coerce_expr_to_expected(*value, struct_type.field_types[i]);
            require_assignable(expr.loc, struct_type.field_types[i], value->type);
            require_plain_prelude_aggregate_element(expr.loc, value->type, "tuple struct");
            require_no_zone_pointer_escape(value->loc, *value, "tuple struct literal");
            elements.push_back(std::move(value));
        }
        return make_ir_tuple_expr(expr.loc, std::move(struct_type), std::move(elements));
    }

    IrExprPtr check_vector(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        if (expr.args.empty()) {
            fail(expr.loc, "empty [] literals need an explicit Vec[T] type or non-empty array elements");
        }
        if (expected &&
            expected->qualifier == TypeQualifier::Value &&
            expected->primitive == IrPrimitiveKind::Array &&
            expected->args.size() == 1 &&
            expr.args.size() != expected->array_size) {
            fail(expr.loc,
                 "array literal has " + std::to_string(expr.args.size()) +
                     " elements but type expects " + std::to_string(expected->array_size));
        }
        lowered->kind = IrExprKind::Vector;
        lowered->type = primitive_type(IrPrimitiveKind::Array, "Array", expr.loc);
        lowered->args.reserve(expr.args.size());
        const IrType* expected_element = vector_literal_expected_element_type(expected);
        const bool expected_vector =
            expected &&
            expected->qualifier == TypeQualifier::Value &&
            expected->primitive == IrPrimitiveKind::Vector &&
            expected->args.size() == 1;
        const bool expected_array =
            expected &&
            expected->qualifier == TypeQualifier::Value &&
            expected->primitive == IrPrimitiveKind::Array &&
            expected->args.size() == 1;
        const char* aggregate_name = expected_vector ? "vector" : "array";
        IrType element_type;
        bool has_element_type = expected_element != nullptr;
        if (expected_element) {
            element_type = *expected_element;
        }
        for (const auto& item : expr.args) {
            std::size_t item_borrow_mark = temporary_borrow_mark();
            IrExprPtr value = check_expr_maybe_expected(*item, expected_element);
            if (expected_element) {
                coerce_expr_to_expected(*value, *expected_element);
                require_assignable(item->loc, *expected_element, value->type);
            }
            require_plain_prelude_aggregate_element(expr.loc, value->type, aggregate_name);
            require_no_zone_pointer_escape(item->loc, *value, std::string(aggregate_name) + " literal");
            if (!has_element_type) {
                element_type = value->type;
                has_element_type = true;
            } else {
                coerce_expr_to_expected(*value, element_type);
                require_assignable(expr.loc, element_type, value->type);
            }
            prefix_temporary_borrow_targets(
                item_borrow_mark,
                std::to_string(lowered->args.size()));
            lowered->args.push_back(std::move(value));
        }
        if (expected_vector) {
            lowered->type = make_vector_storage_type(expr.loc, element_type, expr.args.size());
        } else if (expected_array) {
            lowered->type = array_storage_type(expr.loc, element_type, expr.args.size());
        } else {
            lowered->type = array_storage_type(expr.loc, element_type, expr.args.size());
        }
        return lowered;
    }

    IrExprPtr check_tuple_match_expr(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        IrExprPtr subject = std::move(ir_expr_match_value(*lowered));
        IrType subject_type = subject->type;
        const bool runtime_sequence_subject = is_runtime_sequence_pattern_subject(subject_type);
        if (!is_aggregate_type(subject_type) && !runtime_sequence_subject) {
            fail(expr.loc, "aggregate match requires a tuple, array, or struct value, got " + type_name(subject_type));
        }

        lowered = make_ir_block_expr(expr.loc);

        std::string subject_name = make_hidden_local(
            subject_type.primitive == IrPrimitiveKind::Struct ? "$match_struct" : "$match_tuple");
        declare_local(expr.loc, subject_name, subject_type, false);
        ir_expr_block_body(*lowered).push_back(
            make_ir_var_decl(expr.loc, subject_name, subject_type, std::move(subject), false));

        StateSnapshot branch_input = snapshot_states();
        StateSnapshot continuing_state;
        bool has_continuing_state = false;
        bool has_result = false;
        IrType result_type;
        std::optional<BorrowResultSource> result_borrow_source;
        ProductMatchCoverage coverage;
        std::vector<TupleCheckedExprArm> checked_arms;
        IrType explicit_result_expected;
        const IrType* result_expected = expected;
        if (expected) {
            explicit_result_expected = sized_control_flow_expected_type(*expected, match_arm_value_exprs(expr));
            result_expected = &explicit_result_expected;
        }

        for (const auto& arm : expr_match_arms(expr)) {
            if (coverage.has_irrefutable_arm) {
                fail(arm.pattern.loc, "unreachable match arm after irrefutable pattern");
            }

            std::vector<Pattern> alternatives = expand_or_pattern_alternatives(arm.pattern);
            std::set<std::string> reusable_names;
            if (pattern_contains_or(arm.pattern)) {
                reusable_names = require_same_or_pattern_bindings(arm.pattern.loc, alternatives, subject_type);
            }

            std::size_t alternative_index = 0;
            for (const auto& pattern : alternatives) {
                TupleCheckedExprArm lowered_arm;
                lowered_arm.loc = arm.loc;
                std::vector<IrStmtPtr> condition_prelude;
                lowered_arm.condition = lower_product_match_pattern_condition(
                    pattern,
                    subject_name,
                    subject_type,
                    condition_prelude
                );
                for (auto& statement : condition_prelude) {
                    ir_expr_block_body(*lowered).push_back(std::move(statement));
                }
                if (runtime_sequence_subject) {
                    if (!lowered_arm.condition) coverage.has_irrefutable_arm = true;
                } else {
                    note_product_match_coverage(pattern, subject_type, coverage);
                    if (!lowered_arm.condition) coverage.has_irrefutable_arm = true;
                }

                restore_states(branch_input);
                push_scope();
                local_scopes_.set_reusable_pattern_bindings(alternative_index == 0 ? std::set<std::string>{} : reusable_names);
                lower_product_match_pattern_bindings_from_local(pattern, subject_name, subject_type, lowered_arm.body);
                local_scopes_.clear_reusable_pattern_bindings();
                const IrType* arm_expected = result_expected ? result_expected : (has_result ? &result_type : nullptr);
                std::size_t borrow_mark = temporary_borrow_mark();
                IrExprPtr value = check_expr_maybe_expected(*arm.value, arm_expected);
                bool diverges = is_diverging_value_expr(*arm.value, *value);
                std::optional<BorrowResultSource> arm_borrow_source =
                    finish_control_flow_borrow_result(
                        arm.loc,
                        "match expression arm",
                        *value,
                        lowered_arm.body,
                        local_scopes_.size() - 1,
                        borrow_mark);
                if (!diverges && !arm_borrow_source && is_borrow_type(value->type)) {
                    pop_scope();
                    fail(arm.loc, "match expression arms cannot produce borrow values yet");
                }
                if (!diverges && is_void_value_type(value->type)) {
                    pop_scope();
                    fail(arm.loc, "match expression arms must produce a value");
                }
                if (diverges) {
                    release_temporary_borrows(borrow_mark);
                } else if (result_expected) {
                    coerce_expr_to_expected(*value, *result_expected);
                    require_assignable(arm.loc, *result_expected, value->type);
                    result_type = *result_expected;
                    has_result = true;
                } else if (!has_result) {
                    result_type = value->type;
                    has_result = true;
                } else {
                    coerce_expr_to_expected(*value, result_type);
                    require_assignable(arm.loc, result_type, value->type);
                }
                if (diverges) {
                    lowered_arm.value = std::move(value);
                    discard_scope();
                    checked_arms.push_back(std::move(lowered_arm));
                    ++alternative_index;
                    continue;
                }
                if (arm_borrow_source) {
                    require_same_borrow_result_source(
                        arm.loc,
                        "match expression",
                        result_borrow_source,
                        *arm_borrow_source);
                } else {
                    value = materialize_value_before_auto_destroy_cleanup(
                        arm.loc,
                        std::move(value),
                        lowered_arm.body,
                        local_scopes_.size() - 1,
                        "$match",
                        "match expression result"
                    );
                }
                lowered_arm.value = std::move(value);
                pop_scope();

                StateSnapshot branch_state = snapshot_states();
                if (!has_continuing_state) {
                    continuing_state = branch_state;
                    has_continuing_state = true;
                } else {
                    require_same_states(arm.loc, continuing_state, branch_state,
                                        "has incompatible ownership states after match expression arms");
                    merge_zone_generations_into(continuing_state, branch_state);
                }

                checked_arms.push_back(std::move(lowered_arm));
                ++alternative_index;
            }
        }

        if (runtime_sequence_subject) {
            if (!coverage.has_irrefutable_arm) {
                fail(expr.loc,
                     runtime_sequence_pattern_subject_name(subject_type) +
                         " match must include _ or [..] fallback");
            }
        } else {
            require_tuple_match_exhaustive(expr.loc, subject_type, coverage);
        }
        if (!has_result) {
            fail(expr.loc, "match expression must have at least one reachable value arm");
        }
        restore_states(continuing_state);
        lowered->type = result_type;
        set_ir_expr_block_value(
            *lowered,
            build_tuple_match_if_expr_chain(
                checked_arms,
                result_type,
                [this](SourceLocation loc, const IrType& type) {
                    return make_unreachable_match_fallback_expr(loc, type);
                }
            ));
        if (result_borrow_source) {
            activate_control_flow_borrow_result(expr.loc, *lowered, *result_borrow_source);
        }
        return lowered;
    }

    IrExprPtr check_match_expr(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        if (expr_match_arms(expr).empty()) fail(expr.loc, "match must have at least one arm");

        IrExprPtr match_value = check_expr(*expr_match_value(expr));
        if (is_borrow_type(match_value->type)) {
            fail(expr.loc, "borrow expression result must be passed directly to a call");
        }
        if (!is_value_enum_type(match_value->type)) {
            if (is_value_integer_type(match_value->type) ||
                (match_value->type.qualifier == TypeQualifier::Value &&
                 match_value->type.primitive == IrPrimitiveKind::Bool)) {
                lowered = make_ir_match_expr(expr.loc, std::move(match_value));
                return check_scalar_match_expr(expr, std::move(lowered), expected);
            }
            if (is_aggregate_type(match_value->type) ||
                is_runtime_sequence_pattern_subject(match_value->type)) {
                lowered = make_ir_match_expr(expr.loc, std::move(match_value));
                return check_tuple_match_expr(expr, std::move(lowered), expected);
            }
            fail(expr.loc, "match value must be an enum, integer, bool, tuple, array, or struct, got " + type_name(match_value->type));
        }

        auto enum_found = enums_.find(match_value->type.name);
        if (enum_found == enums_.end()) {
            fail(expr.loc, "unknown enum type '" + match_value->type.name + "'");
        }
        const EnumInfo& enum_info = enum_found->second;
        IrType enum_value_type = match_value->type;
        lowered = make_ir_match_expr(expr.loc, std::move(match_value));

        StateSnapshot branch_input = snapshot_states();
        StateSnapshot continuing_state;
        bool has_continuing_state = false;
        EnumMatchCoverage coverage;
        bool has_result = false;
        IrType result_type;
        std::optional<BorrowResultSource> result_borrow_source;
        IrType explicit_result_expected;
        const IrType* result_expected = expected;
        if (expected) {
            explicit_result_expected = sized_control_flow_expected_type(*expected, match_arm_value_exprs(expr));
            result_expected = &explicit_result_expected;
        }

        for (const auto& arm : expr_match_arms(expr)) {
            std::set<std::string> reusable_names;
            if (pattern_contains_or(arm.pattern)) {
                std::vector<Pattern> alternatives = expand_or_pattern_alternatives(arm.pattern);
                reusable_names = require_same_or_pattern_bindings(
                    arm.pattern.loc,
                    alternatives,
                    enum_match_value_type(arm.pattern.loc, enum_value_type)
                );
            }
            std::vector<IrMatchArm> lowered_patterns = lower_match_arm_patterns(
                arm.pattern,
                enum_info,
                enum_value_type,
                coverage);
            std::size_t alternative_index = 0;
            for (auto& pattern : lowered_patterns) {
                IrMatchExprArm lowered_arm = make_match_expr_arm(std::move(pattern));

                restore_states(branch_input);
                push_scope();
                local_scopes_.set_reusable_pattern_bindings(alternative_index == 0 ? std::set<std::string>{} : reusable_names);
                declare_match_arm_bindings(lowered_arm);
                local_scopes_.clear_reusable_pattern_bindings();

                const IrType* arm_expected = result_expected ? result_expected : (has_result ? &result_type : nullptr);
                std::size_t borrow_mark = temporary_borrow_mark();
                IrExprPtr value = check_expr_maybe_expected(*arm.value, arm_expected);
                bool diverges = is_diverging_value_expr(*arm.value, *value);
                std::optional<BorrowResultSource> arm_borrow_source =
                    finish_control_flow_borrow_result(
                        arm.loc,
                        "match expression arm",
                        *value,
                        lowered_arm.body,
                        local_scopes_.size() - 1,
                        borrow_mark);
                if (!diverges && !arm_borrow_source && is_borrow_type(value->type)) {
                    fail(arm.loc, "match expression arms cannot produce borrow values yet");
                }
                if (!diverges && is_void_value_type(value->type)) {
                    fail(arm.loc, "match expression arms must produce a value");
                }
                if (diverges) {
                    release_temporary_borrows(borrow_mark);
                } else if (result_expected) {
                    coerce_expr_to_expected(*value, *result_expected);
                    require_assignable(arm.loc, *result_expected, value->type);
                    result_type = *result_expected;
                    has_result = true;
                } else if (!has_result) {
                    result_type = value->type;
                    has_result = true;
                } else {
                    coerce_expr_to_expected(*value, result_type);
                    require_assignable(arm.loc, result_type, value->type);
                }
                if (diverges) {
                    lowered_arm.value = std::move(value);
                    discard_scope();
                    ir_expr_match_arms(*lowered).push_back(std::move(lowered_arm));
                    ++alternative_index;
                    continue;
                }
                if (arm_borrow_source) {
                    require_same_borrow_result_source(
                        arm.loc,
                        "match expression",
                        result_borrow_source,
                        *arm_borrow_source);
                } else {
                    value = materialize_value_before_auto_destroy_cleanup(
                        arm.loc,
                        std::move(value),
                        lowered_arm.body,
                        local_scopes_.size() - 1,
                        "$match",
                        "match expression result"
                    );
                }
                lowered_arm.value = std::move(value);
                pop_scope();

                StateSnapshot branch_state = snapshot_states();
                if (!has_continuing_state) {
                    continuing_state = branch_state;
                    has_continuing_state = true;
                } else {
                    require_same_states(arm.loc, continuing_state, branch_state,
                                        "has incompatible ownership states after match expression arms");
                    merge_zone_generations_into(continuing_state, branch_state);
                }

                ir_expr_match_arms(*lowered).push_back(std::move(lowered_arm));
                ++alternative_index;
            }
        }

        require_match_exhaustive(expr.loc, enum_info, coverage);
        if (!has_result) {
            fail(expr.loc, "match expression must have at least one reachable value arm");
        }
        restore_states(continuing_state);
        lowered->type = result_type;
        if (result_borrow_source) {
            activate_control_flow_borrow_result(expr.loc, *lowered, *result_borrow_source);
        }
        return lowered;
    }

    IrExprPtr check_scalar_match_expr(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        StateSnapshot branch_input = snapshot_states();
        StateSnapshot continuing_state;
        bool has_continuing_state = false;
        ScalarMatchCoverage coverage;
        bool has_result = false;
        IrType result_type;
        std::optional<BorrowResultSource> result_borrow_source;
        IrType explicit_result_expected;
        const IrType* result_expected = expected;
        if (expected) {
            explicit_result_expected = sized_control_flow_expected_type(*expected, match_arm_value_exprs(expr));
            result_expected = &explicit_result_expected;
        }

        for (const auto& arm : expr_match_arms(expr)) {
            std::set<std::string> reusable_names;
            if (pattern_contains_or(arm.pattern)) {
                std::vector<Pattern> alternatives = expand_or_pattern_alternatives(arm.pattern);
                reusable_names = require_same_or_pattern_bindings(
                    arm.pattern.loc,
                    alternatives,
                    ir_expr_match_value(*lowered)->type
                );
            }
            std::vector<IrMatchArm> lowered_patterns = lower_scalar_match_arm_patterns(
                arm.pattern, ir_expr_match_value(*lowered)->type, coverage);
            std::size_t alternative_index = 0;
            for (auto& pattern : lowered_patterns) {
                IrMatchExprArm lowered_arm = make_match_expr_arm(std::move(pattern));

                restore_states(branch_input);
                push_scope();
                local_scopes_.set_reusable_pattern_bindings(alternative_index == 0 ? std::set<std::string>{} : reusable_names);
                declare_match_arm_bindings(lowered_arm);
                local_scopes_.clear_reusable_pattern_bindings();
                const IrType* arm_expected = result_expected ? result_expected : (has_result ? &result_type : nullptr);
                std::size_t borrow_mark = temporary_borrow_mark();
                IrExprPtr value = check_expr_maybe_expected(*arm.value, arm_expected);
                bool diverges = is_diverging_value_expr(*arm.value, *value);
                std::optional<BorrowResultSource> arm_borrow_source =
                    finish_control_flow_borrow_result(
                        arm.loc,
                        "match expression arm",
                        *value,
                        lowered_arm.body,
                        local_scopes_.size() - 1,
                        borrow_mark);
                if (!diverges && !arm_borrow_source && is_borrow_type(value->type)) {
                    fail(arm.loc, "match expression arms cannot produce borrow values yet");
                }
                if (!diverges && is_void_value_type(value->type)) {
                    fail(arm.loc, "match expression arms must produce a value");
                }
                if (diverges) {
                    release_temporary_borrows(borrow_mark);
                } else if (result_expected) {
                    coerce_expr_to_expected(*value, *result_expected);
                    require_assignable(arm.loc, *result_expected, value->type);
                    result_type = *result_expected;
                    has_result = true;
                } else if (!has_result) {
                    result_type = value->type;
                    has_result = true;
                } else {
                    coerce_expr_to_expected(*value, result_type);
                    require_assignable(arm.loc, result_type, value->type);
                }
                if (diverges) {
                    lowered_arm.value = std::move(value);
                    discard_scope();
                    ir_expr_match_arms(*lowered).push_back(std::move(lowered_arm));
                    ++alternative_index;
                    continue;
                }
                if (arm_borrow_source) {
                    require_same_borrow_result_source(
                        arm.loc,
                        "match expression",
                        result_borrow_source,
                        *arm_borrow_source);
                } else {
                    value = materialize_value_before_auto_destroy_cleanup(
                        arm.loc,
                        std::move(value),
                        lowered_arm.body,
                        local_scopes_.size() - 1,
                        "$match",
                        "match expression result"
                    );
                }
                lowered_arm.value = std::move(value);
                pop_scope();

                StateSnapshot branch_state = snapshot_states();
                if (!has_continuing_state) {
                    continuing_state = branch_state;
                    has_continuing_state = true;
                } else {
                    require_same_states(arm.loc, continuing_state, branch_state,
                                        "has incompatible ownership states after match expression arms");
                    merge_zone_generations_into(continuing_state, branch_state);
                }

                ir_expr_match_arms(*lowered).push_back(std::move(lowered_arm));
                ++alternative_index;
            }
        }

        require_scalar_match_exhaustive(expr.loc, ir_expr_match_value(*lowered)->type, coverage);
        if (!has_result) {
            fail(expr.loc, "match expression must have at least one reachable value arm");
        }
        restore_states(continuing_state);
        lowered->type = result_type;
        if (result_borrow_source) {
            activate_control_flow_borrow_result(expr.loc, *lowered, *result_borrow_source);
        }
        return lowered;
    }

    IrExprPtr check_if_expr(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        if (expr_if_has_condition_pattern(expr)) return check_if_let_expr(expr, std::move(lowered), expected);
        (void)lowered;

        IrExprPtr condition = check_expr(*expr_if_condition(expr));
        coerce_condition_to_bool(expr.loc, condition);

        StateSnapshot branch_input = snapshot_states();
        CheckedExprBlock then_arm = check_value_block(
            expr.loc,
            "if expression arm",
            expr_if_then_body(expr),
            *expr_if_then_value(expr),
            expected);
        std::optional<IrType> probed_result_type;
        if (then_arm.diverges && !expected) {
            restore_states(branch_input);
            CheckedExprBlock else_probe = check_value_block(
                expr.loc,
                "if expression arm",
                expr_if_else_body(expr),
                *expr_if_else_value(expr),
                nullptr);
            if (else_probe.diverges) {
                fail(expr.loc, "if expression must have at least one reachable value arm");
            }
            probed_result_type = else_probe.value->type;
            restore_states(branch_input);
            then_arm = check_value_block(
                expr.loc,
                "if expression arm",
                expr_if_then_body(expr),
                *expr_if_then_value(expr),
                &*probed_result_type);
        }
        IrType result_type = expected
            ? *expected
            : (probed_result_type ? *probed_result_type : then_arm.value->type);
        bool unsized_vector_expected = is_unsized_vector_storage_type(expected);
        if (then_arm.diverges) {
            result_type = expected ? *expected : result_type;
        } else if (unsized_vector_expected) {
            widen_vector_result_storage(result_type, *then_arm.value);
        } else if (expected) {
            coerce_expr_to_expected(*then_arm.value, result_type);
            require_assignable(expr.loc, result_type, then_arm.value->type);
        }

        restore_states(branch_input);
        const IrType* else_expected = unsized_vector_expected ? expected : &result_type;
        CheckedExprBlock else_arm = check_value_block(
            expr.loc,
            "if expression arm",
            expr_if_else_body(expr),
            *expr_if_else_value(expr),
            else_expected);
        if (then_arm.diverges && else_arm.diverges) {
            fail(expr.loc, "if expression must have at least one reachable value arm");
        }
        if (then_arm.diverges && !expected) {
            result_type = else_arm.value->type;
        }
        if (!else_arm.diverges && unsized_vector_expected) {
            widen_vector_result_storage(result_type, *else_arm.value);
        }
        if (!then_arm.diverges) {
            coerce_expr_to_expected(*then_arm.value, result_type);
            require_assignable(expr.loc, result_type, then_arm.value->type);
        }
        if (!else_arm.diverges) {
            coerce_expr_to_expected(*else_arm.value, result_type);
            require_assignable(expr.loc, result_type, else_arm.value->type);
        }

        if (then_arm.diverges) {
            restore_states(else_arm.state);
        } else if (else_arm.diverges) {
            restore_states(then_arm.state);
        } else {
            require_same_states(expr.loc, then_arm.state, else_arm.state,
                                "has incompatible ownership states after if expression branches");
            restore_merged_states(then_arm.state, else_arm.state);
        }

        std::optional<BorrowResultSource> borrow_source = then_arm.borrow_source;
        if (borrow_source || else_arm.borrow_source) {
            if ((!borrow_source && !then_arm.diverges) || (!else_arm.borrow_source && !else_arm.diverges)) {
                fail(expr.loc, "if expression arms must borrow the same source path and mode in every result arm");
            }
            if (else_arm.borrow_source) {
                require_same_borrow_result_source(
                    expr.loc,
                    "if expression",
                    borrow_source,
                    *else_arm.borrow_source);
            }
        }

        IrExprPtr result = make_ir_if_expr(
            expr.loc,
            result_type,
            std::move(condition),
            std::move(then_arm.statements),
            std::move(then_arm.value),
            std::move(else_arm.statements),
            std::move(else_arm.value)
        );
        if (borrow_source) {
            activate_control_flow_borrow_result(expr.loc, *result, *borrow_source);
        }
        return result;
    }

    IrExprPtr check_if_let_expr(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        IrExprPtr match_value = check_expr(*expr_if_condition(expr));
        if (is_aggregate_type(match_value->type) ||
            is_runtime_sequence_pattern_subject(match_value->type)) {
            return check_aggregate_if_let_expr(expr, std::move(lowered), std::move(match_value), expected);
        }

        lowered = make_ir_block_expr(expr.loc);
        const Pattern& condition_pattern = expanded_pattern(*expr_if_condition_pattern(expr));
        IrType enum_value_type = match_value->type;
        const EnumInfo& enum_info = require_enum_match_value(expr.loc, *match_value);
        std::vector<IrMatchArm> then_arms = lower_if_let_enum_pattern_arms(
            condition_pattern,
            enum_info,
            enum_value_type
        );

        std::string subject_name = make_hidden_local("$iflet_enum");
        declare_local(expr.loc, subject_name, enum_value_type, false);
        ir_expr_block_body(*lowered).push_back(make_ir_var_decl(
            expr.loc,
            subject_name,
            enum_value_type,
            std::move(match_value),
            false
        ));

        IrType matched_type = bool_type(expr.loc);
        std::string matched_name = make_hidden_local("$iflet_matched");
        declare_local(expr.loc, matched_name, matched_type, true);
        ir_expr_block_body(*lowered).push_back(make_ir_var_decl(
            expr.loc,
            matched_name,
            matched_type,
            make_bool_literal_expr(expr.loc, false),
            true
        ));

        std::vector<const Expr*> result_values{
            expr_if_then_value(expr).get(),
            expr_if_else_value(expr).get()};
        IrType explicit_result_expected;
        const IrType* result_expected = expected;
        if (expected) {
            explicit_result_expected = sized_control_flow_expected_type(*expected, result_values);
            result_expected = &explicit_result_expected;
        }

        StateSnapshot branch_input = snapshot_states();
        push_scope();
        declare_match_arm_bindings(then_arms.front());
        CheckedStatements then_statements = check_statements(expr_if_then_body(expr), false);
        if (then_statements.flow == Flow::Returns) {
            discard_scope();
            fail(expr.loc, "if-let expression arm must reach its final value");
        }
        std::size_t then_borrow_mark = temporary_borrow_mark();
        IrExprPtr then_value = check_expr_maybe_expected(*expr_if_then_value(expr), result_expected);
        std::optional<BorrowResultSource> then_borrow_source =
            finish_control_flow_borrow_result(
                expr.loc,
                "if-let expression arm",
                *then_value,
                then_statements.statements,
                local_scopes_.size() - 1,
                then_borrow_mark);
        if (!then_borrow_source && contains_borrow_type(then_value->type)) {
            pop_scope();
            fail(expr.loc, "if-let expression arm cannot produce borrow values yet");
        }
        if (is_void_value_type(then_value->type)) {
            pop_scope();
            fail(expr.loc, "if-let expression arm must produce a value");
        }
        IrType result_type = result_expected ? *result_expected : then_value->type;
        if (result_expected) {
            coerce_expr_to_expected(*then_value, result_type);
            require_assignable(expr.loc, result_type, then_value->type);
        }
        if (!then_borrow_source) {
            then_value = materialize_value_before_auto_destroy_cleanup(
                expr.loc,
                std::move(then_value),
                then_statements.statements,
                local_scopes_.size() - 1,
                "$block",
                "if-let expression arm result"
            );
        }
        pop_scope();
        StateSnapshot then_state = snapshot_states();

        restore_states(branch_input);
        CheckedExprBlock else_checked = check_value_block(
            expr.loc,
            "if-let expression arm",
            expr_if_else_body(expr),
            *expr_if_else_value(expr),
            &result_type);
        coerce_expr_to_expected(*else_checked.value, result_type);
        require_assignable(expr.loc, result_type, else_checked.value->type);

        require_same_states(expr.loc, then_state, else_checked.state,
                            "has incompatible ownership states after if-let expression branches");
        restore_merged_states(then_state, else_checked.state);

        std::optional<BorrowResultSource> borrow_source = then_borrow_source;
        if (borrow_source || else_checked.borrow_source) {
            if (!borrow_source || !else_checked.borrow_source) {
                fail(expr.loc, "if-let expression arms must borrow the same source path and mode in every result arm");
            }
            require_same_borrow_result_source(
                expr.loc,
                "if-let expression",
                borrow_source,
                *else_checked.borrow_source);
        }

        auto pattern_match = std::make_unique<IrStmt>();
        pattern_match->kind = IrStmtKind::Match;
        pattern_match->loc = expr.loc;
        pattern_match->match_value = make_local_lvalue_expr(expr.loc, subject_name, enum_value_type);
        IrStmtMatchArms& match_arms = ensure_ir_stmt_match_arms(*pattern_match);
        for (auto& arm : then_arms) {
            arm.body.push_back(make_bool_assignment_stmt(arm.loc, matched_name, true));
            match_arms.push_back(std::move(arm));
        }
        IrMatchArm no_match;
        no_match.loc = expr.loc;
        no_match.wildcard = true;
        match_arms.push_back(std::move(no_match));
        ir_expr_block_body(*lowered).push_back(std::move(pattern_match));

        IrExprPtr if_expr = make_ir_if_expr(
            expr.loc,
            result_type,
            make_local_lvalue_expr(expr.loc, matched_name, matched_type),
            std::move(then_statements.statements),
            std::move(then_value),
            std::move(else_checked.statements),
            std::move(else_checked.value)
        );

        lowered->type = result_type;
        set_ir_expr_block_value(*lowered, std::move(if_expr));
        if (borrow_source) {
            activate_control_flow_borrow_result(expr.loc, *lowered, *borrow_source);
        }
        return lowered;
    }

    IrExprPtr check_aggregate_if_let_expr(
        const Expr& expr,
        IrExprPtr lowered,
        IrExprPtr subject,
        const IrType* expected = nullptr
    ) {
        IrType subject_type = subject->type;
        const Pattern& condition_pattern = expanded_pattern(*expr_if_condition_pattern(expr));
        lowered = make_ir_block_expr(expr.loc);

        std::string subject_name = make_hidden_local(
            subject_type.primitive == IrPrimitiveKind::Struct ? "$iflet_struct" : "$iflet_tuple");
        declare_local(expr.loc, subject_name, subject_type, false);
        ir_expr_block_body(*lowered).push_back(
            make_ir_var_decl(expr.loc, subject_name, subject_type, std::move(subject), false));

        StateSnapshot branch_input = snapshot_states();
        StateSnapshot continuing_state;
        bool has_continuing_state = false;
        bool has_result = false;
        bool has_irrefutable_alternative = false;
        IrType result_type;
        std::optional<BorrowResultSource> result_borrow_source;
        std::vector<TupleCheckedExprArm> checked_arms;
        std::vector<const Expr*> result_values{
            expr_if_then_value(expr).get(),
            expr_if_else_value(expr).get()};
        IrType explicit_result_expected;
        const IrType* result_expected = expected;
        if (expected) {
            explicit_result_expected = sized_control_flow_expected_type(*expected, result_values);
            result_expected = &explicit_result_expected;
        }

        std::vector<Pattern> alternatives = expand_or_pattern_alternatives(condition_pattern);
        std::set<std::string> reusable_names;
        if (pattern_contains_or(condition_pattern)) {
            reusable_names = require_same_or_pattern_bindings(condition_pattern.loc, alternatives, subject_type);
        }

        for (std::size_t i = 0; i < alternatives.size(); ++i) {
            const Pattern& pattern = alternatives[i];
            TupleCheckedExprArm lowered_arm;
            lowered_arm.loc = pattern.loc;

            std::vector<IrStmtPtr> condition_prelude;
            lowered_arm.condition = lower_product_match_pattern_condition(
                pattern,
                subject_name,
                subject_type,
                condition_prelude
            );
            if (!lowered_arm.condition) {
                has_irrefutable_alternative = true;
                fail(condition_pattern.loc, "irrefutable if-let aggregate pattern cannot have else");
            }
            for (auto& statement : condition_prelude) {
                ir_expr_block_body(*lowered).push_back(std::move(statement));
            }

            restore_states(branch_input);
            push_scope();
            local_scopes_.set_reusable_pattern_bindings(i == 0 ? std::set<std::string>{} : reusable_names);
            lower_product_match_pattern_bindings_from_local(pattern, subject_name, subject_type, lowered_arm.body);
            local_scopes_.clear_reusable_pattern_bindings();
            CheckedStatements then_checked = check_statements(expr_if_then_body(expr), false);
            if (then_checked.flow == Flow::Returns) {
                discard_scope();
                fail(expr.loc, "if-let expression arm must reach its final value");
            }
            for (auto& statement : then_checked.statements) {
                lowered_arm.body.push_back(std::move(statement));
            }

            const IrType* arm_expected = result_expected ? result_expected : (has_result ? &result_type : nullptr);
            std::size_t borrow_mark = temporary_borrow_mark();
            IrExprPtr then_value = check_expr_maybe_expected(*expr_if_then_value(expr), arm_expected);
            std::optional<BorrowResultSource> then_borrow_source =
                finish_control_flow_borrow_result(
                    expr.loc,
                    "if-let expression arm",
                    *then_value,
                    lowered_arm.body,
                    local_scopes_.size() - 1,
                    borrow_mark);
            if (!then_borrow_source && is_borrow_type(then_value->type)) {
                pop_scope();
                fail(expr.loc, "if-let expression arm cannot produce borrow values yet");
            }
            if (is_void_value_type(then_value->type)) {
                pop_scope();
                fail(expr.loc, "if-let expression arm must produce a value");
            }
            if (result_expected) {
                coerce_expr_to_expected(*then_value, *result_expected);
                require_assignable(expr.loc, *result_expected, then_value->type);
                result_type = *result_expected;
                has_result = true;
            } else if (!has_result) {
                result_type = then_value->type;
                has_result = true;
            } else {
                coerce_expr_to_expected(*then_value, result_type);
                require_assignable(expr.loc, result_type, then_value->type);
            }
            if (then_borrow_source) {
                require_same_borrow_result_source(
                    expr.loc,
                    "if-let expression",
                    result_borrow_source,
                    *then_borrow_source);
            } else {
                then_value = materialize_value_before_auto_destroy_cleanup(
                    expr.loc,
                    std::move(then_value),
                    lowered_arm.body,
                    local_scopes_.size() - 1,
                    "$iflet",
                    "if-let expression result"
                );
            }
            lowered_arm.value = std::move(then_value);
            pop_scope();

            StateSnapshot then_state = snapshot_states();
            if (!has_continuing_state) {
                continuing_state = then_state;
                has_continuing_state = true;
            } else {
                require_same_states(
                    expr.loc,
                    continuing_state,
                    then_state,
                    "has incompatible ownership states after if-let pattern alternatives"
                );
                merge_zone_generations_into(continuing_state, then_state);
            }

            checked_arms.push_back(std::move(lowered_arm));
        }

        if (has_irrefutable_alternative) {
            fail(condition_pattern.loc, "irrefutable if-let aggregate pattern cannot have else");
        }

        restore_states(branch_input);
        CheckedExprBlock else_checked = check_value_block(
            expr.loc,
            "if-let expression arm",
            expr_if_else_body(expr),
            *expr_if_else_value(expr),
            &result_type);
        coerce_expr_to_expected(*else_checked.value, result_type);
        require_assignable(expr.loc, result_type, else_checked.value->type);
        if (result_borrow_source || else_checked.borrow_source) {
            if (!result_borrow_source || !else_checked.borrow_source) {
                fail(expr.loc, "if-let expression arms must borrow the same source path and mode in every result arm");
            }
            require_same_borrow_result_source(
                expr.loc,
                "if-let expression",
                result_borrow_source,
                *else_checked.borrow_source);
        }

        require_same_states(expr.loc, continuing_state, else_checked.state,
                            "has incompatible ownership states after if-let expression branches");
        merge_zone_generations_into(continuing_state, else_checked.state);
        restore_states(continuing_state);

        TupleCheckedExprArm fallback;
        fallback.loc = expr.loc;
        fallback.body = std::move(else_checked.statements);
        fallback.value = std::move(else_checked.value);
        checked_arms.push_back(std::move(fallback));

        lowered->type = result_type;
        set_ir_expr_block_value(
            *lowered,
            build_tuple_match_if_expr_chain(
                checked_arms,
                result_type,
                [this](SourceLocation loc, const IrType& type) {
                    return make_unreachable_match_fallback_expr(loc, type);
                }
            ));
        if (result_borrow_source) {
            activate_control_flow_borrow_result(expr.loc, *lowered, *result_borrow_source);
        }
        return lowered;
    }

    IrExprPtr check_block_expr(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        (void)lowered;
        const std::string& label = expr_block_label(expr);
        const ExprPtr& value = expr_block_value(expr);
        if (!label.empty()) {
            CheckedExprBlock block = check_labeled_value_block(expr, expected);
            IrType result_type = block.value->type;
            IrExprPtr result = make_ir_block_expr(
                expr.loc,
                label,
                std::move(result_type),
                std::move(block.statements),
                std::move(block.value)
            );
            if (block.borrow_source) {
                activate_control_flow_borrow_result(expr.loc, *result, *block.borrow_source);
            }
            return result;
        }

        CheckedExprBlock block =
            check_value_block(expr.loc, "block expression", expr_block_body(expr), *value, expected);
        IrType result_type = block.value->type;
        IrExprPtr result = make_ir_block_expr(
            expr.loc,
            {},
            std::move(result_type),
            std::move(block.statements),
            std::move(block.value)
        );
        if (block.borrow_source) {
            activate_control_flow_borrow_result(expr.loc, *result, *block.borrow_source);
        }
        return result;
    }

    CheckedExprBlock check_labeled_value_block(const Expr& expr, const IrType* expected = nullptr) {
        const std::string& label = expr_block_label(expr);
        for (const auto& active : loops_) {
            if (active.label == label) {
                fail(expr.loc, "duplicate active label '" + label + "'");
            }
        }

        push_scope();
        LoopInfo block;
        block.label = label;
        block.is_loop = false;
        block.supports_break_values = true;
        block.scope_depth = local_scopes_.size() - 1;
        if (expected) {
            block.has_break_result_type = true;
            block.break_result_type = *expected;
        }
        loops_.push_back(block);

        CheckedStatements checked = check_statements(expr_block_body(expr), false);
        if (checked.flow == Flow::Returns) {
            loops_.pop_back();
            discard_scope();
            fail(expr.loc, "block expression must reach its final value or a typed break");
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr_maybe_expected(*expr_block_value(expr), expected);
        std::optional<BorrowResultSource> borrow_source =
            finish_control_flow_borrow_result(
                expr.loc,
                "block expression",
                *value,
                checked.statements,
                local_scopes_.size() - 1,
                borrow_mark);
        if (!borrow_source && is_borrow_type(value->type)) {
            loops_.pop_back();
            pop_scope();
            fail(expr.loc, "block expression cannot produce borrow values yet");
        }
        if (is_void_value_type(value->type)) {
            loops_.pop_back();
            pop_scope();
            fail(expr.loc, "block expression must produce a value");
        }

        LoopInfo block_state = loops_.back();
        loops_.pop_back();
        if (block_state.has_break_result_type) {
            widen_vector_result_storage(block_state.break_result_type, *value);
            coerce_expr_to_expected(*value, block_state.break_result_type);
            require_assignable(expr.loc, block_state.break_result_type, value->type);
            coerce_labeled_break_values(checked.statements, label, block_state.break_result_type);
        }
        if (borrow_source) {
            if (block_state.break_borrow_source) {
                require_same_borrow_result_source(
                    expr.loc,
                    "labeled block expression",
                    borrow_source,
                    *block_state.break_borrow_source);
            }
        } else if (block_state.break_borrow_source) {
            fail(expr.loc, "labeled block expression must borrow the same source path and mode in every result arm");
        }
        if (!borrow_source) {
            value = materialize_value_before_auto_destroy_cleanup(
                expr.loc,
                std::move(value),
                checked.statements,
                local_scopes_.size() - 1,
                "$block",
                "block expression result"
            );
        }
        pop_scope();
        StateSnapshot state = snapshot_states();
        for (const auto& break_state : block_state.break_state_snapshots) {
            require_same_states(
                expr.loc,
                state,
                break_state,
                "has incompatible ownership states after labeled block exits"
            );
            merge_existing_zone_generations_into(state, break_state);
        }
        return CheckedExprBlock{
            std::move(checked.statements),
            std::move(value),
            std::move(state),
            std::move(borrow_source)
        };
    }

    CheckedExprBlock check_value_block_with_match_payload(
        SourceLocation loc,
        const std::string& context,
        const IrMatchExprArm& arm,
        const std::vector<StmtPtr>& body,
        const Expr& value_expr,
        const IrType* expected = nullptr
    ) {
        push_scope();
        declare_match_arm_bindings(arm);
        CheckedStatements checked = check_statements(body, false);
        if (checked.flow == Flow::Returns) {
            discard_scope();
            fail(loc, context + " must reach its final value");
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr_maybe_expected(value_expr, expected);
        std::optional<BorrowResultSource> borrow_source =
            finish_control_flow_borrow_result(
                loc,
                context,
                *value,
                checked.statements,
                local_scopes_.size() - 1,
                borrow_mark);
        bool diverges = is_diverging_value_expr(value_expr, *value);
        if (!diverges && !borrow_source && contains_borrow_type(value->type)) {
            pop_scope();
            fail(loc, context + " cannot produce borrow values yet");
        }
        if (!diverges && is_void_value_type(value->type)) {
            pop_scope();
            fail(loc, context + " must produce a value");
        }
        if (diverges) {
            release_temporary_borrows(borrow_mark);
            discard_scope();
            StateSnapshot state = snapshot_states();
            return CheckedExprBlock{
                std::move(checked.statements),
                std::move(value),
                std::move(state),
                std::nullopt,
                true
            };
        }
        if (!borrow_source) {
            value = materialize_value_before_auto_destroy_cleanup(
                loc,
                std::move(value),
                checked.statements,
                local_scopes_.size() - 1,
                "$block",
                context + " result"
            );
        }
        pop_scope();
        StateSnapshot state = snapshot_states();
        return CheckedExprBlock{
            std::move(checked.statements),
            std::move(value),
            std::move(state),
            std::move(borrow_source)
        };
    }

    CheckedExprBlock check_value_block(
        SourceLocation loc,
        const std::string& context,
        const std::vector<StmtPtr>& body,
        const Expr& value_expr,
        const IrType* expected = nullptr
    ) {
        push_scope();
        CheckedStatements checked = check_statements(body, false);
        if (checked.flow == Flow::Returns) {
            discard_scope();
            fail(loc, context + " must reach its final value");
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr_maybe_expected(value_expr, expected);
        std::optional<BorrowResultSource> borrow_source =
            finish_control_flow_borrow_result(
                loc,
                context,
                *value,
                checked.statements,
                local_scopes_.size() - 1,
                borrow_mark);
        bool diverges = is_diverging_value_expr(value_expr, *value);
        if (!diverges && !borrow_source && is_borrow_type(value->type)) {
            pop_scope();
            fail(loc, context + " cannot produce borrow values yet");
        }
        if (!diverges && is_void_value_type(value->type)) {
            pop_scope();
            fail(loc, context + " must produce a value");
        }
        if (diverges) {
            release_temporary_borrows(borrow_mark);
            discard_scope();
            StateSnapshot state = snapshot_states();
            return CheckedExprBlock{
                std::move(checked.statements),
                std::move(value),
                std::move(state),
                std::nullopt,
                true
            };
        }
        if (!borrow_source) {
            value = materialize_value_before_auto_destroy_cleanup(
                loc,
                std::move(value),
                checked.statements,
                local_scopes_.size() - 1,
                "$block",
                context + " result"
            );
        }
        pop_scope();
        StateSnapshot state = snapshot_states();
        return CheckedExprBlock{
            std::move(checked.statements),
            std::move(value),
            std::move(state),
            std::move(borrow_source)
        };
    }

    IrExprPtr make_enum_construct(SourceLocation loc, const EnumCaseInfo& info, std::vector<IrExprPtr> args) {
        if (args.size() != info.payloads.size()) {
            fail(loc, "wrong payload count for enum case '" + info.name + "'");
        }

        std::vector<IrExprPtr> payloads;
        payloads.reserve(args.size());
        for (std::size_t i = 0; i < args.size(); ++i) {
            IrExprPtr payload = std::move(args[i]);
            require_no_zone_pointer_escape(payload->loc, *payload, "enum payload");
            coerce_expr_to_expected(*payload, info.payloads[i]);
            require_assignable(loc, info.payloads[i], payload->type);
            payloads.push_back(std::move(payload));
        }

        EnumConstructorIrInfo ir_info;
        ir_info.enum_name = info.enum_name;
        ir_info.case_name = info.name;
        ir_info.tag = info.tag;
        ir_info.enum_type = info.enum_type;
        ir_info.payload_types = info.payloads;
        return make_enum_constructor_ir(loc, ir_info, std::move(payloads));
    }

    IrExprPtr check_unary(const Expr& expr, IrExprPtr lowered) {
        if (expr.op == TokenKind::Star) {
            std::size_t borrow_mark = temporary_borrow_mark();
            IrExprPtr pointer = check_expr(*expr_operand(expr));
            release_temporary_borrows(borrow_mark);

            IrType element_type = require_raw_pointer_materializable_type(expr.loc, pointer->type, "pointer dereference");
            return make_pointer_load_expr(expr.loc, std::move(pointer), element_type);
        }

        IrExprPtr operand = check_expr(*expr_operand(expr));
        if (is_borrow_type(operand->type)) {
            fail(expr.loc, "borrow expression result must be passed directly to a call");
        }
        switch (expr.op) {
            case TokenKind::Bang:
                require_logical_operand(expr.loc, operand->type);
                lowered->kind = IrExprKind::Unary;
                lowered->unary_op = IrUnaryOp::Not;
                lowered->type = bool_type(expr.loc);
                set_ir_expr_operand(*lowered, std::move(operand));
                return lowered;
            case TokenKind::Tilde:
                require_bitwise_not_operand(expr.loc, operand->type);
                lowered->kind = IrExprKind::Unary;
                lowered->unary_op = IrUnaryOp::BitNot;
                lowered->type = operand->type;
                set_ir_expr_operand(*lowered, std::move(operand));
                return lowered;
            default:
                fail(expr.loc, "unsupported unary operator");
        }
    }

    IrExprPtr check_cast(const Expr& expr, IrExprPtr lowered) {
        IrExprPtr operand = check_expr(*expr_operand(expr));
        IrType target = resolve_executable_type(expr.cast_type);
        if (is_borrow_type(operand->type) && !is_raw_pointer_type(target)) {
            fail(expr.loc, "borrow expression result must be passed directly to a call");
        }

        if (is_value_trait_object_type(target)) {
            TraitObjectConversion conversion =
                require_trait_object_conversion(expr.loc, operand->type, target);
            return make_trait_object_cast_expr(
                expr.loc,
                std::move(operand),
                std::move(target),
                std::move(conversion.vtable_name),
                conversion.vtable_offset);
        }

        bool integer_cast = is_value_integer_type(operand->type) && is_value_integer_type(target);
        bool float_cast = is_value_float_type(operand->type) && is_value_float_type(target);
        bool integer_float_cast =
            (is_value_integer_type(operand->type) && is_value_float_type(target)) ||
            (is_value_float_type(operand->type) && is_value_integer_type(target));
        bool raw_pointer_cast = is_raw_pointer_cast(operand->type, target);
        if (!integer_cast && !float_cast && !integer_float_cast && !raw_pointer_cast) {
            fail(expr.loc, "explicit casts currently require integer types, float types, integer/float casts, or raw pointer casts, got " +
                           type_name(operand->type) + " as " + type_name(target));
        }

        lowered->kind = IrExprKind::Cast;
        lowered->type = target;
        set_ir_expr_operand(*lowered, std::move(operand));
        return lowered;
    }

    TraitObjectConversion require_trait_object_conversion(SourceLocation loc, const IrType& source, const IrType& target) {
        if (source.qualifier != TypeQualifier::Value) {
            fail(loc, "trait object conversions currently require value operands, got " + type_name(source));
        }
        if (source.primitive == IrPrimitiveKind::TraitObject) {
            if (!trait_application_implies_trait(source.name, source.args, target.name, target.args)) {
                fail(loc, "trait object upcast from " + type_name(source) + " to " +
                              type_name(target) +
                              " requires the target trait to be the same trait or a supertrait");
            }
            TraitObjectConversion conversion;
            conversion.vtable_offset = trait_object_upcast_vtable_offset(loc, source, target);
            return conversion;
        }
        if (is_owner_type(source) || contains_borrow_type(source)) {
            fail(loc, "trait object conversions currently require copyable non-borrow values, got " + type_name(source));
        }
        if (!type_implements_trait(target.name, target.args, source)) {
            fail(loc, "type " + type_name(source) + " does not implement trait '" +
                           trait_application_display(target.name, target.args) + "' for dyn conversion");
        }
        TraitObjectConversion conversion;
        conversion.vtable_name = register_trait_object_vtable(loc, source, target);
        return conversion;
    }

    const ImplMethodInfo* find_concrete_trait_method_impl(
        SourceLocation loc,
        const IrType& source,
        const IrType& target,
        const std::string& method_name
    ) const {
        auto found = method_impls_.find(method_lookup_key(source, method_name));
        if (found == method_impls_.end()) return nullptr;

        const ImplMethodInfo* selected = nullptr;
        for (const auto& candidate : found->second) {
            if (candidate.trait_name != target.name) continue;
            if (!same_type_list(candidate.trait_args, target.args)) continue;
            if (selected) {
                fail(loc, "trait object method '" + method_name + "' for " + type_name(source) + " is ambiguous");
            }
            selected = &candidate;
        }
        return selected;
    }

    void require_trait_object_method_object_safe(SourceLocation loc, const TraitInfo::Method& method) const {
        if (!method.generics.empty()) {
            fail(loc,
                 "trait object method '" + method.name +
                     "' is generic and is not object-safe; use static dispatch for generic trait methods");
        }
        if (method.params.empty() ||
            method.params[0].name != "Self" ||
            method.params[0].qualifier != TypeQualifier::Value) {
            fail(method.loc,
                 "trait object method '" + method.name + "' must take value self as its first parameter");
        }
        for (std::size_t i = 1; i < method.params.size(); ++i) {
            if (type_ref_mentions_name(method.params[i], "Self")) {
                fail(method.params[i].loc,
                     "trait object method '" + method.name + "' cannot mention Self outside the receiver yet");
            }
        }
        if (method.has_result && type_ref_mentions_name(method.result, "Self")) {
            fail(method.result.loc, "trait object method '" + method.name + "' cannot return Self yet");
        }
    }

    bool try_specialize_generic_trait_method_impl(
        SourceLocation loc,
        const IrType& source,
        const IrType& target,
        const std::string& method_name,
        ImplMethodInfo& selected
    ) {
        struct Match {
            const ImplMethodInfo* method = nullptr;
            std::map<std::string, IrType> substitutions;
        };

        std::vector<Match> matches;
        std::string first_bound_failure;
        for (const auto& candidate : generic_method_impls_) {
            if (basename_of_qualified_name(candidate.fn->name) != method_name) continue;
            if (candidate.trait_name != target.name) continue;
            if (candidate.trait_args.size() != target.args.size()) continue;

            std::map<std::string, IrType> substitutions;
            if (!infer_generic_impl_method_substitutions(candidate, source, substitutions)) continue;

            bool trait_args_match = true;
            for (std::size_t i = 0; i < target.args.size(); ++i) {
                if (!infer_generic_pattern_type(
                        candidate.trait_args[i],
                        target.args[i],
                        candidate.generic_names,
                        substitutions)) {
                    trait_args_match = false;
                    break;
                }
            }
            if (!trait_args_match) continue;

            std::set<std::string> visiting;
            std::string bound_failure;
            if (!impl_generic_bounds_satisfied(candidate.generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }
            if (!impl_generic_bounds_satisfied(candidate.method_generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }

            matches.push_back(Match{&candidate, std::move(substitutions)});
        }

        if (matches.empty()) {
            if (!first_bound_failure.empty()) fail(loc, first_bound_failure);
            return false;
        }
        if (matches.size() > 1) {
            fail(loc, "trait object method '" + method_name + "' for " + type_name(source) + " is ambiguous");
        }

        selected = specialize_generic_impl_method_with_substitutions(
            *matches.front().method,
            source,
            method_name,
            std::move(matches.front().substitutions));
        return true;
    }

    std::string register_trait_object_vtable(SourceLocation loc, const IrType& source, const IrType& target) {
        std::string key = type_name(source) + " as " + type_name(target);
        auto existing = trait_object_vtable_names_.find(key);
        if (existing != trait_object_vtable_names_.end()) return existing->second;

        auto trait_found = traits_.find(target.name);
        if (trait_found == traits_.end()) fail(loc, "unknown trait '" + target.name + "' in trait object type");
        const TraitInfo& trait = trait_found->second;

        IrTraitObjectVTable table;
        table.name = trait_object_vtable_name(source, target);
        table.object_type = target;
        table.concrete_type = source;
        table.loc = loc;

        std::vector<TraitObjectMethodEntry> object_methods = collect_trait_object_methods(trait, target.args);
        for (const auto& entry : object_methods) {
            const TraitInfo::Method& trait_method = *entry.method;
            require_trait_object_method_object_safe(loc, trait_method);

            IrType method_target = target;
            method_target.name = entry.trait->name;
            method_target.args = entry.trait_args;

            ImplMethodInfo method_impl;
            const ImplMethodInfo* concrete_impl =
                find_concrete_trait_method_impl(loc, source, method_target, trait_method.name);
            if (concrete_impl) {
                method_impl = *concrete_impl;
            } else if (!try_specialize_generic_trait_method_impl(
                           loc,
                           source,
                           method_target,
                           trait_method.name,
                           method_impl)) {
                fail(loc,
                     "trait object conversion could not find impl method '" + trait_method.name + "' for " +
                         trait_application_display(method_target.name, method_target.args) + " for " + type_name(source));
            }
            queue_impl_method_for_lowering(method_impl);

            IrTraitObjectVTableMethod method;
            method.impl_name = method_impl.lowered_name;
            method.thunk_name = table.name + "::" + mangle_text_key(entry.trait->name) + "::" + trait_method.name;
            method.concrete_receiver_type = source;
            method.result_type = method_impl.sig.result;
            method.impl_params = method_impl.sig.params;
            method.erased_params.reserve(method_impl.sig.params.size());
            IrType data_pointer = void_type(loc);
            data_pointer.qualifier = TypeQualifier::Ptr;
            method.erased_params.push_back(data_pointer);
            for (std::size_t i = 1; i < method_impl.sig.params.size(); ++i) {
                method.erased_params.push_back(method_impl.sig.params[i]);
            }
            table.methods.push_back(std::move(method));
        }

        std::string name = table.name;
        trait_object_vtable_names_[key] = name;
        trait_object_vtables_.push_back(std::move(table));
        return name;
    }

    IrExprPtr check_try(const Expr& expr) {
        IrExprPtr operand = check_expr(*expr_operand(expr));
        if (is_borrow_type(operand->type)) {
            fail(expr.loc, "borrow expression result must be passed directly to a call");
        }
        if (!is_value_enum_type(operand->type)) {
            fail(expr.loc, "postfix ? requires an enum value, got " + type_name(operand->type));
        }
        if (!same_type(current_return_, operand->type) && !is_value_enum_type(current_return_)) {
            fail(expr.loc, "postfix ? can only early-return from functions returning " +
                           type_name(operand->type) + ", got " + type_name(current_return_));
        }

        auto enum_found = enums_.find(operand->type.name);
        if (enum_found == enums_.end()) {
            fail(expr.loc, "unknown enum type '" + operand->type.name + "'");
        }

        TryEnumShape shape = analyze_try_enum_shape(
            enum_found->second.name,
            try_cases_for_enum(expr.loc, enum_found->second, operand->type.args));
        if (!shape.supported) fail(expr.loc, shape.diagnostic);

        TryEnumShape return_shape = shape;
        bool converts_residual = !same_type(current_return_, operand->type);
        if (converts_residual) {
            if (has_aggregate_enum_layout(operand->type) || has_aggregate_enum_layout(current_return_)) {
                fail(expr.loc, "postfix ? residual conversion for aggregate enum layouts is planned but is not supported yet");
            }
            auto return_enum = enums_.find(current_return_.name);
            if (return_enum == enums_.end()) {
                fail(expr.loc, "unknown enum type '" + current_return_.name + "'");
            }
            return_shape = analyze_try_enum_shape(
                return_enum->second.name,
                try_cases_for_enum(expr.loc, return_enum->second, current_return_.args),
                "postfix ? return type"
            );
            if (!return_shape.supported) fail(expr.loc, return_shape.diagnostic);
            require_try_residual_compatible(expr.loc, operand->type, current_return_, shape, return_shape);
        }

        bool residual_has_payload = !return_shape.residual_payloads.empty();
        IrType residual_payload_type;
        if (residual_has_payload) {
            residual_payload_type = return_shape.residual_payloads[0];
        }
        return make_ir_try_expr(
            expr.loc,
            std::move(operand),
            shape.success_payload_type,
            shape.success_tag,
            converts_residual,
            return_shape.residual_tag,
            residual_has_payload,
            std::move(residual_payload_type),
            make_active_loop_exit_owner_cleanup_for_branch(expr.loc));
    }

    void require_try_residual_compatible(SourceLocation loc,
                                         const IrType& operand_type,
                                         const IrType& return_type,
                                         const TryEnumShape& operand_shape,
                                         const TryEnumShape& return_shape) const {
        if (operand_shape.residual_payloads.size() != return_shape.residual_payloads.size()) {
            fail(loc,
                 "postfix ? cannot convert residual from " + type_name(operand_type) +
                     " to " + type_name(return_type));
        }
        if (operand_shape.residual_payloads.empty()) return;
        require_assignable(loc, return_shape.residual_payloads[0], operand_shape.residual_payloads[0]);
    }

    IrExprPtr check_null_coalesce(const Expr& expr) {
        IrExprPtr lhs = check_expr(*expr_left(expr));
        if (is_borrow_type(lhs->type)) {
            fail(expr.loc, "borrow expression result must be passed directly to a call");
        }
        if (!is_value_enum_type(lhs->type)) {
            fail(expr.loc, "operator ?? requires an Option/Result-style enum value, got " + type_name(lhs->type));
        }

        auto enum_found = enums_.find(lhs->type.name);
        if (enum_found == enums_.end()) {
            fail(expr.loc, "unknown enum type '" + lhs->type.name + "'");
        }

        TryEnumShape shape = analyze_try_enum_shape(
            enum_found->second.name,
            try_cases_for_enum(expr.loc, enum_found->second, lhs->type.args),
            "operator ??");
        if (!shape.supported) fail(expr.loc, shape.diagnostic);

        StateSnapshot after_lhs = snapshot_states();
        IrExprPtr rhs = check_expr(*expr_right(expr));
        if (is_borrow_type(rhs->type)) {
            fail(expr.loc, "borrow expression result must be passed directly to a call");
        }
        coerce_expr_to_expected(*rhs, shape.success_payload_type);
        require_assignable(expr.loc, shape.success_payload_type, rhs->type);
        StateSnapshot after_rhs = snapshot_states();
        require_same_states(expr.loc, after_lhs, after_rhs, "changes ownership state in ?? fallback");
        restore_merged_states(after_lhs, after_rhs);

        return make_ir_null_coalesce_expr(
            expr.loc,
            std::move(lhs),
            std::move(rhs),
            shape.success_payload_type,
            shape.success_tag);
    }

    std::vector<TryEnumCaseShape> try_cases_for_enum(
        SourceLocation loc,
        const EnumInfo& enum_info,
        const std::vector<IrType>& type_args
    ) {
        std::vector<TryEnumCaseShape> cases;
        cases.reserve(enum_info.case_names.size());
        for (const auto& name : enum_info.case_names) {
            std::string case_key = qualify_in_module(enum_info.module_name, name);
            auto found = enum_cases_.find(case_key);
            if (found == enum_cases_.end()) continue;
            EnumCaseInfo info = found->second;
            if (info.is_generic) {
                info = specialize_enum_case_info(loc, info, type_args);
            }
            cases.push_back(TryEnumCaseShape{
                info.name,
                info.tag,
                info.payloads,
                info.loc
            });
        }
        return cases;
    }

    IrExprPtr check_borrow(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        if (const Expr* operand = expr_operand(expr).get()) {
            if (operand->kind == ExprKind::Name) {
                if (LocalInfo* source = find_local_slot(operand->name)) {
                    if (is_borrow_type(source->type)) {
                        if (auto error = local_unavailable_binding_error(operand->name, *source)) {
                            fail(expr.loc, *error);
                        }
                        require_can_reborrow(expr.loc, operand->name, *source, expr.mutable_borrow);
                        add_borrow_source(*source, "", expr.mutable_borrow);
                        borrow_context_.push_temporary(operand->name, "", expr.mutable_borrow);
                        return make_borrow_expr(
                            expr.loc,
                            operand->name,
                            "",
                            make_local_lvalue_expr(operand->loc, operand->name, source->type),
                            expr.mutable_borrow,
                            source->type);
                    }
                }
            }
        }

        TrackedAggregateAccess access;
        if (expr_operand(expr)) {
            if (!try_build_tracked_aggregate_access(*expr_operand(expr), access)) {
                fail(expr.loc, "borrow expression requires a local binding, field access, tuple index, or constant aggregate index");
            }
        } else {
            LocalInfo& source = require_live_local(expr.loc, expr.name);
            access.base_name = expr.name;
            access.base_type = source.type;
            access.type = source.type;
            access.path.clear();
            access.expr = make_local_lvalue_expr(expr.loc, expr.name, source.type);
        }

        LocalInfo& source = require_live_local(expr.loc, access.base_name);
        if (is_borrow_type(access.type) || access.type.qualifier == TypeQualifier::Ptr) {
            fail(expr.loc, "cannot borrow reference-like value '" + local_borrow_path_display(access.base_name, access.path) + "'");
        }
        if (is_void_value_type(access.type)) {
            fail(expr.loc, "cannot borrow void value '" + local_borrow_path_display(access.base_name, access.path) + "'");
        }

        const bool base_is_borrow_binding = is_borrow_type(source.type);
        if (base_is_borrow_binding) {
            if (expr.mutable_borrow &&
                access.has_final_field_mutability &&
                !access.final_field_mutable) {
                fail(expr.loc,
                     "cannot mutably borrow immutable field '" + access.final_field_label +
                         "' of struct '" + access.final_container_name + "'");
            }
            require_can_reborrow_path(expr.loc, access.base_name, source, access.path, expr.mutable_borrow);
            add_borrow_source(source, access.path, expr.mutable_borrow);
            borrow_context_.push_temporary(access.base_name, access.path, expr.mutable_borrow);
            return make_borrow_expr(
                expr.loc,
                access.base_name,
                access.path,
                std::move(access.expr),
                expr.mutable_borrow,
                access.type);
        }

        if (expr.mutable_borrow) {
            if (auto error = local_mutable_borrow_error(access.base_name, source)) fail(expr.loc, *error);
            if (access.has_final_field_mutability && !access.final_field_mutable) {
                fail(expr.loc,
                     "cannot mutably borrow immutable field '" + access.final_field_label +
                         "' of struct '" + access.final_container_name + "'");
            }
        }
        if (is_owner_type(access.type)) {
            if (access.path.empty()) {
                if (local_has_moved_or_dropped_owned_fields(source)) {
                    fail(expr.loc, "cannot borrow partially moved owning binding '" + access.base_name + "'");
                }
            } else {
                require_owned_field_alive(expr.loc, access.base_name, source, access.path);
            }
        }

        require_can_borrow_path(expr.loc, access.base_name, source, access.path, expr.mutable_borrow);
        add_borrow_source(source, access.path, expr.mutable_borrow);

        borrow_context_.push_temporary(access.base_name, access.path, expr.mutable_borrow);
        return make_borrow_expr(
            expr.loc,
            access.base_name,
            access.path,
            std::move(access.expr),
            expr.mutable_borrow,
            access.type);
    }

    IrExprPtr check_format_print(const Expr& expr, IrExprPtr lowered, const std::string& print_name) {
        (void)lowered;
        if (expr.args.empty()) fail(expr.loc, "'" + expr.name + "' expects a string literal format argument");
        const Expr& format = *expr.args[0];
        if (format.kind != ExprKind::String) {
            fail(format.loc, "'" + expr.name + "' expects a string literal format argument");
        }

        ParsedFormatString format_string = parse_format_string(format.loc, format.string_value, expr.args.size() - 1);
        std::vector<IrExprPtr> args;
        args.reserve(expr.args.size() - 1);

        for (std::size_t i = 1; i < expr.args.size(); ++i) {
            IrExprPtr arg = check_expr(*expr.args[i]);
            const IrFormatSpec& spec = format_string.specs.at(i - 1);
            if (is_borrow_type(arg->type)) {
                fail(expr.args[i]->loc, "format arguments cannot be borrow values");
            }
            if (is_owner_type(arg->type)) {
                fail(expr.args[i]->loc, "format arguments cannot move owning values yet");
            }
            bool is_bool = arg->type.qualifier == TypeQualifier::Value && arg->type.primitive == IrPrimitiveKind::Bool;
            bool is_supported_float =
                arg->type.qualifier == TypeQualifier::Value &&
                (arg->type.primitive == IrPrimitiveKind::F32 || arg->type.primitive == IrPrimitiveKind::F64);
            if (spec.precision >= 0 && !is_supported_float) {
                fail(expr.args[i]->loc, "format precision placeholders require f32 or f64 arguments, got " + type_name(arg->type));
            }
            if (is_value_float_type(arg->type) && !is_supported_float) {
                fail(expr.args[i]->loc, "format arguments do not support " + type_name(arg->type) + " yet");
            }
            if (!is_value_integer_type(arg->type) && !is_bool && !is_supported_float) {
                fail(expr.args[i]->loc, "format arguments currently support integer, bool, f32, and f64 values, got " + type_name(arg->type));
            }
            args.push_back(std::move(arg));
        }
        return make_format_print_expr(
            expr.loc,
            i64_type(expr.loc),
            std::move(format_string.parts),
            std::move(format_string.specs),
            std::move(args),
            is_println_name(print_name)
        );
    }

    static StmtPtr make_format_in_var_decl(SourceLocation loc,
                                           std::string name,
                                           ExprPtr init,
                                           bool mutable_binding) {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::VarDecl;
        stmt->loc = loc;
        stmt->binding.name = std::move(name);
        stmt->binding.mutable_binding = mutable_binding;
        stmt->binding.loc = loc;
        stmt->binding.init = std::move(init);
        return stmt;
    }

    static StmtPtr make_format_in_expr_stmt(SourceLocation loc, ExprPtr expr) {
        auto stmt = std::make_unique<Stmt>();
        stmt->kind = StmtKind::ExprStmt;
        stmt->loc = loc;
        stmt->expr = std::move(expr);
        return stmt;
    }

    std::optional<std::string> format_in_display_trait_for_type(const IrType& type) const {
        if (type.qualifier != TypeQualifier::Value) return std::nullopt;
        if (is_owner_type(type) || contains_borrow_type(type)) return std::nullopt;
        static const std::vector<IrType> no_trait_args;
        if (type_implements_trait("std::Display", no_trait_args, type)) return std::string("std::Display");
        if (type_implements_trait("std::fmt::Display", no_trait_args, type)) return std::string("std::fmt::Display");
        return std::nullopt;
    }

    std::optional<FormatInAppendTarget> format_in_append_target_from_type(SourceLocation loc, const IrType& type) const {
        if (auto target = builtin_format_in_append_target_from_type(type)) return target;
        if (auto display_trait = format_in_display_trait_for_type(type)) {
            return format_in_display_append_target(*display_trait);
        }
        if (type.primitive != IrPrimitiveKind::Unknown) {
            fail(loc, unsupported_format_in_value_message(type));
        }
        return std::nullopt;
    }

    FormatInAppendTarget format_in_append_target_for_local(SourceLocation loc, const std::string& name) {
        const LocalInfo* local = find_local_slot(name);
        if (!local) throw CompileError("internal error: missing format_in! value local '" + name + "'");
        if (auto target = format_in_append_target_from_type(loc, local->type)) return *target;
        fail(loc, "format_in! could not infer a supported value type for '" + name + "'");
    }

    static std::uint64_t format_in_initial_capacity(const ParsedFormatString& format_string) {
        std::uint64_t capacity = 0;
        for (const auto& part : format_string.parts) capacity += part.size();
        capacity += static_cast<std::uint64_t>(format_string.specs.size());
        return capacity;
    }

    ExprPtr make_format_in_append_call(SourceLocation loc,
                                       const std::string& result_name,
                                       const Expr& zone_arg,
                                       std::string part) {
        std::vector<ExprPtr> args;
        args.push_back(clone_expression_tree(zone_arg));
        args.push_back(make_ast_string_expr(loc, std::move(part)));
        return make_ast_method_call_expr(
            loc,
            make_ast_name_expr(loc, result_name),
            "append_string_in",
            {},
            std::move(args)
        );
    }

    ExprPtr make_format_in_append_call(SourceLocation loc,
                                       const std::string& result_name,
                                       const Expr& zone_arg,
                                       FormatInAppendTarget target,
                                       ExprPtr value_arg,
                                       const IrFormatSpec& spec) {
        std::vector<ExprPtr> args;
        args.push_back(clone_expression_tree(zone_arg));
        if (target.kind == FormatInAppendKind::I64 || target.kind == FormatInAppendKind::U64) {
            TypeRef cast_type;
            cast_type.loc = loc;
            cast_type.name = target.kind == FormatInAppendKind::I64 ? "i64" : "u64";
            value_arg = make_ast_cast_expr(loc, std::move(value_arg), std::move(cast_type));
        }
        args.push_back(std::move(value_arg));
        if (format_in_append_target_is_float(target)) {
            args.push_back(make_ast_integer_expr(
                loc,
                static_cast<std::uint64_t>(spec.precision >= 0 ? spec.precision : 6)
            ));
        }
        return make_ast_method_call_expr(
            loc,
            make_ast_name_expr(loc, result_name),
            format_in_builtin_append_method_name(target),
            {},
            std::move(args)
        );
    }

    ExprPtr make_format_in_display_call(SourceLocation loc,
                                        const Expr& zone_arg,
                                        const FormatInAppendTarget& target,
                                        ExprPtr value_arg) {
        std::vector<ExprPtr> args;
        args.push_back(make_ast_borrow_expr(loc, std::move(value_arg), false));
        args.push_back(clone_expression_tree(zone_arg));
        return make_ast_call_expr(
            loc,
            target.display_trait_name + "::format_in",
            nullptr,
            std::move(args));
    }

    ExprPtr make_format_in_extend_from_string_call(SourceLocation loc,
                                                   const std::string& result_name,
                                                   const Expr& zone_arg,
                                                   const std::string& display_name) {
        std::vector<ExprPtr> args;
        args.push_back(clone_expression_tree(zone_arg));
        args.push_back(make_ast_method_call_expr(
            loc,
            make_ast_name_expr(loc, display_name),
            "as_slice",
            {},
            {}));
        return make_ast_method_call_expr(
            loc,
            make_ast_name_expr(loc, result_name),
            "extend_from_slice_in",
            {},
            std::move(args));
    }

    void append_format_in_statement(SourceLocation loc, StmtPtr stmt, std::vector<IrStmtPtr>& statements) {
        CheckedStatement checked = check_statement(*stmt);
        if (checked.flow != Flow::Continues) {
            discard_scope();
            fail(loc, "format_in! generated control-flow where a value append was expected");
        }
        statements.push_back(std::move(checked.statement));
    }

    IrExprPtr check_format_in_macro(const Expr& expr, std::vector<ExprPtr> args) {
        if (args.size() < 2) {
            fail(expr.loc, "format_in! expects a mutable zone borrow, a string literal format, and optional values");
        }
        const Expr& zone_arg = *args[0];
        const Expr& format = *args[1];
        if (format.kind != ExprKind::String) {
            fail(format.loc, "format_in! expects a string literal format argument");
        }

        ParsedFormatString format_string = parse_format_string(format.loc, format.string_value, args.size() - 2);
        std::string result_name = make_hidden_local("$format");
        std::vector<IrStmtPtr> statements;
        push_scope();

        std::vector<ExprPtr> new_args;
        new_args.push_back(clone_expression_tree(zone_arg));
        new_args.push_back(make_ast_integer_expr(expr.loc, format_in_initial_capacity(format_string)));
        append_format_in_statement(expr.loc, make_format_in_var_decl(
            expr.loc,
            result_name,
            make_ast_call_expr(expr.loc, "std::string::new", nullptr, std::move(new_args)),
            true
        ), statements);

        for (std::size_t i = 0; i < format_string.specs.size(); ++i) {
            if (!format_string.parts[i].empty()) {
                append_format_in_statement(expr.loc, make_format_in_expr_stmt(
                    expr.loc,
                    make_format_in_append_call(expr.loc, result_name, zone_arg, format_string.parts[i])
                ), statements);
            }
            const std::string value_name = make_hidden_local("$format_value");
            append_format_in_statement(args[i + 2]->loc, make_format_in_var_decl(
                args[i + 2]->loc,
                value_name,
                clone_expression_tree(*args[i + 2]),
                false
            ), statements);
            FormatInAppendTarget target = format_in_append_target_for_local(args[i + 2]->loc, value_name);
            if (format_string.specs[i].precision >= 0 && !format_in_append_target_is_float(target)) {
                const LocalInfo* local = find_local_slot(value_name);
                if (!local) throw CompileError("internal error: missing format_in! value local '" + value_name + "'");
                fail(args[i + 2]->loc,
                     "format precision placeholders require f32 or f64 arguments, got " + type_name(local->type));
            }
            ExprPtr value = make_ast_name_expr(args[i + 2]->loc, value_name);
            if (format_in_append_target_is_display(target)) {
                const std::string display_name = make_hidden_local("$format_display");
                append_format_in_statement(args[i + 2]->loc, make_format_in_var_decl(
                    args[i + 2]->loc,
                    display_name,
                    make_format_in_display_call(
                        args[i + 2]->loc,
                        zone_arg,
                        target,
                        std::move(value)),
                    true
                ), statements);
                append_format_in_statement(args[i + 2]->loc, make_format_in_expr_stmt(
                    args[i + 2]->loc,
                    make_format_in_extend_from_string_call(
                        args[i + 2]->loc,
                        result_name,
                        zone_arg,
                        display_name)
                ), statements);
                continue;
            }
            ExprPtr append_call = make_format_in_append_call(
                args[i + 2]->loc,
                result_name,
                zone_arg,
                target,
                std::move(value),
                format_string.specs[i]);
            append_format_in_statement(args[i + 2]->loc, make_format_in_expr_stmt(
                args[i + 2]->loc,
                std::move(append_call)
            ), statements);
        }
        if (!format_string.parts.empty() && !format_string.parts.back().empty()) {
            append_format_in_statement(expr.loc, make_format_in_expr_stmt(
                expr.loc,
                make_format_in_append_call(expr.loc, result_name, zone_arg, format_string.parts.back())
            ), statements);
        }

        Expr result_expr;
        result_expr.kind = ExprKind::Name;
        result_expr.loc = expr.loc;
        result_expr.name = result_name;
        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr(result_expr);
        std::optional<BorrowResultSource> borrow_source =
            finish_control_flow_borrow_result(
                expr.loc,
                "format_in! expression",
                *value,
                statements,
                local_scopes_.size() - 1,
                borrow_mark);
        if (!borrow_source && is_borrow_type(value->type)) {
            pop_scope();
            fail(expr.loc, "format_in! expression cannot produce borrow values");
        }
        if (is_void_value_type(value->type)) {
            pop_scope();
            fail(expr.loc, "format_in! expression must produce a value");
        }
        if (!borrow_source) {
            value = materialize_value_before_auto_destroy_cleanup(
                expr.loc,
                std::move(value),
                statements,
                local_scopes_.size() - 1,
                "$format",
                "format_in! result"
            );
        }
        IrType result_type = value->type;
        pop_scope();

        IrExprPtr block = make_ir_block_expr(
            expr.loc,
            {},
            std::move(result_type),
            std::move(statements),
            std::move(value)
        );
        if (borrow_source) activate_control_flow_borrow_result(expr.loc, *block, *borrow_source);
        return block;
    }

    static std::string mangle_text_key(const std::string& text) {
        std::string out;
        out.reserve(text.size());
        for (unsigned char c : text) {
            if (std::isalnum(c)) out.push_back(static_cast<char>(c));
            else out.push_back('_');
        }
        return out;
    }

    static std::string type_specialization_key(const IrType& type) {
        std::string key;
        switch (type.qualifier) {
            case TypeQualifier::Value: break;
            case TypeQualifier::Own: key += "own "; break;
            case TypeQualifier::Ref: key += "ref "; break;
            case TypeQualifier::MutRef: key += "ref mut "; break;
            case TypeQualifier::Ptr: key += "ptr "; break;
        }
        if (type.primitive == IrPrimitiveKind::Vector && type.args.size() == 1) {
            key += "Vec[";
            key += type_specialization_key(type.args[0]);
            key += "; cap=" + std::to_string(type.array_size) + "]";
            return key;
        }
        (void)key;
        return type_name(type);
    }

    static std::string mangle_type_key(const IrType& type) {
        return mangle_text_key(type_specialization_key(type));
    }

    static bool type_ref_mentions_name(const TypeRef& type, const std::string& name) {
        if (type.name == name) return true;
        for (const auto& arg : type.args) {
            if (type_ref_mentions_name(arg, name)) return true;
        }
        return false;
    }

    static std::string trait_object_vtable_name(const IrType& source, const IrType& target) {
        return "dyn::vtable::" + mangle_type_key(source) + "::" + mangle_type_key(target);
    }

    static std::string method_lookup_key(const IrType& receiver_type, const std::string& method_name) {
        return type_name(receiver_type) + "::" + method_name;
    }

    static std::string drop_impl_key(const IrType& type) {
        return type_name(type);
    }

    static bool is_drop_impl_method(const ImplMethodInfo& method) {
        return is_drop_trait_method_name(method.trait_name, basename_of_qualified_name(method.fn->name));
    }

    static bool split_associated_call_name(const std::string& name, std::string& receiver_name, std::string& method_name) {
        std::size_t split = name.rfind("::");
        if (split == std::string::npos || split == 0 || split + 2 >= name.size()) return false;
        receiver_name = name.substr(0, split);
        method_name = name.substr(split + 2);
        return true;
    }

    static std::vector<TypeRef> combined_associated_type_args(const Expr& expr) {
        std::vector<TypeRef> args = expr_receiver_type_args(expr);
        const ExprTypeArgs& type_args = expr_type_args(expr);
        args.insert(args.end(), type_args.begin(), type_args.end());
        return args;
    }

    static std::string impl_method_lowered_name(
        const IrType& self_type,
        const std::string& trait_name,
        const std::string& method_name
    ) {
        std::string trait_key = trait_name.empty() ? "inherent" : mangle_text_key(trait_name);
        return "impl::" + mangle_type_key(self_type) + "::" + trait_key + "::" + method_name;
    }

    static std::string specialized_impl_method_lowered_name(
        const ImplMethodInfo& method,
        const IrType& self_type,
        const std::string& method_name,
        const std::map<std::string, IrType>& substitutions
    ) {
        std::string name = impl_method_lowered_name(self_type, method.trait_name, method_name);
        if (!method.method_generic_names.empty()) {
            name += "__G";
            for (const auto& generic_name : method.method_generic_names) {
                auto found = substitutions.find(generic_name);
                if (found == substitutions.end()) name += "_unused";
                else name += "_" + mangle_type_key(found->second);
            }
        }
        return name;
    }

    std::string generic_specialization_name(const FunctionDecl& fn, const std::map<std::string, IrType>& substitutions) const {
        std::string name = fn.name + "__G";
        for (const auto& generic : fn.generics) {
            auto found = substitutions.find(generic.name);
            if (found == substitutions.end()) name += "_unused";
            else name += "_" + mangle_type_key(found->second);
        }
        return name;
    }

    IrType resolve_type_with_substitutions(const TypeRef& type, const std::map<std::string, IrType>& substitutions) {
        std::map<std::string, IrType> previous = std::move(current_type_substitutions_);
        current_type_substitutions_ = substitutions;
        IrType resolved = resolve_executable_type(type);
        current_type_substitutions_ = std::move(previous);
        return resolved;
    }

    bool infer_generic_impl_method_substitutions(
        const ImplMethodInfo& method,
        const IrType& receiver_type,
        std::map<std::string, IrType>& substitutions
    ) const {
        substitutions.clear();
        if (!infer_generic_pattern_type(method.receiver_type, receiver_type, method.generic_names, substitutions)) return false;
        for (const auto& generic_name : method.generic_names) {
            if (!substitutions.count(generic_name)) return false;
        }
        substitutions.emplace("Self", receiver_type);
        return true;
    }

    bool infer_generic_trait_impl_substitutions(
        const GenericTraitImplInfo& impl,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const IrType& self_type,
        std::map<std::string, IrType>& substitutions
    ) const {
        substitutions.clear();
        if (impl.trait_name != trait_name) return false;
        if (impl.trait_args.size() != trait_args.size()) return false;
        if (!infer_generic_pattern_type(impl.self_type, self_type, impl.generic_names, substitutions)) return false;
        for (std::size_t i = 0; i < trait_args.size(); ++i) {
            if (!infer_generic_pattern_type(impl.trait_args[i], trait_args[i], impl.generic_names, substitutions)) {
                return false;
            }
        }
        for (const auto& generic_name : impl.generic_names) {
            if (!substitutions.count(generic_name)) return false;
        }
        substitutions.emplace("Self", self_type);
        return true;
    }

    bool impl_generic_bounds_satisfied(
        const std::vector<GenericTraitBound>& bounds,
        const std::map<std::string, IrType>& substitutions,
        std::set<std::string>& visiting,
        std::string* failure
    ) const {
        for (const auto& bound : bounds) {
            auto self_found = substitutions.find(bound.generic_name);
            if (self_found == substitutions.end()) continue;

            IrType self_type = self_found->second;
            self_type.qualifier = TypeQualifier::Value;
            std::vector<IrType> trait_args;
            trait_args.reserve(bound.trait_args.size());
            for (const auto& arg : bound.trait_args) {
                trait_args.push_back(substitute_inferred_type(arg, substitutions));
            }
            if (!type_implements_trait(bound.trait_name, trait_args, self_type, visiting)) {
                if (failure) {
                    *failure = "type " + type_name(self_type) + " does not implement trait '" +
                        trait_application_display(bound.trait_name, trait_args) +
                        "' required by impl generic parameter '" + bound.generic_name + "'";
                }
                return false;
            }
        }
        return true;
    }

    bool type_implements_trait(
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const IrType& self_type
    ) const {
        std::set<std::string> visiting;
        return type_implements_trait(trait_name, trait_args, self_type, visiting);
    }

    bool type_implements_trait(
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const IrType& self_type,
        std::set<std::string>& visiting
    ) const {
        std::string key = trait_impl_key(trait_name, trait_args, self_type);
        if (impl_keys_.count(key)) return true;
        if (!visiting.insert(key).second) return false;
        for (const auto& impl : generic_trait_impls_) {
            std::map<std::string, IrType> substitutions;
            if (!infer_generic_trait_impl_substitutions(impl, trait_name, trait_args, self_type, substitutions)) continue;
            if (impl_generic_bounds_satisfied(impl.generic_bounds, substitutions, visiting, nullptr)) {
                visiting.erase(key);
                return true;
            }
        }
        visiting.erase(key);
        return false;
    }

    ImplMethodInfo specialize_generic_impl_method(
        SourceLocation loc,
        const ImplMethodInfo& method,
        const IrType& receiver_type,
        const std::string& method_name
    ) {
        std::map<std::string, IrType> substitutions;
        if (!infer_generic_impl_method_substitutions(method, receiver_type, substitutions)) {
            fail(loc, "generic impl method '" + method_name + "' cannot be specialized for " + type_name(receiver_type));
        }

        std::string previous_module = current_module_name_;
        current_module_name_ = method.module_name;

        FunctionSig sig;
        sig.loc = method.fn->loc;
        sig.module_name = method.module_name;
        sig.is_public = method.is_public;
        for (const auto& param : method.fn->params) {
            IrType param_type = resolve_type_with_substitutions(param.type, substitutions);
            bool vec_view = false;
            sig.params.push_back(function_parameter_abi_type(
                param.type.loc,
                param_type,
                "an impl method parameter",
                vec_view));
        }
        sig.result = method.fn->has_return_type
            ? resolve_type_with_substitutions(method.fn->return_type, substitutions)
            : void_type(method.fn->loc);
        if (method.fn->has_return_type) {
            require_root_vector_runtime_abi(
                method.fn->return_type.loc,
                sig.result,
                "an impl method return type");
        }
        set_function_return_contracts(sig);
        apply_explicit_borrow_return_contract(sig, *method.fn);
        set_function_borrow_return_path_hint(sig, *method.fn);

        current_module_name_ = previous_module;

        ImplMethodInfo specialized = method;
        specialized.receiver_type = receiver_type;
        specialized.substitutions = std::move(substitutions);
        specialized.sig = std::move(sig);
        specialized.lowered_name = specialized_impl_method_lowered_name(
            specialized,
            receiver_type,
            method_name,
            specialized.substitutions);
        functions_.emplace(specialized.lowered_name, specialized.sig);
        return specialized;
    }

    ImplMethodInfo specialize_generic_impl_method_with_substitutions(
        const ImplMethodInfo& method,
        const IrType& receiver_type,
        const std::string& method_name,
        std::map<std::string, IrType> substitutions
    ) {
        substitutions["Self"] = receiver_type;

        std::string previous_module = current_module_name_;
        current_module_name_ = method.module_name;

        FunctionSig sig;
        sig.loc = method.fn->loc;
        sig.module_name = method.module_name;
        sig.is_public = method.is_public;
        for (const auto& param : method.fn->params) {
            IrType param_type = resolve_type_with_substitutions(param.type, substitutions);
            bool vec_view = false;
            sig.params.push_back(function_parameter_abi_type(
                param.type.loc,
                param_type,
                "an impl method parameter",
                vec_view));
        }
        sig.result = method.fn->has_return_type
            ? resolve_type_with_substitutions(method.fn->return_type, substitutions)
            : void_type(method.fn->loc);
        if (method.fn->has_return_type) {
            require_root_vector_runtime_abi(
                method.fn->return_type.loc,
                sig.result,
                "an impl method return type");
        }
        set_function_return_contracts(sig);
        apply_explicit_borrow_return_contract(sig, *method.fn);
        set_function_borrow_return_path_hint(sig, *method.fn);

        current_module_name_ = previous_module;

        ImplMethodInfo specialized = method;
        specialized.receiver_type = receiver_type;
        specialized.substitutions = std::move(substitutions);
        specialized.sig = std::move(sig);
        specialized.lowered_name = specialized_impl_method_lowered_name(
            specialized,
            receiver_type,
            method_name,
            specialized.substitutions);
        functions_.emplace(specialized.lowered_name, specialized.sig);
        return specialized;
    }

    bool try_select_generic_method_impl(
        const Expr& expr,
        const IrType& receiver_type,
        const std::string& generic_origin,
        const std::vector<IrType>& arg_types,
        ImplMethodInfo& selected
    ) {
        std::vector<const ImplMethodInfo*> matches;
        std::string first_bound_failure;
        std::string first_inference_failure;
        bool saw_wrong_arg_count = false;
        for (const auto& candidate : generic_method_impls_) {
            const std::string method_name = expr.name;
            if (basename_of_qualified_name(candidate.fn->name) != method_name) continue;
            std::map<std::string, IrType> substitutions;
            if (!infer_generic_impl_method_substitutions(candidate, receiver_type, substitutions)) continue;
            if (candidate.sig.params.size() != arg_types.size() + 1) {
                saw_wrong_arg_count = true;
                continue;
            }
            if (!bind_or_infer_method_generic_type_args(expr, candidate, arg_types, substitutions, &first_inference_failure)) {
                continue;
            }
            std::set<std::string> visiting;
            std::string bound_failure;
            if (!impl_generic_bounds_satisfied(candidate.generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }
            if (!impl_generic_bounds_satisfied(candidate.method_generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }
            if (!generic_origin.empty()) {
                bool has_applicable_bound = false;
                bool matches_bound = false;
                for (const auto& bound : current_generic_bounds_) {
                    if (bound.generic_name != generic_origin) continue;
                    if (!trait_application_has_method(bound.trait_name, bound.trait_args, method_name)) continue;
                    has_applicable_bound = true;
                    if (candidate.trait_name != bound.trait_name) {
                        if (trait_application_implies_trait(
                                bound.trait_name,
                                bound.trait_args,
                                candidate.trait_name,
                                candidate.trait_args)) {
                            matches_bound = true;
                            break;
                        }
                        continue;
                    }
                    if (candidate.trait_args.size() != bound.trait_args.size()) continue;
                    std::map<std::string, IrType> trait_substitutions = substitutions;
                    bool trait_args_match = true;
                    for (std::size_t i = 0; i < bound.trait_args.size(); ++i) {
                        if (!infer_generic_pattern_type(
                                candidate.trait_args[i],
                                bound.trait_args[i],
                                candidate.generic_names,
                                trait_substitutions)) {
                            trait_args_match = false;
                            break;
                        }
                    }
                    if (trait_args_match) {
                        matches_bound = true;
                        break;
                    }
                }
                if (has_applicable_bound && !matches_bound) continue;
            }
            matches.push_back(&candidate);
        }

        if (matches.empty()) {
            if (!first_bound_failure.empty()) fail(expr.loc, first_bound_failure);
            if (!first_inference_failure.empty()) fail(expr.loc, first_inference_failure);
            if (saw_wrong_arg_count) fail(expr.loc, "wrong argument count for method '" + expr.name + "'");
            return false;
        }
        if (matches.size() > 1) {
            fail(expr.loc, "method call '" + expr.name + "' for type " + type_name(receiver_type) + " is ambiguous");
        }
        std::map<std::string, IrType> substitutions;
        if (!infer_generic_impl_method_substitutions(*matches.front(), receiver_type, substitutions)) {
            fail(expr.loc, "generic impl method '" + expr.name + "' cannot be specialized for " + type_name(receiver_type));
        }
        if (!bind_or_infer_method_generic_type_args(expr, *matches.front(), arg_types, substitutions, nullptr)) {
            fail(expr.loc, "generic method '" + expr.name + "' could not be specialized");
        }
        selected = specialize_generic_impl_method_with_substitutions(
            *matches.front(),
            receiver_type,
            expr.name,
            std::move(substitutions));
        return true;
    }

    bool associated_receiver_name_matches_type(SourceLocation loc, const std::string& receiver_name, const IrType& type) {
        if (type.primitive == IrPrimitiveKind::Struct) {
            return resolve_struct_type_name(receiver_name) == type.name;
        }
        if (type.primitive == IrPrimitiveKind::Vector) {
            std::string base = unqualified_name(receiver_name);
            return base == "Vec";
        }
        if (type.primitive == IrPrimitiveKind::Enum) {
            return resolve_enum_type_name(receiver_name) == type.name;
        }

        IrType resolved;
        if (!try_resolve_associated_receiver_type(loc, receiver_name, resolved)) return false;
        IrType pattern = type;
        pattern.args.clear();
        pattern.field_names.clear();
        pattern.field_types.clear();
        pattern.field_mutable.clear();
        return same_type(resolved, pattern);
    }

    void bind_method_generic_type_args(
        SourceLocation loc,
        const ImplMethodInfo& method,
        const std::vector<TypeRef>& type_args,
        std::map<std::string, IrType>& substitutions
    ) {
        if (type_args.size() != method.method_generic_names.size()) {
            fail(loc,
                 "generic method '" + basename_of_qualified_name(method.fn->name) +
                     "' expects " + std::to_string(method.method_generic_names.size()) +
                     " type argument" + (method.method_generic_names.size() == 1 ? "" : "s"));
        }
        for (std::size_t i = 0; i < type_args.size(); ++i) {
            bind_generic_type(
                type_args[i].loc,
                method.method_generic_names[i],
                resolve_executable_type(type_args[i]),
                substitutions);
        }
    }

    bool bind_or_infer_method_generic_type_args(
        const Expr& expr,
        const ImplMethodInfo& method,
        const std::vector<IrType>& arg_types,
        std::map<std::string, IrType>& substitutions,
        std::string* failure
    ) {
        const ExprTypeArgs& type_args = expr_type_args(expr);
        if (method.method_generic_names.empty()) {
            if (!type_args.empty()) {
                fail(expr.loc, "method '" + expr.name + "' does not take type arguments");
            }
            return true;
        }

        if (!type_args.empty()) {
            bind_method_generic_type_args(expr.loc, method, type_args, substitutions);
        } else {
            for (std::size_t i = 0; i < arg_types.size(); ++i) {
                infer_named_generic_type(
                    expr.loc,
                    method.fn->params[i + 1].type,
                    arg_types[i],
                    method.method_generic_names,
                    substitutions);
            }
        }

        for (const auto& generic_name : method.method_generic_names) {
            if (!substitutions.count(generic_name)) {
                if (failure) {
                    *failure = "generic type '" + generic_name +
                        "' could not be inferred for method '" + expr.name + "'";
                }
                return false;
            }
        }
        return true;
    }

    bool bind_or_infer_associated_generic_type_args(
        const Expr& expr,
        const ImplMethodInfo& method,
        const std::vector<IrType>& arg_types,
        std::map<std::string, IrType>& substitutions,
        std::string* failure
    ) {
        substitutions.clear();
        const std::size_t impl_count = method.generic_names.size();
        const std::size_t method_count = method.method_generic_names.size();
        const std::size_t total_count = impl_count + method_count;
        const ExprTypeArgs& type_args = expr_type_args(expr);
        if (type_args.size() > total_count) {
            fail(expr.loc,
                 "generic associated function expects " + std::to_string(total_count) +
                     " type argument" + (total_count == 1 ? "" : "s"));
        }

        std::size_t type_arg_index = 0;
        for (; type_arg_index < type_args.size() && type_arg_index < impl_count; ++type_arg_index) {
            bind_generic_type(
                type_args[type_arg_index].loc,
                method.generic_names[type_arg_index],
                resolve_executable_type(type_args[type_arg_index]),
                substitutions);
        }
        for (; type_arg_index < type_args.size(); ++type_arg_index) {
            std::size_t method_index = type_arg_index - impl_count;
            bind_generic_type(
                type_args[type_arg_index].loc,
                method.method_generic_names[method_index],
                resolve_executable_type(type_args[type_arg_index]),
                substitutions);
        }

        std::vector<std::string> all_generic_names = method.generic_names;
        all_generic_names.insert(
            all_generic_names.end(),
            method.method_generic_names.begin(),
            method.method_generic_names.end());
        for (std::size_t i = 0; i < arg_types.size(); ++i) {
            infer_named_generic_type(
                expr.loc,
                method.fn->params[i].type,
                arg_types[i],
                all_generic_names,
                substitutions);
        }

        for (const auto& generic_name : all_generic_names) {
            if (!substitutions.count(generic_name)) {
                if (failure) {
                    *failure = "generic type '" + generic_name +
                        "' could not be inferred for associated function '" +
                        basename_of_qualified_name(method.fn->name) + "'";
                }
                return false;
            }
        }
        return true;
    }

    bool try_select_generic_associated_impl(
        const Expr& expr,
        const std::string& receiver_name,
        const std::string& method_name,
        const std::vector<IrType>& arg_types,
        ImplMethodInfo& selected
    ) {
        std::vector<ImplMethodInfo> matches;
        std::string first_bound_failure;
        std::string first_inference_failure;
        bool saw_wrong_arg_count = false;
        for (const auto& candidate : generic_associated_impls_) {
            if (basename_of_qualified_name(candidate.fn->name) != method_name) continue;
            if (!associated_receiver_name_matches_type(expr.loc, receiver_name, candidate.receiver_type)) continue;
            if (candidate.sig.params.size() != arg_types.size()) {
                saw_wrong_arg_count = true;
                continue;
            }

            std::map<std::string, IrType> substitutions;
            if (!bind_or_infer_associated_generic_type_args(expr, candidate, arg_types, substitutions, &first_inference_failure)) {
                continue;
            }
            IrType receiver_type = substitute_inferred_type(candidate.receiver_type, substitutions);
            substitutions["Self"] = receiver_type;

            std::set<std::string> visiting;
            std::string bound_failure;
            if (!impl_generic_bounds_satisfied(candidate.generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }
            if (!impl_generic_bounds_satisfied(candidate.method_generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }
            matches.push_back(specialize_generic_impl_method_with_substitutions(
                candidate,
                receiver_type,
                method_name,
                std::move(substitutions)
            ));
        }

        if (matches.empty()) {
            if (!first_bound_failure.empty()) fail(expr.loc, first_bound_failure);
            if (!first_inference_failure.empty()) fail(expr.loc, first_inference_failure);
            if (saw_wrong_arg_count) fail(expr.loc, "wrong argument count for associated function '" + method_name + "'");
            return false;
        }
        if (matches.size() > 1) {
            fail(expr.loc, "associated function '" + method_name + "' for type '" + receiver_name + "' is ambiguous");
        }
        selected = std::move(matches.front());
        return true;
    }

    void require_generic_bounds(
        SourceLocation loc,
        const FunctionDecl& fn,
        const std::map<std::string, IrType>& substitutions
    ) {
        bool has_constraint = false;
        for (const auto& generic : fn.generics) {
            if (generic.has_constraint) {
                has_constraint = true;
                break;
            }
        }
        if (!has_constraint) return;

        std::string previous_module = current_module_name_;
        std::map<std::string, IrType> previous_substitutions = std::move(current_type_substitutions_);
        current_module_name_ = fn.module_name;
        current_type_substitutions_ = substitutions;
        for (const auto& generic : fn.generics) {
            if (!generic.has_constraint) continue;
            auto self_found = substitutions.find(generic.name);
            if (self_found == substitutions.end()) continue;
            GenericTraitBound bound = resolve_generic_trait_bound(generic);
            if (!type_implements_trait(bound.trait_name, bound.trait_args, self_found->second)) {
                fail(loc,
                     "type " + type_name(self_found->second) + " does not implement trait '" +
                         trait_application_display(bound.trait_name, bound.trait_args) +
                         "' required by generic parameter '" + generic.name + "'");
            }
        }
        current_type_substitutions_ = std::move(previous_substitutions);
        current_module_name_ = previous_module;
    }

    const FunctionDecl* find_generic_function(const Expr& expr, std::string& resolved_name) const {
        resolved_name = resolve_generic_function_name(expr.name);
        auto found = generic_functions_.find(resolved_name);
        if (found != generic_functions_.end()) return found->second;
        return nullptr;
    }

    std::uint32_t require_enum_case_tag(SourceLocation loc,
                                        const IrType& enum_type,
                                        const std::string& case_name) const {
        auto found = enum_cases_.find(case_name);
        if (found == enum_cases_.end() || found->second.enum_name != enum_type.name) {
            fail(loc, "internal std enum probe helper expected case '" + case_name +
                      "' for " + type_name(enum_type));
        }
        return found->second.tag;
    }

    IrExprPtr check_std_option_result_probe_helper_call(SourceLocation loc,
                                                        const std::string& resolved_name,
                                                        std::vector<IrExprPtr> args,
                                                        std::size_t borrow_mark) {
        if (args.size() != 1) {
            fail(loc, "std enum probe helper expects one borrowed enum value");
        }
        if (!is_borrow_type(args[0]->type)) {
            fail(loc, "std enum probe helper expects a borrowed enum value, got " + type_name(args[0]->type));
        }

        IrType enum_type = value_qualified_type(args[0]->type);
        std::optional<StdEnumProbeHelper> helper = std_enum_probe_helper_for_name(resolved_name);
        if (!helper) {
            throw CompileError("internal error: unknown std enum probe helper '" + resolved_name + "'");
        }
        if (enum_type.name != helper->enum_name || enum_type.primitive != IrPrimitiveKind::Enum) {
            fail(loc, "std enum probe helper expected " + helper->enum_name + ", got " + type_name(enum_type));
        }

        std::uint32_t tag = require_enum_case_tag(loc, enum_type, helper->case_name);
        IrExprPtr tag_expr = make_enum_tag_expr(loc, std::move(args[0]));
        IrExprPtr expected = make_integer_literal(loc, i64_type(loc), tag);
        release_temporary_borrows(borrow_mark);
        return make_bool_binary_expr(loc, IrBinaryOp::Eq, std::move(tag_expr), std::move(expected));
    }

    IrExprPtr check_assert_compare_macro(const Expr& expr, PreludeMacroKind kind, std::vector<ExprPtr> args) {
        if (args.size() != 2) fail(expr.loc, "wrong argument count for '" + expr.name + "!'");

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr left = check_expr(*args[0]);
        IrExprPtr right = check_expr(*args[1]);
        release_temporary_borrows(borrow_mark);

        std::string function_name;
        std::vector<IrExprPtr> lowered_args;
        lowered_args.reserve(2);
        if (left->type.primitive == IrPrimitiveKind::Bool && right->type.primitive == IrPrimitiveKind::Bool &&
            left->type.qualifier == TypeQualifier::Value && right->type.qualifier == TypeQualifier::Value) {
            function_name = kind == PreludeMacroKind::AssertEq ? "assert_eq_bool" : "assert_ne_bool";
            lowered_args.push_back(std::move(left));
            lowered_args.push_back(std::move(right));
        } else if (is_value_integer_type(left->type) && is_value_integer_type(right->type)) {
            IrType wide = i64_type(expr.loc);
            function_name = kind == PreludeMacroKind::AssertEq ? "assert_eq_i64" : "assert_ne_i64";
            lowered_args.push_back(make_cast_expr(expr.loc, std::move(left), wide));
            lowered_args.push_back(make_cast_expr(expr.loc, std::move(right), wide));
        } else {
            fail(expr.loc, expr.name + "! arguments must both be integer values or both be bool values");
        }

        return make_builtin_call(expr.loc, function_name, std::move(lowered_args), i64_type(expr.loc));
    }

    static TokenKind macro_matching_delimiter(TokenKind kind) {
        if (kind == TokenKind::LParen) return TokenKind::RParen;
        if (kind == TokenKind::LBrace) return TokenKind::RBrace;
        if (kind == TokenKind::LBracket) return TokenKind::RBracket;
        return TokenKind::End;
    }

    static bool macro_is_closing_delimiter(TokenKind kind) {
        return kind == TokenKind::RParen || kind == TokenKind::RBrace || kind == TokenKind::RBracket;
    }

    static std::vector<std::vector<Token>> split_macro_top_level_arguments(
        const std::vector<Token>& tokens,
        SourceLocation loc,
        const std::string& macro_name
    ) {
        std::vector<std::vector<Token>> args(1);
        std::vector<TokenKind> closing_stack;
        for (const Token& token : tokens) {
            if (closing_stack.empty() && token.kind == TokenKind::Comma) {
                args.emplace_back();
                continue;
            }
            if (!closing_stack.empty() && token.kind == closing_stack.back()) {
                args.back().push_back(token);
                closing_stack.pop_back();
                continue;
            }
            TokenKind matching = macro_matching_delimiter(token.kind);
            if (matching != TokenKind::End) {
                args.back().push_back(token);
                closing_stack.push_back(matching);
                continue;
            }
            if (macro_is_closing_delimiter(token.kind)) {
                fail(token.loc, "mismatched delimiter in " + macro_name + "! invocation");
            }
            args.back().push_back(token);
        }
        if (!closing_stack.empty()) fail(loc, "unterminated " + macro_name + "! invocation");
        return args;
    }

    static void require_non_empty_macro_arguments(
        const std::vector<std::vector<Token>>& args,
        SourceLocation loc,
        const std::string& message
    ) {
        for (const auto& arg : args) {
            if (arg.empty()) fail(loc, message);
        }
    }

    IrExprPtr check_vec_macro_call(const Expr& expr) {
        if (!expr.macro_tokens) {
            fail(expr.loc, "macro invocation '" + expr.name + "!' is missing token payload");
        }
        std::string usage = "Vec! expects Vec!(T, ref mut zone, capacity)";
        std::vector<std::vector<Token>> parts =
            split_macro_top_level_arguments(*expr.macro_tokens, expr.loc, expr.name);
        if (parts.size() != 3) fail(expr.loc, usage);
        require_non_empty_macro_arguments(parts, expr.loc, usage);

        TypeRef element_type = parse_macro_type_ref(std::move(parts[0]), expr.loc);
        std::vector<ExprPtr> args;
        args.reserve(2);
        args.push_back(parse_macro_expression(std::move(parts[1]), expr.loc));
        args.push_back(parse_macro_expression(std::move(parts[2]), expr.loc));

        Expr call;
        call.kind = ExprKind::Call;
        call.loc = expr.loc;
        call.name = "std::vec::new";
        call.args = std::move(args);
        set_expr_type_args(call, {std::move(element_type)});

        auto lowered = std::make_unique<IrExpr>();
        lowered->loc = expr.loc;
        return check_call(call, std::move(lowered));
    }

    IrExprPtr check_prelude_macro_call(const Expr& expr, PreludeMacroKind kind) {
        if (kind == PreludeMacroKind::Format) {
            fail(expr.loc, "prelude macro 'format!' has no implicit allocation zone; use format_in!(ref mut zone, ...) for explicit-zone strings");
        }
        if (kind == PreludeMacroKind::FormatIn) {
            if (!expr.macro_tokens) {
                fail(expr.loc, "macro invocation '" + expr.name + "!' is missing token payload");
            }
            return check_format_in_macro(expr, parse_macro_argument_expressions(*expr.macro_tokens, expr.loc));
        }
        if (kind == PreludeMacroKind::Matches) {
            fail(expr.loc, "prelude macro 'matches!' needs pattern-position macro expansion before it can be lowered");
        }
        if (kind == PreludeMacroKind::Vec) {
            return check_vec_macro_call(expr);
        }
        if (!is_supported_prelude_macro(kind)) {
            fail(expr.loc, "prelude macro '" + expr.name + "!' is reserved but macro expansion is not supported yet");
        }

        if (!expr.macro_tokens) {
            fail(expr.loc, "macro invocation '" + expr.name + "!' is missing token payload");
        }
        std::vector<ExprPtr> args = parse_macro_argument_expressions(*expr.macro_tokens, expr.loc);
        if (kind == PreludeMacroKind::AssertEq || kind == PreludeMacroKind::AssertNe) {
            return check_assert_compare_macro(expr, kind, std::move(args));
        }

        Expr call;
        call.kind = ExprKind::Call;
        call.loc = expr.loc;
        call.name = prelude_macro_function_name(kind);
        call.args = std::move(args);
        auto lowered = std::make_unique<IrExpr>();
        lowered->loc = expr.loc;
        return check_call(call, std::move(lowered));
    }

    IrExprPtr check_macro_call(const Expr& expr) {
        std::string prelude_name = resolve_use_path(expr.name);
        bool local_decl_shadows_prelude = unqualified_decl_shadows_prelude_name(expr.name, prelude_name);
        if (prelude_specials_available() && !local_decl_shadows_prelude) {
            PreludeMacroKind prelude = prelude_macro_kind_for_resolved_name(prelude_name);
            if (prelude != PreludeMacroKind::None) return check_prelude_macro_call(expr, prelude);
        }

        const MetaFunctionInfo& meta = require_meta_invocation(expr.loc, MetaInvocationSite::ExpressionMacro, expr.name);
        if (!expr.macro_tokens) {
            fail(expr.loc, "macro invocation '" + expr.name + "!' is missing token payload");
        }
        if (meta.token_return) {
            ExprPtr expanded = expand_expression_macro_token_return(
                *expr.macro_tokens, expr.loc, meta.parameter_name, *meta.token_return);
            return check_expr(*expanded);
        }
        ExprPtr expanded = parse_macro_expression(*expr.macro_tokens, expr.loc);
        if (meta.ast_return) {
            expanded = expand_ast_expression_return(*meta.ast_return, meta.parameter_name, *expanded, expr.loc);
        }
        return check_expr(*expanded);
    }

    IrExprPtr check_generic_call(const Expr& expr, const FunctionDecl& fn, const std::string& resolved_name, IrExprPtr lowered) {
        (void)lowered;
        require_function_decl_access(expr.loc, fn, resolved_name);
        if (const Attribute* deprecated = deprecated_attribute(fn.attributes)) {
            warn_deprecated_use(expr.loc, "function", resolved_name, deprecated_message(fn.attributes));
            (void)deprecated;
        }
        if (fn.params.size() != expr.args.size()) fail(expr.loc, "wrong argument count for '" + expr.name + "'");
        if (expr.args.size() > std::numeric_limits<std::uint16_t>::max()) {
            fail(expr.loc, "function calls support up to 65535 arguments");
        }

        std::vector<IrExprPtr> args;
        args.reserve(expr.args.size());
        std::size_t borrow_mark = temporary_borrow_mark();
        for (const auto& arg_expr : expr.args) args.push_back(check_expr(*arg_expr));

        std::map<std::string, IrType> substitutions;
        const ExprTypeArgs& type_args = expr_type_args(expr);
        if (!type_args.empty()) {
            if (type_args.size() != fn.generics.size()) {
                fail(expr.loc,
                     "generic function '" + expr.name + "' expects " + std::to_string(fn.generics.size()) +
                         " type argument" + (fn.generics.size() == 1 ? "" : "s"));
            }
            for (std::size_t i = 0; i < type_args.size(); ++i) {
                bind_generic_type(
                    type_args[i].loc,
                    fn.generics[i].name,
                    resolve_executable_type(type_args[i]),
                    substitutions
                );
            }
        }
        for (std::size_t i = 0; i < fn.params.size(); ++i) {
            infer_generic_type(expr.loc, fn.params[i].type, args[i]->type, fn.generics, substitutions);
        }
        for (const auto& generic : fn.generics) {
            if (!substitutions.count(generic.name)) {
                fail(expr.loc, "generic type '" + generic.name + "' could not be inferred for '" + expr.name + "'");
            }
        }
        require_generic_bounds(expr.loc, fn, substitutions);

        std::vector<IrType> param_types;
        param_types.reserve(fn.params.size());
        for (std::size_t i = 0; i < fn.params.size(); ++i) {
            IrType source_param_type = resolve_type_with_substitutions(fn.params[i].type, substitutions);
            bool vec_view = false;
            IrType param_type = function_parameter_abi_type(
                fn.params[i].type.loc,
                source_param_type,
                "a function parameter",
                vec_view);
            if (vec_view) {
                IrExprPtr slice_arg = try_make_implicit_slice_argument(*expr.args[i], param_type);
                if (!slice_arg) {
                    fail(expr.args[i]->loc,
                         "Vec parameter arguments currently require a local array or Vec binding; bind the value to a local first");
                }
                args[i] = std::move(slice_arg);
            } else {
                coerce_expr_to_expected(*args[i], param_type);
            }
            require_assignable(expr.loc, param_type, args[i]->type);
            param_types.push_back(param_type);
        }
        IrType result = fn.has_return_type ? resolve_type_with_substitutions(fn.return_type, substitutions) : void_type(fn.loc);
        require_function_signature_root_vector_runtime_abi(fn, param_types, result);

        std::string specialized_name = generic_specialization_name(fn, substitutions);
        FunctionSig call_sig;
        call_sig.params = param_types;
        call_sig.result = result;
        call_sig.module_name = fn.module_name;
        call_sig.is_public = fn.is_public;
        call_sig.loc = fn.loc;
        set_function_return_contracts(call_sig);
        apply_explicit_borrow_return_contract(call_sig, fn);
        set_function_borrow_return_path_hint(call_sig, fn);

        if (std_enum_probe_helper_for_name(resolved_name)) {
            return check_std_option_result_probe_helper_call(
                expr.loc,
                resolved_name,
                std::move(args),
                borrow_mark);
        }

        if (!queued_specializations_.count(specialized_name)) {
            functions_.emplace(specialized_name, call_sig);
            pending_specializations_.push_back(PendingSpecialization{&fn, specialized_name, substitutions});
            queued_specializations_.insert(specialized_name);
        }
        return finish_tracked_call(
            expr.loc,
            expr.name,
            std::move(specialized_name),
            call_sig,
            std::move(args),
            borrow_mark);
    }

    void queue_generic_function_specialization(
        const FunctionDecl& fn,
        const std::string& specialized_name,
        std::vector<IrType> param_types,
        const IrType& result,
        const std::map<std::string, IrType>& substitutions
    ) {
        if (queued_specializations_.count(specialized_name)) return;

        FunctionSig sig;
        sig.params = std::move(param_types);
        sig.result = result;
        require_function_signature_root_vector_runtime_abi(fn, sig.params, sig.result);
        sig.module_name = fn.module_name;
        sig.is_public = fn.is_public;
        sig.loc = fn.loc;
        set_function_return_contracts(sig);
        apply_explicit_borrow_return_contract(sig, fn);
        set_function_borrow_return_path_hint(sig, fn);
        functions_.emplace(specialized_name, std::move(sig));
        pending_specializations_.push_back(PendingSpecialization{&fn, specialized_name, substitutions});
        queued_specializations_.insert(specialized_name);
    }

    IrExprPtr check_generic_function_ref_with_expected(
        const Expr& expr,
        const FunctionDecl& fn,
        const std::string& resolved_name,
        const IrType& expected
    ) {
        if (expected.qualifier != TypeQualifier::Value ||
            expected.primitive != IrPrimitiveKind::Function ||
            expected.args.empty() ||
            expected.array_size + 1 != expected.args.size()) {
            return nullptr;
        }

        require_function_decl_access(expr.loc, fn, resolved_name);
        if (const Attribute* deprecated = deprecated_attribute(fn.attributes)) {
            warn_deprecated_use(expr.loc, "function", resolved_name, deprecated_message(fn.attributes));
            (void)deprecated;
        }

        std::size_t param_count = static_cast<std::size_t>(expected.array_size);
        if (fn.params.size() != param_count) {
            fail(expr.loc,
                 "generic function '" + expr.name + "' cannot be used as " + type_name(expected) +
                     ": parameter count mismatch");
        }

        std::map<std::string, IrType> substitutions;
        for (std::size_t i = 0; i < fn.params.size(); ++i) {
            infer_generic_type(expr.loc, fn.params[i].type, expected.args[i], fn.generics, substitutions);
        }

        IrType expected_result = function_pointer_result_type(expected);
        if (fn.has_return_type) {
            infer_generic_type(expr.loc, fn.return_type, expected_result, fn.generics, substitutions);
        }
        for (const auto& generic : fn.generics) {
            if (!substitutions.count(generic.name)) {
                fail(expr.loc,
                     "generic type '" + generic.name +
                         "' could not be inferred for function pointer '" + expr.name + "'");
            }
        }
        require_generic_bounds(expr.loc, fn, substitutions);

        std::vector<IrType> param_types;
        param_types.reserve(fn.params.size());
        for (const auto& param : fn.params) {
            IrType source_param_type = resolve_type_with_substitutions(param.type, substitutions);
            bool vec_view = false;
            param_types.push_back(function_parameter_abi_type(
                param.type.loc,
                source_param_type,
                "a function pointer parameter",
                vec_view));
        }
        IrType result = fn.has_return_type
            ? resolve_type_with_substitutions(fn.return_type, substitutions)
            : void_type(fn.loc);

        FunctionSig selected_sig;
        selected_sig.params = param_types;
        selected_sig.result = result;
        IrType selected_type = function_pointer_type(selected_sig, expr.loc);
        if (!same_type(selected_type, expected)) {
            fail(expr.loc,
                 "generic function '" + expr.name + "' specializes to " +
                     type_name(selected_type) + ", expected " + type_name(expected));
        }

        std::string specialized_name = generic_specialization_name(fn, substitutions);
        queue_generic_function_specialization(fn, specialized_name, std::move(param_types), result, substitutions);

        return make_function_ref_expr(expr.loc, specialized_name, selected_type);
    }

    void queue_impl_method_for_lowering(const ImplMethodInfo& method) {
        if (!queued_impl_methods_.count(method.lowered_name)) {
            impl_methods_to_lower_.push_back(method);
            queued_impl_methods_.insert(method.lowered_name);
        }
    }

    IrExprPtr check_range_call(const Expr& expr,
                               IrExprPtr lowered,
                               const IrType* expected_range = nullptr,
                               const std::string& call_name = "") {
        (void)lowered;
        if (!expr_type_args(expr).empty()) {
            fail(expr.loc, "range constructors do not take type arguments");
        }
        if (expr.args.size() != 2) fail(expr.loc, "range expects start and end values");
        const std::string& range_name = call_name.empty() ? expr.name : call_name;

        IrType bound = i64_type(expr.loc);
        bool has_bound = expected_range &&
                         is_prelude_range_type(*expected_range) &&
                         !expected_range->args.empty();
        if (has_bound) {
            bound = expected_range->args[0];
        } else if (expr.args[0]->kind == ExprKind::Integer && !expr.args[0]->literal_suffix.empty()) {
            bound = integer_literal_suffix_type(expr.args[0]->literal_suffix, expr.args[0]->loc);
            has_bound = true;
        } else if (expr.args[1]->kind == ExprKind::Integer && !expr.args[1]->literal_suffix.empty()) {
            bound = integer_literal_suffix_type(expr.args[1]->literal_suffix, expr.args[1]->loc);
            has_bound = true;
        }

        IrType range_type = make_prelude_range_type(
            expr.loc,
            is_prelude_inclusive_range_function_name(range_name),
            bound
        );
        IrExprPtr start = has_bound
            ? check_expr_with_expected(*expr.args[0], bound)
            : check_expr(*expr.args[0]);
        if (!has_bound) {
            if (!is_value_integer_type(start->type)) {
                fail(expr.args[0]->loc, "range bounds must be integers");
            }
            bound = start->type;
            range_type = make_prelude_range_type(
                expr.loc,
                is_prelude_inclusive_range_function_name(range_name),
                bound
            );
        }
        coerce_expr_to_expected(*start, bound);
        require_assignable(expr.args[0]->loc, bound, start->type);

        IrExprPtr end = check_expr_with_expected(*expr.args[1], bound);
        coerce_expr_to_expected(*end, bound);
        require_assignable(expr.args[1]->loc, bound, end->type);

        std::vector<IrExprPtr> args;
        args.reserve(2);
        args.push_back(std::move(start));
        args.push_back(std::move(end));
        return make_ir_tuple_expr(expr.loc, std::move(range_type), std::move(args));
    }

    IrExprPtr check_explicit_move_call(const Expr& expr, IrExprPtr lowered, bool require_place) {
        (void)lowered;
        const char* helper_name = require_place ? "take" : "move";
        if (expr.args.size() != 1) {
            fail(expr.loc, std::string(helper_name) + " expects one value");
        }
        const ExprTypeArgs& type_args = expr_type_args(expr);
        if (type_args.size() > 1) {
            fail(expr.loc, std::string(helper_name) + " expects at most one type argument");
        }
        if (require_place && !is_take_place_expression(*expr.args[0])) {
            fail(expr.args[0]->loc, take_place_expectation());
        }

        std::optional<IrType> expected;
        if (!type_args.empty()) {
            expected = resolve_executable_type(type_args.front());
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = expected
            ? check_expr_with_expected(*expr.args[0], *expected)
            : check_expr(*expr.args[0]);
        if (expected) {
            coerce_expr_to_expected(*value, *expected);
            require_assignable(expr.args[0]->loc, *expected, value->type);
        }
        if (contains_borrow_type(value->type)) {
            fail(expr.args[0]->loc,
                 std::string(helper_name) + " cannot return borrow-valued expressions");
        }
        release_temporary_borrows(borrow_mark);
        return value;
    }

    LocalInfo* slice_view_local_method_receiver(const Expr& method_expr) {
        if (!expr_operand(method_expr) || expr_operand(method_expr)->kind != ExprKind::Name) return nullptr;
        LocalInfo* local = find_local_slot(expr_operand(method_expr)->name);
        if (!local) return nullptr;
        if (local->type.primitive != IrPrimitiveKind::Array &&
            local->type.primitive != IrPrimitiveKind::Vector) {
            return nullptr;
        }
        if (local->type.args.size() != 1) return nullptr;
        return local;
    }

    void require_slice_view_receiver(SourceLocation loc,
                                     const std::string& name,
                                     const LocalInfo& local) const {
        if (auto error = local_unavailable_binding_error(name, local)) fail(loc, *error);
        if (auto error = local_method_mutability_error(name, local, "as_slice")) fail(loc, *error);
        if (contains_borrow_type(local.type) || is_owner_type(local.type.args[0])) {
            fail(loc, "as_slice currently supports copyable element arrays and vectors only");
        }
        require_slice_element_materializable(loc, local.type.args[0], "Slice.as_slice");
        require_not_borrowed(loc, name, local, "create Slice from");
    }

    IrExprPtr check_slice_view_method_call(const Expr& expr, IrExprPtr lowered, const LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_slice_view_receiver(expr.loc, name, local);
        require_slice_view_method_shape(expr.loc, expr_type_args(expr).size(), expr.args.size());

        const IrType& element = local.type.args[0];
        IrExprPtr data;
        IrExprPtr length;
        if (local.type.primitive == IrPrimitiveKind::Array) {
            data = make_slice_data_pointer_expr(
                expr.loc,
                make_local_lvalue_expr(expr_operand(expr)->loc, name, local.type),
                element
            );
            length = make_integer_literal(expr.loc, i64_type(expr.loc), local.type.array_size);
        } else {
            data = make_slice_data_pointer_expr(
                expr.loc,
                make_vec_storage_lvalue_expr(expr_operand(expr)->loc, name, local.type),
                element
            );
            length = make_local_vec_len_expr(
                expr.loc,
                make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
                local_vector_known_length(local)
            );
        }
        return make_slice_view_expr(
            expr.loc,
            std::move(data),
            std::move(length),
            make_prelude_slice_type(expr.loc, element)
        );
    }

    IrExprPtr make_slice_view_from_local_binding(SourceLocation loc,
                                                 SourceLocation receiver_loc,
                                                 const std::string& name,
                                                 const LocalInfo& local,
                                                 const IrType& slice_type) {
        if (!is_prelude_slice_type(slice_type) || slice_type.args.size() != 1) {
            fail(loc, "internal error: implicit Slice argument requires a Slice[T] parameter");
        }
        if (local.type.primitive != IrPrimitiveKind::Array &&
            local.type.primitive != IrPrimitiveKind::Vector) {
            fail(loc, "implicit Slice argument requires a local array or Vec binding");
        }
        if (local.type.args.size() != 1 || !same_type(slice_type.args[0], local.type.args[0])) {
            fail(loc, "type mismatch: expected " + type_name(slice_type) + ", got " + type_name(local.type));
        }
        if (contains_borrow_type(local.type) || is_owner_type(local.type.args[0])) {
            fail(loc, "implicit Slice arguments currently support copyable element arrays and vectors only");
        }
        require_slice_element_materializable(loc, local.type.args[0], "implicit Slice argument");
        require_not_borrowed(loc, name, local, "create Slice from");

        const IrType& element = local.type.args[0];
        IrExprPtr data;
        IrExprPtr length;
        if (local.type.primitive == IrPrimitiveKind::Array) {
            data = make_slice_data_pointer_expr(
                loc,
                make_local_lvalue_expr(receiver_loc, name, local.type),
                element);
            length = make_integer_literal(loc, i64_type(loc), local.type.array_size);
        } else {
            data = make_slice_data_pointer_expr(
                loc,
                make_vec_storage_lvalue_expr(receiver_loc, name, local.type),
                element);
            length = make_local_vec_len_expr(
                loc,
                make_vec_local_lvalue(receiver_loc, name, local.type),
                local_vector_known_length(local));
        }
        return make_slice_view_expr(loc, std::move(data), std::move(length), slice_type);
    }

    bool can_materialize_implicit_slice_temporary(const Expr& expr) const {
        switch (expr.kind) {
            case ExprKind::Vector:
                return true;
            case ExprKind::If:
                return expr_if_then_value(expr) &&
                       expr_if_else_value(expr) &&
                       can_materialize_implicit_slice_temporary(*expr_if_then_value(expr)) &&
                       can_materialize_implicit_slice_temporary(*expr_if_else_value(expr));
            case ExprKind::Block:
                return expr_block_value(expr) &&
                       can_materialize_implicit_slice_temporary(*expr_block_value(expr));
            case ExprKind::Match:
                if (expr_match_arms(expr).empty()) return false;
                for (const auto& arm : expr_match_arms(expr)) {
                    if (!arm.value || !can_materialize_implicit_slice_temporary(*arm.value)) return false;
                }
                return true;
            default:
                return false;
        }
    }

    IrExprPtr try_make_implicit_slice_argument(const Expr& arg_expr, const IrType& expected) {
        if (!is_prelude_slice_type(expected) || expected.args.size() != 1) return nullptr;
        if (arg_expr.kind == ExprKind::Name) {
            if (LocalInfo* local = find_local_slot(arg_expr.name)) {
                if (local->type.primitive == IrPrimitiveKind::Array ||
                    local->type.primitive == IrPrimitiveKind::Vector) {
                    if (auto error = local_unavailable_binding_error(arg_expr.name, *local)) {
                        fail(arg_expr.loc, *error);
                    }
                    require_can_read_borrow_path(arg_expr.loc, arg_expr.name, *local, "");
                    require_zone_pointer_valid(arg_expr.loc, arg_expr.name, *local);
                    copy_aggregate_borrow_sources_to_temporaries(arg_expr.loc, arg_expr.name, *local);
                    return make_slice_view_from_local_binding(
                        arg_expr.loc,
                        arg_expr.loc,
                        arg_expr.name,
                        *local,
                        expected);
                }
            }
            return nullptr;
        }
        if (!can_materialize_implicit_slice_temporary(arg_expr)) return nullptr;

        IrType vector_type = make_vector_storage_type(
            arg_expr.loc,
            expected.args[0],
            vector_storage_capacity_from_source_expr(arg_expr));
        IrExprPtr vector_value = check_expr_with_expected(arg_expr, vector_type);
        coerce_expr_to_expected(*vector_value, vector_type);
        require_assignable(arg_expr.loc, vector_type, vector_value->type);

        std::string temp_name = make_hidden_local("$slice_vec");
        declare_local(arg_expr.loc, temp_name, vector_type, false);
        IrStmtPtr temp_decl = make_ir_var_decl(
            arg_expr.loc,
            temp_name,
            vector_type,
            std::move(vector_value),
            false);
        IrStmt* temp_decl_ptr = temp_decl.get();

        LocalInfo& local = local_slot_by_name(temp_name);
        local.ir_storage_type = &temp_decl_ptr->binding.type;
        local.ir_init_expr = temp_decl_ptr->binding.init.get();
        set_local_vector_known_length(
            local,
            vector_known_length_from_source_expr(vector_type, arg_expr, *local.ir_init_expr));

        IrExprPtr slice_view = make_slice_view_from_local_binding(
            arg_expr.loc,
            arg_expr.loc,
            temp_name,
            local,
            expected);

        auto block = std::make_unique<IrExpr>();
        block->kind = IrExprKind::Block;
        block->loc = arg_expr.loc;
        block->type = expected;
        std::vector<IrStmtPtr> body;
        body.push_back(std::move(temp_decl));
        set_ir_expr_block_payload(*block, "", std::move(body), std::move(slice_view));
        return block;
    }

    IrExprPtr check_call_argument_for_expected(const Expr& arg_expr, const IrType& expected) {
        if (IrExprPtr slice_view = try_make_implicit_slice_argument(arg_expr, expected)) {
            return slice_view;
        }
        IrExprPtr arg = check_expr_with_expected(arg_expr, expected);
        coerce_expr_to_expected(*arg, expected);
        require_assignable(arg_expr.loc, expected, arg->type);
        return arg;
    }

    IrExprPtr coerce_checked_call_argument_or_implicit_slice(const Expr& arg_expr,
                                                             IrExprPtr checked,
                                                             const IrType& expected) {
        if (IrExprPtr slice_view = try_make_implicit_slice_argument(arg_expr, expected)) {
            return slice_view;
        }
        coerce_expr_to_expected(*checked, expected);
        require_assignable(arg_expr.loc, expected, checked->type);
        return checked;
    }

    IrExprPtr check_vec_len_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        require_collection_len_function_shape(expr.loc, expr_type_args(expr).size(), expr.args.size());
        return check_collection_len_expr(expr.loc, *expr.args[0]);
    }

    VectorKnownLength known_local_vec_length_for_expr(const Expr& expr) {
        if (expr.kind != ExprKind::Name) return {};
        const LocalInfo* local = find_local_slot(expr.name);
        if (!local || !is_vector_storage_type(local->type)) return {};
        return local_vector_known_length(*local);
    }

    VectorKnownLength known_local_vec_length_for_access(const TrackedAggregateAccess& access) {
        if (!access.path.empty()) return {};
        const LocalInfo* local = find_local_slot(access.base_name);
        if (!local || !is_vector_storage_type(local->type)) return {};
        return local_vector_known_length(*local);
    }

    IrExprPtr check_collection_len_expr(SourceLocation loc, const Expr& source) {
        IrExprPtr operand = check_aggregate_access_operand(source);
        VectorKnownLength known_vec_length =
            vector_known_length_from_source_expr(operand->type, source, *operand);
        return make_local_vec_len_expr(loc, std::move(operand), known_vec_length);
    }

    IrExprPtr check_collection_is_empty_method_call(const Expr& expr) {
        require_collection_is_empty_method_shape(expr.loc, expr_type_args(expr).size(), expr.args.size());
        IrExprPtr operand = check_aggregate_access_operand(*expr_operand(expr));
        VectorKnownLength known_vec_length =
            vector_known_length_from_source_expr(operand->type, *expr_operand(expr), *operand);
        return make_local_vec_is_empty_expr(expr.loc, std::move(operand), known_vec_length);
    }

    LocalInfo* vec_local_method_receiver(const Expr& method_expr) {
        if (!expr_operand(method_expr) || expr_operand(method_expr)->kind != ExprKind::Name) return nullptr;
        LocalInfo* local = find_local_slot(expr_operand(method_expr)->name);
        if (!local || !is_vector_storage_type(local->type)) {
            return nullptr;
        }
        return local;
    }

    bool is_local_vec_method_receiver(const Expr& method_expr) {
        if (!expr_operand(method_expr) || expr_operand(method_expr)->kind != ExprKind::Name) return false;
        LocalInfo* local = find_local_slot(expr_operand(method_expr)->name);
        return local && is_vector_storage_type(local->type);
    }

    void require_mutable_vec_method_receiver(SourceLocation loc,
                                             const std::string& name,
                                             LocalInfo& local,
                                             const std::string& method_name) {
        if (auto error = local_unavailable_binding_error(name, local)) fail(loc, *error);
        if (auto error = local_method_mutability_error(name, local, "Vec." + method_name)) fail(loc, *error);
        if (contains_borrow_type(local.type) || is_owner_type(local.type.args[0])) {
            fail(loc, "Vec." + method_name + " currently supports copyable element vectors only");
        }
        require_not_borrowed(loc, name, local, "mutate Vec with ." + method_name);
    }

    void require_readable_vec_method_receiver(SourceLocation loc,
                                              const std::string& name,
                                              const LocalInfo& local,
                                              const std::string& method_name) const {
        if (auto error = local_unavailable_binding_error(name, local)) fail(loc, *error);
        if (contains_borrow_type(local.type) || is_owner_type(local.type.args[0])) {
            fail(loc, "Vec." + method_name + " currently supports copyable element vectors only");
        }
        if (local_has_mutable_borrows(local) || local_has_mutable_field_borrows(local)) {
            fail(loc, "cannot read mutably borrowed Vec binding '" + name + "'");
        }
    }

    IrExprPtr check_vec_first_method_call(const Expr& expr, IrExprPtr lowered, const LocalInfo& local) const {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_readable_vec_method_receiver(expr.loc, name, local, "first");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::First, expr_type_args(expr).size(), expr.args.size());
        require_local_vec_known_non_empty(expr.loc, LocalVecMethod::First, local_vector_known_length(local));
        return make_vec_first_expr(expr.loc, expr_operand(expr)->loc, name, local.type);
    }

    IrExprPtr check_vec_last_method_call(const Expr& expr, IrExprPtr lowered, const LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_readable_vec_method_receiver(expr.loc, name, local, "last");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Last, expr_type_args(expr).size(), expr.args.size());
        require_local_vec_known_non_empty(expr.loc, LocalVecMethod::Last, local_vector_known_length(local));
        return make_vec_last_expr(expr.loc, expr_operand(expr)->loc, name, local.type);
    }

    IrExprPtr check_vec_get_method_call(const Expr& expr, IrExprPtr lowered, const LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_readable_vec_method_receiver(expr.loc, name, local, "get");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Get, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr index = check_expr(*expr.args[0]);
        require_local_vec_integer_argument(expr.args[0]->loc, LocalVecMethod::Get, "index", index->type);
        require_local_vec_static_index_argument(
            *expr.args[0],
            *index,
            LocalVecMethod::Get,
            "index",
            local_vector_known_length(local)
        );
        require_local_vec_known_non_empty(expr.loc, LocalVecMethod::Get, local_vector_known_length(local));
        release_temporary_borrows(borrow_mark);

        return make_vec_index_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(index)
        );
    }

    IrExprPtr check_vec_reserve_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "reserve");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Reserve, expr_type_args(expr).size(), expr.args.size());
        const Expr& capacity_expr = *expr.args[0];
        StaticIntegerValue known_capacity;
        if (known_integer_capacity(capacity_expr, known_capacity)) {
            require_local_vec_non_negative_argument(
                capacity_expr.loc,
                LocalVecMethod::Reserve,
                "capacity",
                known_capacity
            );
            widen_vector_storage(local, known_capacity.value);
            return make_void_noop_expr(expr.loc);
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr requested_capacity = check_expr(capacity_expr);
        require_local_vec_integer_argument(
            capacity_expr.loc,
            LocalVecMethod::Reserve,
            "capacity",
            requested_capacity->type
        );
        release_temporary_borrows(borrow_mark);

        StaticIntegerValue folded_capacity;
        if (try_fold_static_integer_value(*requested_capacity, folded_capacity)) {
            require_local_vec_non_negative_argument(
                capacity_expr.loc,
                LocalVecMethod::Reserve,
                "capacity",
                folded_capacity
            );
            widen_vector_storage(local, folded_capacity.value);
            return make_void_noop_expr(expr.loc);
        }

        return make_vec_reserve_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(requested_capacity)
        );
    }

    IrExprPtr check_vec_capacity_method_call(const Expr& expr, IrExprPtr lowered, const LocalInfo& local) const {
        (void)lowered;
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Capacity, expr_type_args(expr).size(), expr.args.size());
        return make_vec_capacity_expr(expr.loc, local.type);
    }

    IrExprPtr check_vec_pop_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "pop");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Pop, expr_type_args(expr).size(), expr.args.size());
        require_local_vec_known_non_empty(expr.loc, LocalVecMethod::Pop, local_vector_known_length(local));
        set_local_vector_known_length(local, vector_known_length_after_remove(local_vector_known_length(local)));
        return make_vec_pop_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type)
        );
    }

    IrExprPtr check_vec_clear_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "clear");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Clear, expr_type_args(expr).size(), expr.args.size());
        set_local_vector_known_length(local, vector_known_length_after_clear());
        return make_vec_clear_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type)
        );
    }

    IrExprPtr check_vec_truncate_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "truncate");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Truncate, expr_type_args(expr).size(), expr.args.size());

        const Expr& length_expr = *expr.args[0];
        StaticIntegerValue source_known_length;
        bool has_source_known_length = known_integer_capacity(length_expr, source_known_length);
        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr new_length = check_expr(length_expr);
        require_local_vec_integer_argument(length_expr.loc, LocalVecMethod::Truncate, "length", new_length->type);
        StaticIntegerValue folded_length;
        const StaticIntegerValue* requested_known_length = nullptr;
        if (has_source_known_length) {
            requested_known_length = &source_known_length;
        } else if (try_fold_static_integer_value(*new_length, folded_length)) {
            requested_known_length = &folded_length;
        }
        set_local_vector_known_length(
            local,
            vector_known_length_after_checked_truncate(
                length_expr.loc,
                local_vector_known_length(local),
                requested_known_length
            )
        );
        release_temporary_borrows(borrow_mark);

        return make_vec_truncate_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(new_length)
        );
    }

    IrExprPtr check_vec_set_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "set");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Set, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr index = check_expr(*expr.args[0]);
        require_local_vec_integer_argument(expr.args[0]->loc, LocalVecMethod::Set, "index", index->type);
        require_local_vec_static_index_argument(
            *expr.args[0],
            *index,
            LocalVecMethod::Set,
            "index",
            local_vector_known_length(local)
        );
        IrExprPtr value = check_expr_with_expected(*expr.args[1], local.type.args[0]);
        coerce_expr_to_expected(*value, local.type.args[0]);
        require_assignable(expr.args[1]->loc, local.type.args[0], value->type);
        require_local_vec_known_non_empty(expr.loc, LocalVecMethod::Set, local_vector_known_length(local));
        release_temporary_borrows(borrow_mark);

        return make_vec_set_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(index),
            std::move(value)
        );
    }

    IrExprPtr check_vec_swap_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "swap");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Swap, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr first_index = check_expr(*expr.args[0]);
        require_local_vec_integer_argument(
            expr.args[0]->loc,
            LocalVecMethod::Swap,
            "first index",
            first_index->type
        );
        require_local_vec_static_index_argument(
            *expr.args[0],
            *first_index,
            LocalVecMethod::Swap,
            "first index",
            local_vector_known_length(local)
        );
        IrExprPtr second_index = check_expr(*expr.args[1]);
        require_local_vec_integer_argument(
            expr.args[1]->loc,
            LocalVecMethod::Swap,
            "second index",
            second_index->type
        );
        require_local_vec_static_index_argument(
            *expr.args[1],
            *second_index,
            LocalVecMethod::Swap,
            "second index",
            local_vector_known_length(local)
        );
        require_local_vec_known_non_empty(expr.loc, LocalVecMethod::Swap, local_vector_known_length(local));
        release_temporary_borrows(borrow_mark);

        return make_vec_swap_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(first_index),
            std::move(second_index)
        );
    }

    IrExprPtr check_vec_remove_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "remove");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Remove, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr index = check_expr(*expr.args[0]);
        require_local_vec_integer_argument(expr.args[0]->loc, LocalVecMethod::Remove, "index", index->type);
        require_local_vec_static_index_argument(
            *expr.args[0],
            *index,
            LocalVecMethod::Remove,
            "index",
            local_vector_known_length(local)
        );
        require_local_vec_known_non_empty(expr.loc, LocalVecMethod::Remove, local_vector_known_length(local));
        release_temporary_borrows(borrow_mark);

        set_local_vector_known_length(local, vector_known_length_after_remove(local_vector_known_length(local)));

        return make_vec_remove_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(index)
        );
    }

    IrExprPtr check_vec_insert_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "insert");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Insert, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr index = check_expr(*expr.args[0]);
        require_local_vec_integer_argument(expr.args[0]->loc, LocalVecMethod::Insert, "index", index->type);
        require_local_vec_static_index_argument(
            *expr.args[0],
            *index,
            LocalVecMethod::Insert,
            "index",
            local_vector_known_length(local),
            true
        );
        IrExprPtr value = check_expr_with_expected(*expr.args[1], local.type.args[0]);
        coerce_expr_to_expected(*value, local.type.args[0]);
        require_assignable(expr.args[1]->loc, local.type.args[0], value->type);
        release_temporary_borrows(borrow_mark);

        widen_vector_storage_for_push(local);

        return make_vec_insert_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(index),
            std::move(value)
        );
    }

    IrExprPtr check_vec_contains_method_call(const Expr& expr, IrExprPtr lowered, const LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_readable_vec_method_receiver(expr.loc, name, local, "contains");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Contains, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr_with_expected(*expr.args[0], local.type.args[0]);
        coerce_expr_to_expected(*value, local.type.args[0]);
        require_comparable_operands(expr.args[0]->loc, local.type.args[0], value->type);
        release_temporary_borrows(borrow_mark);

        return make_vec_contains_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(value)
        );
    }

    IrExprPtr check_vec_index_of_method_call(const Expr& expr, IrExprPtr lowered, const LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_readable_vec_method_receiver(expr.loc, name, local, "index_of");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::IndexOf, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr_with_expected(*expr.args[0], local.type.args[0]);
        coerce_expr_to_expected(*value, local.type.args[0]);
        require_comparable_operands(expr.args[0]->loc, local.type.args[0], value->type);
        release_temporary_borrows(borrow_mark);

        return make_vec_index_of_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(value)
        );
    }

    IrExprPtr check_vec_count_method_call(const Expr& expr, IrExprPtr lowered, const LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_readable_vec_method_receiver(expr.loc, name, local, "count");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Count, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr_with_expected(*expr.args[0], local.type.args[0]);
        coerce_expr_to_expected(*value, local.type.args[0]);
        require_comparable_operands(expr.args[0]->loc, local.type.args[0], value->type);
        release_temporary_borrows(borrow_mark);

        return make_vec_count_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(value)
        );
    }

    IrExprPtr check_vec_push_method_call(const Expr& expr, IrExprPtr lowered, LocalInfo& local) {
        (void)lowered;
        const std::string& name = expr_operand(expr)->name;
        require_mutable_vec_method_receiver(expr.loc, name, local, "push");
        require_local_vec_method_shape(expr.loc, LocalVecMethod::Push, expr_type_args(expr).size(), expr.args.size());

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr value = check_expr_with_expected(*expr.args[0], local.type.args[0]);
        coerce_expr_to_expected(*value, local.type.args[0]);
        require_assignable(expr.args[0]->loc, local.type.args[0], value->type);
        release_temporary_borrows(borrow_mark);

        widen_vector_storage_for_push(local);

        return make_vec_push_expr(
            expr.loc,
            make_vec_local_lvalue(expr_operand(expr)->loc, name, local.type),
            std::move(value)
        );
    }

    std::optional<IrType> resolve_optional_pointer_helper_type_arg(const Expr& expr,
                                                                   const std::string& operation) {
        if (expr_type_args(expr).empty()) return std::nullopt;
        if (expr_type_args(expr).size() != 1) {
            fail(expr.loc, operation + " expects at most one type argument");
        }
        IrType element_type = resolve_executable_type(expr_type_args(expr)[0]);
        if (element_type.qualifier != TypeQualifier::Value) {
            fail(expr_type_args(expr)[0].loc, operation + "<T> expects a value type, got " + type_name(element_type));
        }
        element_type.qualifier = TypeQualifier::Ptr;
        return element_type;
    }

    IrExprPtr check_pointer_helper_pointer_arg(const Expr& arg,
                                               const std::optional<IrType>& expected_pointer) {
        IrExprPtr pointer = check_expr(arg);
        if (expected_pointer) {
            coerce_expr_to_expected(*pointer, *expected_pointer);
            require_assignable(arg.loc, *expected_pointer, pointer->type);
        }
        return pointer;
    }

    IrExprPtr check_pointer_offset_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        std::optional<IrType> expected_pointer = resolve_optional_pointer_helper_type_arg(expr, "ptr_offset");
        if (expr.args.size() != 2) fail(expr.loc, "ptr_offset expects a pointer and a byte offset");

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr pointer = check_pointer_helper_pointer_arg(*expr.args[0], expected_pointer);
        IrExprPtr offset = check_expr(*expr.args[1]);
        release_temporary_borrows(borrow_mark);

        if (!is_raw_pointer_type(pointer->type)) {
            fail(expr.args[0]->loc, "ptr_offset first argument must be a raw pointer, got " + type_name(pointer->type));
        }
        if (!is_value_integer_type(offset->type)) {
            fail(expr.args[1]->loc, "ptr_offset byte offset must be an integer, got " + type_name(offset->type));
        }

        return make_pointer_offset_expr(expr.loc, std::move(pointer), std::move(offset));
    }

    IrExprPtr check_pointer_add_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        std::optional<IrType> expected_pointer = resolve_optional_pointer_helper_type_arg(expr, "ptr_add");
        if (expr.args.size() != 2) fail(expr.loc, "ptr_add expects a pointer and an element offset");

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr pointer = check_pointer_helper_pointer_arg(*expr.args[0], expected_pointer);
        IrExprPtr offset = check_expr(*expr.args[1]);
        release_temporary_borrows(borrow_mark);

        (void)require_raw_pointer_add_type(expr.args[0]->loc, pointer->type, "ptr_add");
        if (!is_value_integer_type(offset->type)) {
            fail(expr.args[1]->loc, "ptr_add element offset must be an integer, got " + type_name(offset->type));
        }

        return make_pointer_add_expr(expr.loc, std::move(pointer), std::move(offset));
    }

    IrExprPtr check_pointer_load_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        std::optional<IrType> expected_pointer = resolve_optional_pointer_helper_type_arg(expr, "ptr_load");
        if (expr.args.size() != 1) fail(expr.loc, "ptr_load expects one pointer");

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr pointer = check_pointer_helper_pointer_arg(*expr.args[0], expected_pointer);
        release_temporary_borrows(borrow_mark);

        IrType element_type = require_raw_pointer_materializable_type(expr.args[0]->loc, pointer->type, "ptr_load");
        return make_pointer_load_expr(expr.loc, std::move(pointer), element_type);
    }

    IrExprPtr check_pointer_store_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        std::optional<IrType> expected_pointer = resolve_optional_pointer_helper_type_arg(expr, "ptr_store");
        if (expr.args.size() != 2) fail(expr.loc, "ptr_store expects a pointer and a value");

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr pointer = check_pointer_helper_pointer_arg(*expr.args[0], expected_pointer);
        IrType element_type = require_raw_pointer_materializable_type(expr.args[0]->loc, pointer->type, "ptr_store");
        IrExprPtr value = check_expr(*expr.args[1]);
        coerce_expr_to_expected(*value, element_type);
        require_assignable(expr.args[1]->loc, element_type, value->type);
        release_temporary_borrows(borrow_mark);

        return make_pointer_store_expr(expr.loc, std::move(pointer), std::move(value));
    }

    IrType resolve_optional_mem_place_type_arg(const Expr& expr, const std::string& operation) {
        if (expr_type_args(expr).size() > 1) {
            fail(expr.loc, operation + " expects at most one type argument");
        }
        if (expr_type_args(expr).empty()) return {};
        IrType explicit_type = resolve_executable_type(expr_type_args(expr)[0]);
        if (explicit_type.qualifier != TypeQualifier::Value) {
            fail(expr_type_args(expr)[0].loc, operation + "<T> expects a value type, got " + type_name(explicit_type));
        }
        return explicit_type;
    }

    IrType require_mem_place_mut_borrow(SourceLocation loc,
                                        const IrExpr& place,
                                        const std::string& operation,
                                        const std::string& argument_name) {
        if (place.type.qualifier != TypeQualifier::MutRef) {
            fail(loc, operation + " " + argument_name + " must be a mutable borrow, got " + type_name(place.type));
        }
        IrType element_type = place.type;
        element_type.qualifier = TypeQualifier::Value;
        return element_type;
    }

    void require_mem_place_type(SourceLocation loc,
                                const IrType& inferred,
                                const IrType& explicit_type,
                                bool has_explicit_type,
                                const std::string& operation,
                                const std::string& argument_name) {
        if (has_explicit_type && !same_type(inferred, explicit_type)) {
            fail(loc,
                 operation + " " + argument_name + " type mismatch: expected " +
                     type_name(explicit_type) + ", got " + type_name(inferred));
        }
        IrType pointer_type = has_explicit_type ? explicit_type : inferred;
        pointer_type.qualifier = TypeQualifier::Ptr;
        (void)require_raw_pointer_materializable_type(loc, pointer_type, operation);
    }

    static IrStmtPtr make_ir_expr_stmt(SourceLocation loc, IrExprPtr expr) {
        auto stmt = std::make_unique<IrStmt>();
        stmt->kind = IrStmtKind::ExprStmt;
        stmt->loc = loc;
        stmt->expr = std::move(expr);
        return stmt;
    }

    IrExprPtr check_mem_replace_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        const std::string operation = "std::mem::replace";
        if (expr.args.size() != 2) fail(expr.loc, operation + " expects a mutable place and a value");

        IrType explicit_type = resolve_optional_mem_place_type_arg(expr, operation);
        bool has_explicit_type = !expr_type_args(expr).empty();

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr target = check_expr(*expr.args[0]);
        IrType element_type = require_mem_place_mut_borrow(
            expr.args[0]->loc,
            *target,
            operation,
            "target");
        require_mem_place_type(
            expr.args[0]->loc,
            element_type,
            explicit_type,
            has_explicit_type,
            operation,
            "target");
        if (has_explicit_type) element_type = explicit_type;

        IrExprPtr value = check_expr_maybe_expected(*expr.args[1], &element_type);
        coerce_expr_to_expected(*value, element_type);
        require_assignable(expr.args[1]->loc, element_type, value->type);
        release_temporary_borrows(borrow_mark);

        IrType pointer_type = element_type;
        pointer_type.qualifier = TypeQualifier::Ptr;
        std::string raw_name = make_hidden_local("$mem_replace_raw");
        std::string previous_name = make_hidden_local("$mem_replace_previous");
        declare_local(expr.loc, raw_name, pointer_type, false);
        declare_local(expr.loc, previous_name, element_type, false);

        std::vector<IrStmtPtr> body;
        body.push_back(make_ir_var_decl(
            expr.loc,
            raw_name,
            pointer_type,
            make_cast_expr(expr.args[0]->loc, std::move(target), pointer_type),
            false));
        body.push_back(make_ir_var_decl(
            expr.loc,
            previous_name,
            element_type,
            make_pointer_load_expr(
                expr.loc,
                make_local_lvalue_expr(expr.loc, raw_name, pointer_type),
                element_type),
            false));
        body.push_back(make_ir_expr_stmt(
            expr.loc,
            make_pointer_store_expr(
                expr.loc,
                make_local_lvalue_expr(expr.loc, raw_name, pointer_type),
                std::move(value))));

        return make_ir_block_expr(
            expr.loc,
            {},
            element_type,
            std::move(body),
            make_local_lvalue_expr(expr.loc, previous_name, element_type));
    }

    IrExprPtr check_mem_swap_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        const std::string operation = "std::mem::swap";
        if (expr.args.size() != 2) fail(expr.loc, operation + " expects two mutable places");

        IrType explicit_type = resolve_optional_mem_place_type_arg(expr, operation);
        bool has_explicit_type = !expr_type_args(expr).empty();

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr left = check_expr(*expr.args[0]);
        IrType element_type = require_mem_place_mut_borrow(
            expr.args[0]->loc,
            *left,
            operation,
            "left argument");
        require_mem_place_type(
            expr.args[0]->loc,
            element_type,
            explicit_type,
            has_explicit_type,
            operation,
            "left argument");
        if (has_explicit_type) element_type = explicit_type;

        IrExprPtr right = check_expr(*expr.args[1]);
        IrType right_type = require_mem_place_mut_borrow(
            expr.args[1]->loc,
            *right,
            operation,
            "right argument");
        if (!same_type(right_type, element_type)) {
            fail(expr.args[1]->loc,
                 operation + " right argument type mismatch: expected " +
                     type_name(element_type) + ", got " + type_name(right_type));
        }
        release_temporary_borrows(borrow_mark);

        IrType pointer_type = element_type;
        pointer_type.qualifier = TypeQualifier::Ptr;
        std::string left_raw_name = make_hidden_local("$mem_swap_left");
        std::string right_raw_name = make_hidden_local("$mem_swap_right");
        std::string temporary_name = make_hidden_local("$mem_swap_temporary");
        declare_local(expr.loc, left_raw_name, pointer_type, false);
        declare_local(expr.loc, right_raw_name, pointer_type, false);
        declare_local(expr.loc, temporary_name, element_type, false);

        std::vector<IrStmtPtr> body;
        body.push_back(make_ir_var_decl(
            expr.loc,
            left_raw_name,
            pointer_type,
            make_cast_expr(expr.args[0]->loc, std::move(left), pointer_type),
            false));
        body.push_back(make_ir_var_decl(
            expr.loc,
            right_raw_name,
            pointer_type,
            make_cast_expr(expr.args[1]->loc, std::move(right), pointer_type),
            false));
        body.push_back(make_ir_var_decl(
            expr.loc,
            temporary_name,
            element_type,
            make_pointer_load_expr(
                expr.loc,
                make_local_lvalue_expr(expr.loc, left_raw_name, pointer_type),
                element_type),
            false));
        body.push_back(make_ir_expr_stmt(
            expr.loc,
            make_pointer_store_expr(
                expr.loc,
                make_local_lvalue_expr(expr.loc, left_raw_name, pointer_type),
                make_pointer_load_expr(
                    expr.loc,
                    make_local_lvalue_expr(expr.loc, right_raw_name, pointer_type),
                    element_type))));

        IrExprPtr final_store = make_pointer_store_expr(
            expr.loc,
            make_local_lvalue_expr(expr.loc, right_raw_name, pointer_type),
            make_local_lvalue_expr(expr.loc, temporary_name, element_type));
        return make_ir_block_expr(
            expr.loc,
            {},
            void_type(expr.loc),
            std::move(body),
            std::move(final_store));
    }

    IrExprPtr check_layout_query_call(const Expr& expr, IrExprPtr lowered, bool align_query) {
        (void)lowered;
        const std::string operation = align_query ? "align_of" : "size_of";
        if (expr_type_args(expr).size() != 1) {
            fail(expr.loc, operation + " expects exactly one type argument");
        }
        if (!expr.args.empty()) {
            fail(expr.loc, operation + " does not take value arguments");
        }

        IrType queried = resolve_executable_type(expr_type_args(expr)[0]);
        std::uint64_t bytes = 0;
        bool supported = align_query
            ? ari_layout_align_bytes(queried, bytes)
            : ari_layout_size_bytes(queried, bytes);
        if (!supported) {
            fail(expr_type_args(expr)[0].loc, operation + " does not support " + type_name(queried));
        }
        if (bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            fail(expr_type_args(expr)[0].loc, operation + " result is too large for i64");
        }
        return make_integer_literal(expr.loc, i64_type(expr.loc), bytes);
    }

    IrExprPtr check_typed_zone_alloc_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        if (expr_type_args(expr).empty()) return nullptr;
        if (expr_type_args(expr).size() != 1) {
            fail(expr.loc, "zone::alloc<T> expects exactly one type argument");
        }
        if (expr.args.size() != 1) {
            fail(expr.loc, "zone::alloc<T> expects exactly one zone argument");
        }

        IrType allocated = resolve_executable_type(expr_type_args(expr)[0]);
        if (allocated.qualifier != TypeQualifier::Value) {
            fail(expr_type_args(expr)[0].loc, "zone::alloc<T> expects a value type, got " + type_name(allocated));
        }

        std::uint64_t size_bytes = 0;
        std::uint64_t align_bytes = 0;
        if (!ari_layout_size_bytes(allocated, size_bytes) ||
            !ari_layout_align_bytes(allocated, align_bytes)) {
            fail(expr_type_args(expr)[0].loc, "zone::alloc<T> does not support " + type_name(allocated));
        }
        if (size_bytes == 0) {
            fail(expr_type_args(expr)[0].loc, "zone::alloc<T> requires a non-zero-sized type");
        }
        if (size_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
            align_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            fail(expr_type_args(expr)[0].loc, "zone::alloc<T> layout is too large for i64");
        }

        SourceLocation loc{1, 1};
        IrType zone = primitive_type(IrPrimitiveKind::Zone, "Zone", loc);
        zone.qualifier = TypeQualifier::MutRef;

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr zone_arg = check_expr(*expr.args[0]);
        coerce_expr_to_expected(*zone_arg, zone);
        require_assignable(expr.args[0]->loc, zone, zone_arg->type);

        IrType pointer_type = allocated;
        pointer_type.qualifier = TypeQualifier::Ptr;
        std::vector<IrExprPtr> args;
        args.reserve(3);
        args.push_back(std::move(zone_arg));
        args.push_back(make_integer_literal(expr.loc, i64_type(expr.loc), size_bytes));
        args.push_back(make_integer_literal(expr.loc, i64_type(expr.loc), align_bytes));
        release_temporary_borrows(borrow_mark);
        return make_ir_call_expr(expr.loc, "zone::alloc", std::move(pointer_type), std::move(args));
    }

    IrExprPtr check_zone_new_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        if (expr_type_args(expr).size() != 1) {
            fail(expr.loc, "zone::new<T> expects exactly one type argument");
        }
        if (expr.args.size() != 2) {
            fail(expr.loc, "zone::new<T> expects a zone and a value");
        }

        IrType allocated = resolve_executable_type(expr_type_args(expr)[0]);
        if (allocated.qualifier != TypeQualifier::Value) {
            fail(expr_type_args(expr)[0].loc, "zone::new<T> expects a value type, got " + type_name(allocated));
        }
        if (is_owner_type(allocated) || contains_borrow_type(allocated)) {
            fail(expr_type_args(expr)[0].loc, "zone::new<T> cannot place ownership- or borrow-valued types yet");
        }

        std::uint64_t size_bytes = 0;
        std::uint64_t align_bytes = 0;
        if (!ari_layout_size_bytes(allocated, size_bytes) ||
            !ari_layout_align_bytes(allocated, align_bytes)) {
            fail(expr_type_args(expr)[0].loc, "zone::new<T> does not support " + type_name(allocated));
        }
        if (size_bytes == 0) {
            fail(expr_type_args(expr)[0].loc, "zone::new<T> requires a non-zero-sized type");
        }
        if (size_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
            align_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            fail(expr_type_args(expr)[0].loc, "zone::new<T> layout is too large for i64");
        }

        SourceLocation loc{1, 1};
        IrType zone = primitive_type(IrPrimitiveKind::Zone, "Zone", loc);
        zone.qualifier = TypeQualifier::MutRef;

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr zone_arg = check_expr(*expr.args[0]);
        coerce_expr_to_expected(*zone_arg, zone);
        require_assignable(expr.args[0]->loc, zone, zone_arg->type);

        IrExprPtr value = check_expr(*expr.args[1]);
        coerce_expr_to_expected(*value, allocated);
        require_assignable(expr.args[1]->loc, allocated, value->type);

        IrType pointer_type = allocated;
        pointer_type.qualifier = TypeQualifier::Ptr;
        std::vector<IrExprPtr> args;
        args.reserve(4);
        args.push_back(std::move(zone_arg));
        args.push_back(make_integer_literal(expr.loc, i64_type(expr.loc), size_bytes));
        args.push_back(make_integer_literal(expr.loc, i64_type(expr.loc), align_bytes));
        args.push_back(std::move(value));
        release_temporary_borrows(borrow_mark);
        return make_ir_call_expr(expr.loc, "zone::new", std::move(pointer_type), std::move(args));
    }

    IrExprPtr check_zone_promote_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        if (expr_type_args(expr).size() != 1) {
            fail(expr.loc, "zone::promote<T> expects exactly one type argument");
        }
        if (expr.args.size() != 2) {
            fail(expr.loc, "zone::promote<T> expects a target zone and source pointer");
        }

        IrType allocated = resolve_executable_type(expr_type_args(expr)[0]);
        if (allocated.qualifier != TypeQualifier::Value) {
            fail(expr_type_args(expr)[0].loc, "zone::promote<T> expects a value type, got " + type_name(allocated));
        }
        if (is_owner_type(allocated) || contains_borrow_type(allocated)) {
            fail(expr_type_args(expr)[0].loc, "zone::promote<T> cannot copy ownership- or borrow-valued types yet");
        }

        std::uint64_t size_bytes = 0;
        std::uint64_t align_bytes = 0;
        if (!ari_layout_size_bytes(allocated, size_bytes) ||
            !ari_layout_align_bytes(allocated, align_bytes)) {
            fail(expr_type_args(expr)[0].loc, "zone::promote<T> does not support " + type_name(allocated));
        }
        if (size_bytes == 0) {
            fail(expr_type_args(expr)[0].loc, "zone::promote<T> requires a non-zero-sized type");
        }
        if (size_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ||
            align_bytes > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            fail(expr_type_args(expr)[0].loc, "zone::promote<T> layout is too large for i64");
        }

        IrType zone = primitive_type(IrPrimitiveKind::Zone, "Zone", expr.loc);
        zone.qualifier = TypeQualifier::MutRef;
        IrType source_pointer_type = allocated;
        source_pointer_type.qualifier = TypeQualifier::Ptr;

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr zone_arg = check_expr(*expr.args[0]);
        coerce_expr_to_expected(*zone_arg, zone);
        require_assignable(expr.args[0]->loc, zone, zone_arg->type);

        IrExprPtr source = check_expr(*expr.args[1]);
        coerce_expr_to_expected(*source, source_pointer_type);
        require_assignable(expr.args[1]->loc, source_pointer_type, source->type);

        IrExprPtr value = make_pointer_load_expr(expr.args[1]->loc, std::move(source), allocated);

        std::vector<IrExprPtr> args;
        args.reserve(4);
        args.push_back(std::move(zone_arg));
        args.push_back(make_integer_literal(expr.loc, i64_type(expr.loc), size_bytes));
        args.push_back(make_integer_literal(expr.loc, i64_type(expr.loc), align_bytes));
        args.push_back(std::move(value));
        release_temporary_borrows(borrow_mark);
        return make_ir_call_expr(expr.loc, "zone::new", std::move(source_pointer_type), std::move(args));
    }

    IrExprPtr check_zone_temp_call(const Expr& expr, IrExprPtr lowered) {
        (void)lowered;
        if (!expr_type_args(expr).empty()) {
            fail(expr.loc, "zone::temp does not take type arguments");
        }
        if (expr.args.size() != 1) {
            fail(expr.loc, "zone::temp expects a capacity");
        }

        IrType capacity_type = i64_type(expr.loc);
        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr capacity = check_expr(*expr.args[0]);
        coerce_expr_to_expected(*capacity, capacity_type);
        require_assignable(expr.args[0]->loc, capacity_type, capacity->type);
        release_temporary_borrows(borrow_mark);

        IrType zone = primitive_type(IrPrimitiveKind::Zone, "Zone", expr.loc);
        zone.qualifier = TypeQualifier::Own;
        std::vector<IrExprPtr> args;
        args.push_back(std::move(capacity));
        return make_ir_call_expr(expr.loc, "zone::temp", std::move(zone), std::move(args));
    }

    bool is_collection_len_method_receiver(const Expr& expr) {
        if (expr.kind == ExprKind::Vector) return true;
        if (expr.kind == ExprKind::Name) {
            const LocalInfo* local = find_local_slot(expr.name);
            return local && (local->type.primitive == IrPrimitiveKind::Vector ||
                             local->type.primitive == IrPrimitiveKind::Array ||
                             is_prelude_slice_type(local->type));
        }
        if (expr.kind == ExprKind::Block && expr_block_value(expr)) {
            return is_collection_len_method_receiver(*expr_block_value(expr));
        }
        if (expr.kind == ExprKind::If) {
            return expr_if_then_value(expr) && expr_if_else_value(expr) &&
                   is_collection_len_method_receiver(*expr_if_then_value(expr)) &&
                   is_collection_len_method_receiver(*expr_if_else_value(expr));
        }
        if (expr.kind == ExprKind::Match) {
            if (expr_match_arms(expr).empty()) return false;
            for (const auto& arm : expr_match_arms(expr)) {
                if (!arm.value || !is_collection_len_method_receiver(*arm.value)) return false;
            }
            return true;
        }
        return false;
    }

    const ImplMethodInfo* select_trait_qualified_concrete_method_impl(
        SourceLocation loc,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const IrType& receiver_type,
        const std::string& method_name,
        const std::vector<ImplMethodInfo>& candidates
    ) const {
        const ImplMethodInfo* selected = nullptr;
        for (const auto& candidate : candidates) {
            if (!same_type(candidate.receiver_type, receiver_type)) continue;
            if (candidate.trait_name.empty()) continue;
            if (!trait_application_implies_trait(
                    trait_name,
                    trait_args,
                    candidate.trait_name,
                    candidate.trait_args)) {
                continue;
            }
            if (selected) {
                fail(loc,
                     "trait-qualified method call '" +
                         trait_method_display(trait_name, trait_args, method_name) +
                         "' for type " + type_name(receiver_type) + " is ambiguous");
            }
            selected = &candidate;
        }
        return selected;
    }

    bool try_select_trait_qualified_generic_method_impl(
        const Expr& expr,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const IrType& receiver_type,
        const std::string& method_name,
        const std::vector<IrType>& arg_types,
        ImplMethodInfo& selected
    ) {
        std::vector<ImplMethodInfo> matches;
        std::string first_bound_failure;
        std::string first_inference_failure;
        bool saw_wrong_arg_count = false;

        Expr method_expr;
        method_expr.kind = ExprKind::Call;
        method_expr.loc = expr.loc;
        method_expr.name = method_name;

        for (const auto& candidate : generic_method_impls_) {
            if (basename_of_qualified_name(candidate.fn->name) != method_name) continue;
            if (candidate.trait_name.empty()) continue;

            std::map<std::string, IrType> substitutions;
            if (!infer_generic_impl_method_substitutions(candidate, receiver_type, substitutions)) continue;

            std::vector<IrType> candidate_trait_args;
            candidate_trait_args.reserve(candidate.trait_args.size());
            for (const auto& arg : candidate.trait_args) {
                candidate_trait_args.push_back(substitute_inferred_type(arg, substitutions));
            }
            if (!trait_application_implies_trait(
                    trait_name,
                    trait_args,
                    candidate.trait_name,
                    candidate_trait_args)) {
                continue;
            }

            if (candidate.sig.params.size() != arg_types.size() + 1) {
                saw_wrong_arg_count = true;
                continue;
            }
            if (!bind_or_infer_method_generic_type_args(
                    method_expr,
                    candidate,
                    arg_types,
                    substitutions,
                    &first_inference_failure)) {
                continue;
            }

            std::set<std::string> visiting;
            std::string bound_failure;
            if (!impl_generic_bounds_satisfied(candidate.generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }
            if (!impl_generic_bounds_satisfied(candidate.method_generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }

            matches.push_back(specialize_generic_impl_method_with_substitutions(
                candidate,
                receiver_type,
                method_name,
                std::move(substitutions)));
        }

        if (matches.empty()) {
            if (!first_bound_failure.empty()) fail(expr.loc, first_bound_failure);
            if (!first_inference_failure.empty()) fail(expr.loc, first_inference_failure);
            if (saw_wrong_arg_count) fail(expr.loc, "wrong argument count for method '" + method_name + "'");
            return false;
        }
        if (matches.size() > 1) {
            fail(expr.loc,
                 "trait-qualified method call '" +
                     trait_method_display(trait_name, trait_args, method_name) +
                     "' for type " + type_name(receiver_type) + " is ambiguous");
        }
        selected = std::move(matches.front());
        return true;
    }

    const ImplMethodInfo* select_trait_qualified_concrete_associated_impl(
        SourceLocation loc,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const IrType& receiver_type,
        const std::string& method_name,
        const std::vector<ImplMethodInfo>& candidates
    ) const {
        const ImplMethodInfo* selected = nullptr;
        for (const auto& candidate : candidates) {
            if (!same_type(candidate.receiver_type, receiver_type)) continue;
            if (candidate.trait_name.empty()) continue;
            if (!trait_application_implies_trait(
                    trait_name,
                    trait_args,
                    candidate.trait_name,
                    candidate.trait_args)) {
                continue;
            }
            if (selected) {
                fail(loc,
                     "trait-qualified associated function call '" +
                         trait_method_display(trait_name, trait_args, method_name) +
                         "' for type " + type_name(receiver_type) + " is ambiguous");
            }
            selected = &candidate;
        }
        return selected;
    }

    bool bind_or_infer_associated_method_generic_type_args(
        const Expr& expr,
        const ImplMethodInfo& method,
        const std::vector<IrType>& arg_types,
        std::map<std::string, IrType>& substitutions,
        std::string* failure
    ) {
        const std::string method_name = basename_of_qualified_name(method.fn->name);
        const ExprTypeArgs& type_args = expr_type_args(expr);
        if (method.method_generic_names.empty()) {
            if (!type_args.empty()) {
                fail(expr.loc, "associated function '" + method_name + "' does not take method type arguments");
            }
            return true;
        }

        if (!type_args.empty()) {
            if (type_args.size() != method.method_generic_names.size()) {
                fail(expr.loc,
                     "generic associated function '" + method_name + "' expects " +
                         std::to_string(method.method_generic_names.size()) +
                         " method type argument" +
                         (method.method_generic_names.size() == 1 ? "" : "s"));
            }
            for (std::size_t i = 0; i < type_args.size(); ++i) {
                bind_generic_type(
                    type_args[i].loc,
                    method.method_generic_names[i],
                    resolve_executable_type(type_args[i]),
                    substitutions);
            }
        } else {
            for (std::size_t i = 0; i < arg_types.size(); ++i) {
                infer_named_generic_type(
                    expr.loc,
                    method.fn->params[i].type,
                    arg_types[i],
                    method.method_generic_names,
                    substitutions);
            }
        }

        for (const auto& generic_name : method.method_generic_names) {
            if (!substitutions.count(generic_name)) {
                if (failure) {
                    *failure = "generic type '" + generic_name +
                        "' could not be inferred for associated function '" +
                        method_name + "'";
                }
                return false;
            }
        }
        return true;
    }

    bool try_select_trait_qualified_generic_associated_impl(
        const Expr& expr,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const IrType& receiver_type,
        const std::string& method_name,
        const std::vector<IrType>& arg_types,
        ImplMethodInfo& selected
    ) {
        std::vector<ImplMethodInfo> matches;
        std::string first_bound_failure;
        std::string first_inference_failure;
        bool saw_wrong_arg_count = false;

        for (const auto& candidate : generic_associated_impls_) {
            if (basename_of_qualified_name(candidate.fn->name) != method_name) continue;
            if (candidate.trait_name.empty()) continue;

            std::map<std::string, IrType> substitutions;
            if (!infer_generic_pattern_type(
                    candidate.receiver_type,
                    receiver_type,
                    candidate.generic_names,
                    substitutions)) {
                continue;
            }
            if (candidate.trait_name == trait_name && candidate.trait_args.size() == trait_args.size()) {
                bool trait_args_match = true;
                for (std::size_t i = 0; i < trait_args.size(); ++i) {
                    if (!infer_generic_pattern_type(
                            candidate.trait_args[i],
                            trait_args[i],
                            candidate.generic_names,
                            substitutions)) {
                        trait_args_match = false;
                        break;
                    }
                }
                if (!trait_args_match) continue;
            }
            bool has_all_impl_generics = true;
            for (const auto& generic_name : candidate.generic_names) {
                if (!substitutions.count(generic_name)) {
                    has_all_impl_generics = false;
                    break;
                }
            }
            if (!has_all_impl_generics) continue;

            std::vector<IrType> candidate_trait_args;
            candidate_trait_args.reserve(candidate.trait_args.size());
            for (const auto& arg : candidate.trait_args) {
                candidate_trait_args.push_back(substitute_inferred_type(arg, substitutions));
            }
            if (!trait_application_implies_trait(
                    trait_name,
                    trait_args,
                    candidate.trait_name,
                    candidate_trait_args)) {
                continue;
            }

            if (candidate.sig.params.size() != arg_types.size()) {
                saw_wrong_arg_count = true;
                continue;
            }
            if (!bind_or_infer_associated_method_generic_type_args(
                    expr,
                    candidate,
                    arg_types,
                    substitutions,
                    &first_inference_failure)) {
                continue;
            }

            std::set<std::string> visiting;
            std::string bound_failure;
            if (!impl_generic_bounds_satisfied(candidate.generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }
            if (!impl_generic_bounds_satisfied(candidate.method_generic_bounds, substitutions, visiting, &bound_failure)) {
                if (first_bound_failure.empty()) first_bound_failure = bound_failure;
                continue;
            }

            matches.push_back(specialize_generic_impl_method_with_substitutions(
                candidate,
                receiver_type,
                method_name,
                std::move(substitutions)));
        }

        if (matches.empty()) {
            if (!first_bound_failure.empty()) fail(expr.loc, first_bound_failure);
            if (!first_inference_failure.empty()) fail(expr.loc, first_inference_failure);
            if (saw_wrong_arg_count) fail(expr.loc, "wrong argument count for associated function '" + method_name + "'");
            return false;
        }
        if (matches.size() > 1) {
            fail(expr.loc,
                 "trait-qualified associated function call '" +
                     trait_method_display(trait_name, trait_args, method_name) +
                     "' for type " + type_name(receiver_type) + " is ambiguous");
        }
        selected = std::move(matches.front());
        return true;
    }

    const ImplMethodInfo* select_constrained_method_impl(
        const Expr& method_expr,
        const IrType& receiver_type,
        const std::vector<ImplMethodInfo>& candidates
    ) {
        std::string generic_name = generic_origin_from_expr(*expr_operand(method_expr));
        if (generic_name.empty()) return nullptr;

        const ImplMethodInfo* selected = nullptr;
        for (const auto& bound : current_generic_bounds_) {
            if (bound.generic_name != generic_name) continue;
            if (!trait_application_has_method(bound.trait_name, bound.trait_args, method_expr.name)) continue;

            for (const auto& candidate : candidates) {
                if (!same_type(candidate.receiver_type, receiver_type)) continue;
                if (!trait_application_implies_trait(
                        bound.trait_name,
                        bound.trait_args,
                        candidate.trait_name,
                        candidate.trait_args)) {
                    continue;
                }
                if (selected) {
                    fail(method_expr.loc,
                         "method call '" + method_expr.name + "' for generic parameter '" +
                             generic_name + "' is ambiguous across trait bounds");
                }
                selected = &candidate;
            }
        }
        return selected;
    }

    IrExprPtr check_indirect_call(SourceLocation loc, IrExprPtr callee, const std::vector<ExprPtr>& arg_exprs, IrExprPtr lowered) {
        if (callee->type.qualifier != TypeQualifier::Value ||
            callee->type.primitive != IrPrimitiveKind::Function ||
            callee->type.args.empty() ||
            callee->type.array_size + 1 != callee->type.args.size()) {
            fail(loc, "called value must be a function pointer, got " + type_name(callee->type));
        }
        std::size_t param_count = static_cast<std::size_t>(callee->type.array_size);
        if (param_count != arg_exprs.size()) {
            fail(loc, "wrong argument count for function pointer call");
        }

        lowered->kind = IrExprKind::IndirectCall;
        lowered->loc = loc;
        lowered->type = function_pointer_result_type(callee->type);
        set_ir_expr_operand(*lowered, std::move(callee));
        lowered->args.reserve(arg_exprs.size());
        std::size_t borrow_mark = temporary_borrow_mark();
        for (std::size_t i = 0; i < arg_exprs.size(); ++i) {
            IrExprPtr arg = check_call_argument_for_expected(*arg_exprs[i], ir_expr_operand(*lowered)->type.args[i]);
            require_assignable(arg_exprs[i]->loc, ir_expr_operand(*lowered)->type.args[i], arg->type);
            require_no_zone_pointer_escape(arg_exprs[i]->loc, *arg, "function pointer call argument");
            lowered->args.push_back(std::move(arg));
        }
        if (is_borrow_type(lowered->type)) {
            fail(loc, "function pointer calls cannot return tracked borrow values yet");
        }
        release_temporary_borrows(borrow_mark);
        return lowered;
    }

    std::vector<IrType> enum_constructor_type_args(
        const Expr& expr,
        const EnumCaseInfo& info,
        const std::vector<IrType>& arg_types
    ) {
        auto enum_found = enums_.find(info.enum_name);
        if (enum_found == enums_.end()) fail(expr.loc, "unknown enum '" + info.enum_name + "'");
        const EnumInfo& enum_info = enum_found->second;
        if (!info.is_generic) {
            if (!expr_type_args(expr).empty()) {
                fail(expr.loc, "enum case constructor '" + expr.name + "' does not take type arguments");
            }
            return {};
        }

        if (arg_types.size() != info.payload_refs.size()) {
            fail(expr.loc, "wrong payload count for enum case '" + info.name + "'");
        }

        std::vector<IrType> type_args;
        type_args.reserve(enum_info.generic_arity);
        if (!expr_type_args(expr).empty()) {
            if (expr_type_args(expr).size() != enum_info.generic_arity) {
                fail(expr.loc,
                     "enum '" + enum_info.name + "' expects " + std::to_string(enum_info.generic_arity) +
                         " type argument" + (enum_info.generic_arity == 1 ? "" : "s"));
            }
            for (const auto& arg : expr_type_args(expr)) {
                type_args.push_back(resolve_executable_type(arg));
            }
            return type_args;
        }

        std::map<std::string, IrType> inferred;
        for (std::size_t i = 0; i < info.payload_refs.size(); ++i) {
            infer_named_generic_type(expr.loc, info.payload_refs[i], arg_types[i], info.generic_names, inferred);
        }

        for (const auto& generic_name : info.generic_names) {
            auto found = inferred.find(generic_name);
            if (found == inferred.end()) {
                fail(expr.loc,
                     "cannot infer type argument '" + generic_name +
                         "' for enum case '" + info.name + "'");
            }
            type_args.push_back(found->second);
        }
        return type_args;
    }

    std::vector<IrType> enum_constructor_type_args_from_expected(
        const Expr& expr,
        const EnumCaseInfo& info,
        const IrType* expected,
        bool& has_type_args
    ) {
        has_type_args = false;
        auto enum_found = enums_.find(info.enum_name);
        if (enum_found == enums_.end()) fail(expr.loc, "unknown enum '" + info.enum_name + "'");
        const EnumInfo& enum_info = enum_found->second;
        if (!info.is_generic) {
            if (!expr_type_args(expr).empty()) {
                fail(expr.loc, "enum case constructor '" + expr.name + "' does not take type arguments");
            }
            has_type_args = true;
            return {};
        }

        std::vector<IrType> type_args;
        type_args.reserve(enum_info.generic_arity);
        if (!expr_type_args(expr).empty()) {
            if (expr_type_args(expr).size() != enum_info.generic_arity) {
                fail(expr.loc,
                     "enum '" + enum_info.name + "' expects " + std::to_string(enum_info.generic_arity) +
                         " type argument" + (enum_info.generic_arity == 1 ? "" : "s"));
            }
            for (const auto& arg : expr_type_args(expr)) {
                type_args.push_back(resolve_executable_type(arg));
            }
            has_type_args = true;
            return type_args;
        }

        if (expected_type_matches_enum_constructor(expected, info.enum_name, enum_info.generic_arity)) {
            has_type_args = true;
            return expected->args;
        }

        return {};
    }

    IrExprPtr check_enum_constructor_call(const Expr& expr, const EnumCaseInfo& base_info, const IrType* expected = nullptr) {
        require_enum_case_access(expr.loc, base_info);
        auto enum_found = enums_.find(base_info.enum_name);
        if (enum_found != enums_.end() && enum_found->second.deprecated) {
            warn_deprecated_use(expr.loc, "enum", enum_found->second.name, enum_found->second.deprecated_message);
        }
        if (expr.args.size() != base_info.payload_refs.size()) {
            fail(expr.loc, "wrong payload count for enum case '" + base_info.name + "'");
        }

        EnumCaseInfo info = base_info;
        bool has_type_args = !base_info.is_generic;
        std::vector<IrType> type_args = enum_constructor_type_args_from_expected(expr, base_info, expected, has_type_args);
        if (base_info.is_generic && has_type_args) {
            info = specialize_enum_case_info(expr.loc, base_info, type_args);
        }

        std::vector<IrExprPtr> args;
        std::vector<IrType> arg_types;
        args.reserve(expr.args.size());
        arg_types.reserve(expr.args.size());
        std::size_t borrow_mark = temporary_borrow_mark();
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            const IrType* payload_expected = has_type_args ? &info.payloads[i] : nullptr;
            IrExprPtr arg = check_expr_maybe_expected(*expr.args[i], payload_expected);
            arg_types.push_back(arg->type);
            args.push_back(std::move(arg));
        }
        release_temporary_borrows(borrow_mark);

        if (base_info.is_generic && !has_type_args) {
            type_args = enum_constructor_type_args(expr, base_info, arg_types);
            info = specialize_enum_case_info(expr.loc, base_info, type_args);
        }
        return make_enum_construct(expr.loc, info, std::move(args));
    }

    IrExprPtr check_call(const Expr& expr, IrExprPtr lowered, const IrType* expected = nullptr) {
        if (expr_operand(expr)) {
            return check_indirect_call(expr.loc, check_expr(*expr_operand(expr)), expr.args, std::move(lowered));
        }
        if (LocalInfo* local = find_local_slot(expr.name)) {
            if (local->type.primitive != IrPrimitiveKind::Function || local->type.qualifier != TypeQualifier::Value) {
                fail(expr.loc, "called value must be a function pointer, got " + type_name(local->type));
            }
            Expr local_expr;
            local_expr.kind = ExprKind::Name;
            local_expr.loc = expr.loc;
            local_expr.name = expr.name;
            return check_indirect_call(expr.loc, check_expr(local_expr), expr.args, std::move(lowered));
        }
        std::string prelude_name = resolve_use_path(expr.name);
        bool local_decl_shadows_prelude = unqualified_decl_shadows_prelude_name(expr.name, prelude_name);
        const std::string& special_name = local_decl_shadows_prelude ? expr.name : prelude_name;
        bool can_use_prelude_special = prelude_specials_available() && !local_decl_shadows_prelude;
        bool can_use_source_declared_prelude_special =
            can_use_prelude_special && source_std_generic_function_available(special_name);

        if (can_use_prelude_special && is_format_print_name(special_name)) {
            if (!expr_type_args(expr).empty()) {
                fail(expr.loc, "function '" + expr.name + "' does not take type arguments");
            }
            return check_format_print(expr, std::move(lowered), special_name);
        }

        if (can_use_prelude_special && is_prelude_vec_len_function_name(special_name)) {
            return check_vec_len_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_prelude_pointer_offset_function_name(special_name)) {
            return check_pointer_offset_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_prelude_pointer_add_function_name(special_name)) {
            return check_pointer_add_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_prelude_pointer_load_function_name(special_name)) {
            return check_pointer_load_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_prelude_pointer_store_function_name(special_name)) {
            return check_pointer_store_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_prelude_mem_replace_function_name(special_name)) {
            return check_mem_replace_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_prelude_mem_swap_function_name(special_name)) {
            return check_mem_swap_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_prelude_size_of_function_name(special_name)) {
            return check_layout_query_call(expr, std::move(lowered), false);
        }
        if (can_use_source_declared_prelude_special && is_prelude_align_of_function_name(special_name)) {
            return check_layout_query_call(expr, std::move(lowered), true);
        }
        if (can_use_source_declared_prelude_special && is_prelude_move_function_name(special_name)) {
            return check_explicit_move_call(expr, std::move(lowered), false);
        }
        if (can_use_source_declared_prelude_special && is_prelude_take_function_name(special_name)) {
            return check_explicit_move_call(expr, std::move(lowered), true);
        }
        if (can_use_source_declared_prelude_special && is_zone_alloc_function_name(special_name) && !expr_type_args(expr).empty()) {
            return check_typed_zone_alloc_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_zone_new_function_name(special_name)) {
            return check_zone_new_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_zone_promote_function_name(special_name)) {
            return check_zone_promote_call(expr, std::move(lowered));
        }
        if (can_use_prelude_special && is_zone_scratch_function_name(special_name)) {
            fail(expr.loc, "zone::scratch<T> can only initialize a local pointer binding");
        }
        if (can_use_prelude_special && is_zone_temp_function_name(special_name)) {
            if (!allow_zone_temp_init_) {
                fail(expr.loc, "zone::temp can only initialize a local temporary zone binding");
            }
            return check_zone_temp_call(expr, std::move(lowered));
        }
        if (can_use_source_declared_prelude_special && is_prelude_range_function_name(special_name)) {
            return check_range_call(expr, std::move(lowered), expected, special_name);
        }

        if (!expr_type_args(expr).empty()) {
            std::string generic_name;
            if (const FunctionDecl* generic = find_generic_function(expr, generic_name)) {
                return check_generic_call(expr, *generic, generic_name, std::move(lowered));
            }
        }

        std::string function_name = resolve_function_name(expr.name);
        auto found = functions_.find(function_name);
        if (found == functions_.end()) {
            std::string generic_name;
            if (const FunctionDecl* generic = find_generic_function(expr, generic_name)) {
                return check_generic_call(expr, *generic, generic_name, std::move(lowered));
            }
            std::string case_name = resolve_enum_case_name(expr.name);
            auto case_found = enum_cases_.find(case_name);
            if (case_found != enum_cases_.end()) {
                return check_enum_constructor_call(expr, case_found->second, expected);
            }
            std::string struct_name = resolve_struct_type_name(expr.name);
            auto struct_found = structs_.find(struct_name);
            if (struct_found != structs_.end()) {
                require_struct_access(expr.loc, struct_found->second);
                return check_struct_constructor_call(expr, struct_found->second, std::move(lowered), expected);
            }
            if (is_planned_prelude_function_name(expr.name)) {
                fail(expr.loc, planned_prelude_function_message(expr.name));
            }
            std::string receiver_name;
            std::string method_name;
            if (split_associated_call_name(expr.name, receiver_name, method_name)) {
                if (IrExprPtr associated =
                        check_associated_call(expr, receiver_name, method_name, std::move(lowered), expected)) {
                    return associated;
                }
            }
            fail(expr.loc, "unknown function or enum case '" + expr.name + "'");
        }
        const FunctionSig& sig = found->second;
        require_function_access(expr.loc, sig, function_name);
        if (sig.deprecated) {
            warn_deprecated_use(expr.loc, "function", function_name, sig.deprecated_message);
        }
        if (!expr_type_args(expr).empty()) {
            fail(expr.loc, "function '" + expr.name + "' does not take type arguments");
        }
        if (sig.is_variadic) {
            if (expr.args.size() < sig.params.size()) {
                fail(expr.loc,
                     "wrong argument count for variadic function '" + expr.name +
                         "': expected at least " + std::to_string(sig.params.size()));
            }
        } else if (sig.params.size() != expr.args.size()) {
            fail(expr.loc, "wrong argument count for '" + expr.name + "'");
        }
        if (expr.args.size() > std::numeric_limits<std::uint16_t>::max()) {
            fail(expr.loc, "function calls support up to 65535 arguments");
        }

        std::vector<IrExprPtr> args;
        args.reserve(expr.args.size());
        std::size_t borrow_mark = temporary_borrow_mark();
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            IrExprPtr arg = i < sig.params.size()
                ? check_call_argument_for_expected(*expr.args[i], sig.params[i])
                : check_expr(*expr.args[i]);
            if (i < sig.params.size()) {
                require_assignable(expr.loc, sig.params[i], arg->type);
            } else if (!is_c_vararg_value_type(arg->type)) {
                fail(expr.args[i]->loc, "C variadic argument type is not supported: " + type_name(arg->type));
            } else {
                arg = promote_c_vararg(std::move(arg), expr.args[i]->loc);
            }
            if (sig.is_extern) {
                const char* escape_context = sig.extern_abi == "ari"
                    ? "extern ari builtin call argument"
                    : "extern C call argument";
                if (!std_string_extern_builtin_allows_zone_pointer_argument(function_name, i) &&
                    !zone_metadata_extern_builtin_allows_zone_pointer_argument(function_name, i)) {
                    require_no_zone_pointer_escape(expr.args[i]->loc, *arg, escape_context);
                }
            }
            args.push_back(std::move(arg));
        }
        IrExprPtr call = finish_tracked_call(
            expr.loc,
            function_name,
            function_name,
            sig,
            std::move(args),
            borrow_mark);
        mark_zone_reset_call(*call);
        return call;
    }

    IrExprPtr check_trait_qualified_method_call(
        const Expr& expr,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const std::string& method_name,
        IrExprPtr lowered
    ) {
        (void)lowered;
        const std::string display = trait_method_display(trait_name, trait_args, method_name);
        if (!trait_application_has_method(trait_name, trait_args, method_name)) {
            fail(expr.loc, "trait '" + trait_application_display(trait_name, trait_args) +
                     "' has no method '" + method_name + "'");
        }
        if (expr.args.empty()) {
            fail(expr.loc, "trait-qualified method call '" + display + "' requires a receiver argument");
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr receiver = check_expr(*expr.args.front());
        IrType method_receiver_type = receiver->type;
        if (is_receiver_borrow_type(method_receiver_type)) {
            method_receiver_type = value_qualified_type(method_receiver_type);
        }

        std::vector<IrExprPtr> checked_args;
        std::vector<IrType> arg_types;
        checked_args.reserve(expr.args.size() - 1);
        arg_types.reserve(expr.args.size() - 1);
        for (std::size_t i = 1; i < expr.args.size(); ++i) {
            IrExprPtr arg = check_expr(*expr.args[i]);
            arg_types.push_back(arg->type);
            checked_args.push_back(std::move(arg));
        }

        if (!type_implements_trait(trait_name, trait_args, method_receiver_type)) {
            fail(expr.loc,
                 "type " + type_name(method_receiver_type) + " does not implement trait '" +
                     trait_application_display(trait_name, trait_args) +
                     "' for trait-qualified method call");
        }

        const ImplMethodInfo* selected = nullptr;
        auto found = method_impls_.find(method_lookup_key(method_receiver_type, method_name));
        if (found != method_impls_.end()) {
            selected = select_trait_qualified_concrete_method_impl(
                expr.loc,
                trait_name,
                trait_args,
                method_receiver_type,
                method_name,
                found->second);
        }

        ImplMethodInfo generic_selected;
        bool has_generic_selected = false;
        if (!selected) {
            has_generic_selected = try_select_trait_qualified_generic_method_impl(
                expr,
                trait_name,
                trait_args,
                method_receiver_type,
                method_name,
                arg_types,
                generic_selected);
        }
        if (!selected && !has_generic_selected) {
            fail(expr.loc,
                 "trait-qualified method call '" + display +
                     "' has no matching impl for type " + type_name(method_receiver_type));
        }

        const ImplMethodInfo& method = has_generic_selected ? generic_selected : *selected;
        const FunctionSig& sig = method.sig;
        if (sig.params.empty()) {
            fail(expr.loc, "method '" + method_name + "' for type " + type_name(method_receiver_type) +
                     " has no receiver parameter");
        }
        if (sig.params.size() != expr.args.size()) {
            fail(expr.loc, "wrong argument count for trait-qualified method call '" + display + "'");
        }

        bool mutable_receiver_borrow = false;
        if (!is_borrow_type(receiver->type) &&
            borrowed_receiver_matches_value(sig.params[0], receiver->type, mutable_receiver_borrow)) {
            ExprPtr borrow_expr = make_ast_borrow_expr(
                expr.args.front()->loc,
                clone_borrowable_receiver_expr(*expr.args.front()),
                mutable_receiver_borrow);
            if (!expr_operand(*borrow_expr)) {
                fail(expr.loc,
                     "method '" + method_name +
                         "' with borrowed receiver requires a local binding, field access, tuple index, or constant aggregate index");
            }
            receiver = check_expr(*borrow_expr);
        }

        weaken_mut_receiver_to_shared_if_needed(*receiver, sig.params[0]);
        coerce_expr_to_expected(*receiver, sig.params[0]);
        require_assignable(expr.loc, sig.params[0], receiver->type);

        require_impl_method_access(expr.loc, method, method_name);
        queue_impl_method_for_lowering(method);
        std::vector<IrExprPtr> args;
        args.reserve(expr.args.size());
        args.push_back(std::move(receiver));
        for (std::size_t i = 1; i < expr.args.size(); ++i) {
            IrExprPtr arg = coerce_checked_call_argument_or_implicit_slice(
                *expr.args[i],
                std::move(checked_args[i - 1]),
                sig.params[i]);
            args.push_back(std::move(arg));
        }
        return finish_tracked_call(
            expr.loc,
            display,
            method.lowered_name,
            sig,
            std::move(args),
            borrow_mark);
    }

    IrExprPtr check_trait_qualified_associated_call(
        const Expr& expr,
        const std::string& trait_name,
        const std::vector<IrType>& trait_args,
        const std::string& method_name,
        IrExprPtr lowered,
        const IrType* expected = nullptr
    ) {
        (void)lowered;
        const std::string display = trait_method_display(trait_name, trait_args, method_name);
        const ExprTypeArgs& type_args = expr_type_args(expr);
        IrType receiver_type;
        if (!type_args.empty()) {
            receiver_type = resolve_executable_type(type_args.front());
        } else if (expected && trait_expected_type_can_select_associated_self(*expected)) {
            receiver_type = *expected;
        } else {
            fail(expr.loc,
                 "trait-qualified associated function call '" + display +
                     "' requires an explicit implementing type argument");
        }
        if (receiver_type.qualifier != TypeQualifier::Value) {
            fail(type_args.empty() ? expr.loc : type_args.front().loc,
                 "trait-qualified associated function implementing type must be a value type, got " +
                     type_name(receiver_type));
        }

        std::set<std::string> visiting;
        if (!type_implements_trait(trait_name, trait_args, receiver_type, visiting)) {
            fail(expr.loc,
                 "type " + type_name(receiver_type) + " does not implement trait '" +
                     trait_application_display(trait_name, trait_args) +
                     "' for trait-qualified associated function call");
        }

        Expr method_expr;
        method_expr.kind = ExprKind::Call;
        method_expr.loc = expr.loc;
        method_expr.name = method_name;
        if (!type_args.empty()) {
            ExprTypeArgs method_type_args(type_args.begin() + 1, type_args.end());
            set_expr_type_args(method_expr, std::move(method_type_args));
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        std::vector<IrExprPtr> checked_args;
        std::vector<IrType> arg_types;
        checked_args.reserve(expr.args.size());
        arg_types.reserve(expr.args.size());
        for (const auto& arg_expr : expr.args) {
            IrExprPtr arg = check_expr(*arg_expr);
            arg_types.push_back(arg->type);
            checked_args.push_back(std::move(arg));
        }

        const ImplMethodInfo* selected = nullptr;
        auto found = associated_impls_.find(method_lookup_key(receiver_type, method_name));
        if (found != associated_impls_.end()) {
            selected = select_trait_qualified_concrete_associated_impl(
                expr.loc,
                trait_name,
                trait_args,
                receiver_type,
                method_name,
                found->second);
        }

        ImplMethodInfo generic_selected;
        bool has_generic_selected = false;
        if (!selected) {
            has_generic_selected = try_select_trait_qualified_generic_associated_impl(
                method_expr,
                trait_name,
                trait_args,
                receiver_type,
                method_name,
                arg_types,
                generic_selected);
        }
        if (!selected && !has_generic_selected) {
            fail(expr.loc,
                 "trait-qualified associated function call '" + display +
                     "' has no matching impl for type " + type_name(receiver_type));
        }
        if (selected && !expr_type_args(method_expr).empty()) {
            fail(expr.loc, "associated function '" + method_name + "' does not take method type arguments");
        }

        const ImplMethodInfo& method = has_generic_selected ? generic_selected : *selected;
        require_impl_method_access(expr.loc, method, method_name);
        const FunctionSig& sig = method.sig;
        if (sig.params.size() != expr.args.size()) {
            fail(expr.loc, "wrong argument count for trait-qualified associated function call '" + display + "'");
        }

        queue_impl_method_for_lowering(method);
        std::vector<IrExprPtr> args;
        args.reserve(expr.args.size());
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            IrExprPtr arg = coerce_checked_call_argument_or_implicit_slice(
                *expr.args[i],
                std::move(checked_args[i]),
                sig.params[i]);
            args.push_back(std::move(arg));
        }
        return finish_tracked_call(
            expr.loc,
            display,
            method.lowered_name,
            sig,
            std::move(args),
            borrow_mark);
    }

    IrExprPtr check_associated_call(
        const Expr& expr,
        const std::string& receiver_name,
        const std::string& method_name,
        IrExprPtr lowered,
        const IrType* expected = nullptr
    ) {
        std::string trait_name;
        std::vector<IrType> trait_args;
        if (try_resolve_trait_qualified_call_target(
                expr.loc,
                receiver_name,
                expr_receiver_type_args(expr),
                trait_name,
                trait_args)) {
            const TraitInfo::Method* trait_method =
                find_trait_application_method(expr.loc, trait_name, trait_args, method_name);
            if (!trait_method) {
                fail(expr.loc, "trait '" + trait_application_display(trait_name, trait_args) +
                         "' has no method '" + method_name + "'");
            }
            if (!trait_method->has_self_receiver) {
                return check_trait_qualified_associated_call(
                    expr,
                    trait_name,
                    trait_args,
                    method_name,
                    std::move(lowered),
                    expected);
            }
            return check_trait_qualified_method_call(
                expr,
                trait_name,
                trait_args,
                method_name,
                std::move(lowered));
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        std::vector<IrExprPtr> checked_args;
        std::vector<IrType> arg_types;
        checked_args.reserve(expr.args.size());
        arg_types.reserve(expr.args.size());
        for (const auto& arg_expr : expr.args) {
            IrExprPtr arg = check_expr(*arg_expr);
            arg_types.push_back(arg->type);
            checked_args.push_back(std::move(arg));
        }

        Expr associated_expr;
        associated_expr.kind = ExprKind::Call;
        associated_expr.loc = expr.loc;
        associated_expr.name = expr.name;
        set_expr_type_args(associated_expr, combined_associated_type_args(expr));

        ImplMethodInfo generic_selected;
        bool has_generic_selected = try_select_generic_associated_impl(
            associated_expr,
            receiver_name,
            method_name,
            arg_types,
            generic_selected);

        IrType receiver_type;
        const std::vector<ImplMethodInfo>* concrete_methods = nullptr;
        if (!has_generic_selected) {
            if (!try_resolve_associated_receiver_type(expr.loc, receiver_name, receiver_type)) {
                release_temporary_borrows(borrow_mark);
                return nullptr;
            }

            auto found = associated_impls_.find(method_lookup_key(receiver_type, method_name));
            if (found == associated_impls_.end()) {
                fail(expr.loc, "unknown associated function '" + method_name + "' for type " + type_name(receiver_type));
            }
            if (!expr_type_args(associated_expr).empty()) {
                fail(expr.loc, "associated function '" + expr.name + "' does not take type arguments");
            }
            if (found->second.size() > 1) {
                fail(expr.loc, "associated function '" + method_name + "' for type " + type_name(receiver_type) + " is ambiguous");
            }
            concrete_methods = &found->second;
        }

        const ImplMethodInfo& method = has_generic_selected ? generic_selected : concrete_methods->front();
        require_impl_method_access(expr.loc, method, method_name);
        const FunctionSig& sig = method.sig;
        if (sig.params.size() != expr.args.size()) {
            fail(expr.loc, "wrong argument count for associated function '" + method_name + "'");
        }

        queue_impl_method_for_lowering(method);
        std::vector<IrExprPtr> args;
        args.reserve(expr.args.size());
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            IrExprPtr arg = coerce_checked_call_argument_or_implicit_slice(
                *expr.args[i],
                std::move(checked_args[i]),
                sig.params[i]);
            args.push_back(std::move(arg));
        }
        return finish_tracked_call(
            expr.loc,
            method_name,
            method.lowered_name,
            sig,
            std::move(args),
            borrow_mark);
    }

    IrExprPtr check_trait_object_method_call(const Expr& expr, IrExprPtr receiver, IrExprPtr lowered) {
        (void)lowered;
        auto trait_found = traits_.find(receiver->type.name);
        if (trait_found == traits_.end()) {
            fail(expr.loc, "unknown trait '" + receiver->type.name + "' in trait object method call");
        }
        const TraitInfo& trait = trait_found->second;
        std::vector<TraitObjectMethodEntry> object_methods =
            collect_trait_object_methods(trait, receiver->type.args);
        const TraitObjectMethodEntry* method_entry =
            find_trait_object_method_entry(expr.loc, object_methods, expr.name, receiver->type);
        if (!method_entry) {
            fail(expr.loc, "unknown method '" + expr.name + "' for type " + type_name(receiver->type));
        }
        const TraitInfo::Method& method = *method_entry->method;
        if (!expr_type_args(expr).empty()) {
            fail(expr.loc, "trait object method '" + expr.name + "' does not accept type arguments under dyn dispatch");
        }
        require_trait_object_method_object_safe(expr.loc, method);
        if (method.params.size() != expr.args.size() + 1) {
            fail(expr.loc, "wrong argument count for method '" + expr.name + "'");
        }

        std::uint64_t slot = 0;
        for (const auto& entry : object_methods) {
            if (&entry == method_entry) break;
            ++slot;
        }

        std::map<std::string, IrType> substitutions;
        substitutions.emplace("Self", receiver->type);
        for (std::size_t i = 0; i < method_entry->trait->generic_names.size(); ++i) {
            substitutions.emplace(method_entry->trait->generic_names[i], method_entry->trait_args[i]);
        }

        std::string previous_module = current_module_name_;
        current_module_name_ = method_entry->trait->module_name;
        std::vector<IrType> erased_params;
        IrType data_pointer = void_type(expr.loc);
        data_pointer.qualifier = TypeQualifier::Ptr;
        erased_params.push_back(data_pointer);
        std::vector<IrType> expected_args;
        expected_args.reserve(expr.args.size());
        for (std::size_t i = 1; i < method.params.size(); ++i) {
            IrType source_param = resolve_type_with_substitutions(method.params[i], substitutions);
            bool vec_view = false;
            IrType param = function_parameter_abi_type(
                method.params[i].loc,
                source_param,
                "a trait object method parameter",
                vec_view);
            erased_params.push_back(param);
            expected_args.push_back(std::move(param));
        }
        IrType result = method.has_result
            ? resolve_type_with_substitutions(method.result, substitutions)
            : void_type(method.loc);
        current_module_name_ = previous_module;

        std::vector<IrExprPtr> args;
        args.reserve(expr.args.size());
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            IrExprPtr arg = check_call_argument_for_expected(*expr.args[i], expected_args[i]);
            require_assignable(expr.args[i]->loc, expected_args[i], arg->type);
            args.push_back(std::move(arg));
        }
        return make_trait_object_call_expr(
            expr.loc,
            expr.name,
            std::move(receiver),
            slot,
            std::move(result),
            std::move(erased_params),
            std::move(args));
    }

    IrExprPtr check_method_call(const Expr& expr, IrExprPtr lowered) {
        if (expr.name == "len" && is_collection_len_method_receiver(*expr_operand(expr))) {
            require_collection_len_method_shape(expr.loc, expr_type_args(expr).size(), expr.args.size());
            return check_collection_len_expr(expr.loc, *expr_operand(expr));
        }
        if (expr.name == "is_empty" && is_collection_len_method_receiver(*expr_operand(expr))) {
            return check_collection_is_empty_method_call(expr);
        }
        if (expr.name == "as_slice") {
            if (LocalInfo* local = slice_view_local_method_receiver(expr)) {
                return check_slice_view_method_call(expr, std::move(lowered), *local);
            }
        }
        if (LocalInfo* local = vec_local_method_receiver(expr)) {
            switch (classify_local_vec_method(expr.name)) {
                case LocalVecMethod::First:
                    return check_vec_first_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Last:
                    return check_vec_last_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Get:
                    return check_vec_get_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Reserve:
                    return check_vec_reserve_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Capacity:
                    return check_vec_capacity_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Pop:
                    return check_vec_pop_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Clear:
                    return check_vec_clear_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Truncate:
                    return check_vec_truncate_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Set:
                    return check_vec_set_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Swap:
                    return check_vec_swap_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Remove:
                    return check_vec_remove_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Insert:
                    return check_vec_insert_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Contains:
                    return check_vec_contains_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::IndexOf:
                    return check_vec_index_of_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Count:
                    return check_vec_count_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::Push:
                    return check_vec_push_method_call(expr, std::move(lowered), *local);
                case LocalVecMethod::AsSlice:
                case LocalVecMethod::IsEmpty:
                case LocalVecMethod::Len:
                case LocalVecMethod::Unknown:
                    break;
            }
        }

        if (ExprPtr implicit_vec_call = rewrite_std_vec_implicit_zone_method_call(expr)) {
            return check_method_call(*implicit_vec_call, std::move(lowered));
        }
        if (ExprPtr implicit_string_call = rewrite_std_string_implicit_zone_method_call(expr)) {
            return check_method_call(*implicit_string_call, std::move(lowered));
        }

        std::size_t borrow_mark = temporary_borrow_mark();
        IrExprPtr receiver = check_expr(*expr_operand(expr));
        if (is_value_trait_object_type(receiver->type)) {
            IrExprPtr out = check_trait_object_method_call(expr, std::move(receiver), std::move(lowered));
            release_temporary_borrows(borrow_mark);
            return out;
        }

        IrType method_receiver_type = receiver->type;
        auto found = method_impls_.find(method_lookup_key(method_receiver_type, expr.name));
        if (found == method_impls_.end() && is_receiver_borrow_type(receiver->type)) {
            method_receiver_type = value_qualified_type(receiver->type);
            found = method_impls_.find(method_lookup_key(method_receiver_type, expr.name));
        }
        const ImplMethodInfo* selected = nullptr;
        ImplMethodInfo generic_selected;
        bool has_generic_selected = false;
        std::string generic_origin = generic_origin_from_expr(*expr_operand(expr));
        std::vector<IrExprPtr> generic_args;
        std::vector<IrType> generic_arg_types;
        if (found != method_impls_.end()) {
            selected = select_constrained_method_impl(expr, method_receiver_type, found->second);
        }
        if (found == method_impls_.end()) {
            generic_args.reserve(expr.args.size());
            generic_arg_types.reserve(expr.args.size());
            for (const auto& arg_expr : expr.args) {
                IrExprPtr arg = check_expr(*arg_expr);
                generic_arg_types.push_back(arg->type);
                generic_args.push_back(std::move(arg));
            }
            has_generic_selected = try_select_generic_method_impl(
                expr,
                method_receiver_type,
                generic_origin,
                generic_arg_types,
                generic_selected);
            if (!has_generic_selected) {
                if (is_local_vec_method_receiver(expr)) {
                    fail(expr.loc, local_vec_api_freeze_message(expr.name));
                }
                fail(expr.loc, "unknown method '" + expr.name + "' for type " + type_name(receiver->type));
            }
        }
        if (found != method_impls_.end() && !expr_type_args(expr).empty()) {
            fail(expr.loc, "method '" + expr.name + "' does not take type arguments");
        }
        if (found != method_impls_.end() && !selected && found->second.size() > 1) {
            fail(expr.loc, "method call '" + expr.name + "' for type " + type_name(method_receiver_type) + " is ambiguous");
        }

        const ImplMethodInfo& method = has_generic_selected
            ? generic_selected
            : (selected ? *selected : found->second.front());
        const FunctionSig& sig = method.sig;
        if (sig.params.empty()) {
            fail(expr.loc, "method '" + expr.name + "' for type " + type_name(receiver->type) + " has no receiver parameter");
        }
        if (sig.params.size() != expr.args.size() + 1) {
            fail(expr.loc, "wrong argument count for method '" + expr.name + "'");
        }

        bool mutable_receiver_borrow = false;
        if (!is_borrow_type(receiver->type) &&
            borrowed_receiver_matches_value(sig.params[0], receiver->type, mutable_receiver_borrow)) {
            ExprPtr borrow_expr = make_ast_borrow_expr(
                expr_operand(expr)->loc,
                clone_borrowable_receiver_expr(*expr_operand(expr)),
                mutable_receiver_borrow);
            if (!expr_operand(*borrow_expr)) {
                fail(expr.loc,
                     "method '" + expr.name +
                         "' with borrowed receiver requires a local binding, field access, tuple index, or constant aggregate index");
            }
            receiver = check_expr(*borrow_expr);
        }

        weaken_mut_receiver_to_shared_if_needed(*receiver, sig.params[0]);
        coerce_expr_to_expected(*receiver, sig.params[0]);
        require_assignable(expr.loc, sig.params[0], receiver->type);

        require_impl_method_access(expr.loc, method, expr.name);
        queue_impl_method_for_lowering(method);
        std::vector<IrExprPtr> args;
        args.reserve(expr.args.size() + 1);
        args.push_back(std::move(receiver));
        for (std::size_t i = 0; i < expr.args.size(); ++i) {
            IrExprPtr arg = has_generic_selected
                ? coerce_checked_call_argument_or_implicit_slice(
                      *expr.args[i],
                      std::move(generic_args[i]),
                      sig.params[i + 1])
                : check_call_argument_for_expected(*expr.args[i], sig.params[i + 1]);
            require_assignable(expr.loc, sig.params[i + 1], arg->type);
            args.push_back(std::move(arg));
        }
        require_std_vec_same_zone_method_matches_source(expr.loc, expr.name, method_receiver_type, args);
        require_std_box_same_zone_method_matches_source(expr.loc, expr.name, method_receiver_type, args);
        require_std_string_same_zone_method_matches_source(expr.loc, expr.name, method_receiver_type, args);
        return finish_tracked_call(
            expr.loc,
            expr.name,
            method.lowered_name,
            sig,
            std::move(args),
            borrow_mark);
    }

    IrExprPtr check_binary(const Expr& expr, IrExprPtr lowered) {
        IrExprPtr lhs = check_expr(*expr_left(expr));
        IrExprPtr rhs = check_expr(*expr_right(expr));
        if (is_borrow_type(lhs->type) || is_borrow_type(rhs->type)) {
            fail(expr.loc, "borrow expression result must be passed directly to a call");
        }
        coerce_integer_binary_operands(*lhs, *rhs);
        coerce_float_binary_operands(*lhs, *rhs);
        lowered->kind = IrExprKind::Binary;
        lowered->op = lower_binary_op(expr.op, expr.loc);

        switch (lowered->op) {
            case IrBinaryOp::LogicalOr:
            case IrBinaryOp::LogicalAnd:
                require_logical_operand(expr.loc, lhs->type);
                require_logical_operand(expr.loc, rhs->type);
                lowered->type = bool_type(expr.loc);
                break;
            case IrBinaryOp::Add:
            case IrBinaryOp::Sub:
            case IrBinaryOp::Mul:
            case IrBinaryOp::Div:
                require_numeric_operands(expr.loc, lhs->type, rhs->type);
                lowered->type = lhs->type;
                break;
            case IrBinaryOp::Mod:
            case IrBinaryOp::BitAnd:
            case IrBinaryOp::BitOr:
            case IrBinaryOp::BitXor:
                require_numeric_operands(expr.loc, lhs->type, rhs->type);
                require_integer_operands(expr.loc, lhs->type, rhs->type);
                lowered->type = lhs->type;
                break;
            case IrBinaryOp::Shl:
            case IrBinaryOp::Shr:
                require_integer_shift_operands(expr.loc, lhs->type, rhs->type);
                lowered->type = lhs->type;
                break;
            case IrBinaryOp::Eq:
            case IrBinaryOp::Ne:
                require_comparable_operands(expr.loc, lhs->type, rhs->type);
                lowered->type = bool_type(expr.loc);
                break;
            case IrBinaryOp::Lt:
            case IrBinaryOp::Le:
            case IrBinaryOp::Gt:
            case IrBinaryOp::Ge:
                require_numeric_operands(expr.loc, lhs->type, rhs->type);
                lowered->type = bool_type(expr.loc);
                break;
        }

        set_ir_expr_left(*lowered, std::move(lhs));
        set_ir_expr_right(*lowered, std::move(rhs));
        return lowered;
    }

    static IrStmtKind lower_stmt_kind(StmtKind kind, SourceLocation loc) {
        switch (kind) {
            case StmtKind::Block: return IrStmtKind::Block;
            case StmtKind::VarDecl: return IrStmtKind::VarDecl;
            case StmtKind::Assign: return IrStmtKind::Assign;
            case StmtKind::ExprStmt: return IrStmtKind::ExprStmt;
            case StmtKind::Return: return IrStmtKind::Return;
            case StmtKind::If: return IrStmtKind::If;
            case StmtKind::While: return IrStmtKind::While;
            case StmtKind::WhileLet: return IrStmtKind::WhileLet;
            case StmtKind::For: return IrStmtKind::ForRange;
            case StmtKind::InitWhile: return IrStmtKind::InitWhile;
            case StmtKind::Continue: return IrStmtKind::Continue;
            case StmtKind::Break: return IrStmtKind::Break;
            case StmtKind::Match: return IrStmtKind::Match;
            case StmtKind::Drop: return IrStmtKind::Drop;
        }
        fail(loc, "unsupported statement kind");
    }

    static IrBinaryOp lower_binary_op(TokenKind op, SourceLocation loc) {
        switch (op) {
            case TokenKind::PipePipe: return IrBinaryOp::LogicalOr;
            case TokenKind::AmpAmp: return IrBinaryOp::LogicalAnd;
            case TokenKind::Plus: return IrBinaryOp::Add;
            case TokenKind::Minus: return IrBinaryOp::Sub;
            case TokenKind::Star: return IrBinaryOp::Mul;
            case TokenKind::Slash: return IrBinaryOp::Div;
            case TokenKind::Percent: return IrBinaryOp::Mod;
            case TokenKind::Amp: return IrBinaryOp::BitAnd;
            case TokenKind::Pipe: return IrBinaryOp::BitOr;
            case TokenKind::Caret: return IrBinaryOp::BitXor;
            case TokenKind::LessLess: return IrBinaryOp::Shl;
            case TokenKind::GreaterGreater: return IrBinaryOp::Shr;
            case TokenKind::EqEq: return IrBinaryOp::Eq;
            case TokenKind::BangEq: return IrBinaryOp::Ne;
            case TokenKind::Less: return IrBinaryOp::Lt;
            case TokenKind::LessEq: return IrBinaryOp::Le;
            case TokenKind::Greater: return IrBinaryOp::Gt;
            case TokenKind::GreaterEq: return IrBinaryOp::Ge;
            default:
                fail(loc, "unsupported binary operator");
        }
    }

    static bool is_c_vararg_value_type(const IrType& type) {
        if (type.qualifier == TypeQualifier::Ptr ||
            type.qualifier == TypeQualifier::Ref ||
            type.qualifier == TypeQualifier::MutRef) {
            return true;
        }
        if (type.qualifier != TypeQualifier::Value) return false;
        if (is_integer_primitive(type.primitive) ||
            is_float_primitive(type.primitive) ||
            type.primitive == IrPrimitiveKind::Bool ||
            type.primitive == IrPrimitiveKind::String ||
            type.primitive == IrPrimitiveKind::Function) {
            return true;
        }
        return type.primitive == IrPrimitiveKind::Enum && type.field_types.empty();
    }

    static IrType c_vararg_promoted_type(const IrType& type) {
        if (type.qualifier != TypeQualifier::Value) return type;
        if (type.primitive == IrPrimitiveKind::Bool ||
            type.primitive == IrPrimitiveKind::I8 ||
            type.primitive == IrPrimitiveKind::U8 ||
            type.primitive == IrPrimitiveKind::I16 ||
            type.primitive == IrPrimitiveKind::U16) {
            return integer_type(IrPrimitiveKind::I32, type.loc);
        }
        if (type.primitive == IrPrimitiveKind::F32) {
            return primitive_type(IrPrimitiveKind::F64, "f64", type.loc);
        }
        return type;
    }

    static IrExprPtr promote_c_vararg(IrExprPtr arg, SourceLocation loc) {
        IrType promoted = c_vararg_promoted_type(arg->type);
        return make_cast_expr(loc, std::move(arg), promoted);
    }

    static IrType enum_tag_storage_type(SourceLocation loc) {
        return integer_type(IrPrimitiveKind::I32, loc);
    }

    static IrType enum_payload_storage_type(SourceLocation loc) {
        return integer_type(IrPrimitiveKind::U64, loc);
    }

    static IrType enum_payload_slot_storage_type(SourceLocation loc, const IrType& payload_type) {
        if (has_aggregate_enum_layout(payload_type)) return payload_type;
        return enum_payload_storage_type(loc);
    }

    static IrExpr make_pattern_integer_literal(SourceLocation loc,
                                               std::uint64_t value,
                                               bool negative,
                                               const std::string& suffix,
                                               const IrType& default_type) {
        IrExpr literal;
        literal.kind = IrExprKind::Integer;
        literal.loc = loc;
        literal.int_value = value;
        literal.int_negative = negative;
        literal.type = suffix.empty()
            ? default_type
            : integer_literal_suffix_type(suffix, loc);
        return literal;
    }

    static void coerce_labeled_break_values(const std::vector<IrStmtPtr>& statements, const std::string& label, const IrType& expected) {
        for (const auto& stmt : statements) coerce_labeled_break_values(*stmt, label, expected);
    }

    static void coerce_labeled_break_values(const std::vector<IrMatchArm>& arms, const std::string& label, const IrType& expected) {
        for (const auto& arm : arms) coerce_labeled_break_values(arm.body, label, expected);
    }

    static void coerce_labeled_break_values(IrStmt& stmt, const std::string& label, const IrType& expected) {
        if (stmt.kind == IrStmtKind::Break) {
            IrExprPtr& break_value = ir_stmt_break_value(stmt);
            if (ir_stmt_break_label(stmt) == label && break_value) {
                coerce_expr_to_expected(*break_value, expected);
                require_assignable(stmt.loc, expected, break_value->type);
            }
            return;
        }

        switch (stmt.kind) {
            case IrStmtKind::Block:
                coerce_labeled_break_values(ir_stmt_statements(stmt), label, expected);
                break;
            case IrStmtKind::If:
                coerce_labeled_break_values(ir_stmt_then_body(stmt), label, expected);
                coerce_labeled_break_values(ir_stmt_else_body(stmt), label, expected);
                break;
            case IrStmtKind::While:
            case IrStmtKind::WhileLet:
            case IrStmtKind::ForRange:
            case IrStmtKind::ForVector:
            case IrStmtKind::InitWhile:
                coerce_labeled_break_values(ir_stmt_loop_body(stmt), label, expected);
                break;
            case IrStmtKind::Match:
                coerce_labeled_break_values(ensure_ir_stmt_match_arms(stmt), label, expected);
                break;
            default:
                break;
        }
    }

    static void coerce_expr_to_expected(IrExpr& expr, const IrType& expected) {
        if (is_null_literal(expr) && expected.qualifier == TypeQualifier::Ptr) {
            expr.type = expected;
            return;
        }
        if (expr.kind == IrExprKind::Tuple && expected.primitive == IrPrimitiveKind::Tuple &&
            expr.args.size() == expected.args.size()) {
            for (std::size_t i = 0; i < expr.args.size(); ++i) {
                coerce_expr_to_expected(*expr.args[i], expected.args[i]);
                require_assignable(expr.loc, expected.args[i], expr.args[i]->type);
            }
            expr.type = expected;
            return;
        }
        if (expr.kind == IrExprKind::Tuple && expected.primitive == IrPrimitiveKind::Struct &&
            expr.args.size() == expected.field_types.size()) {
            for (std::size_t i = 0; i < expr.args.size(); ++i) {
                coerce_expr_to_expected(*expr.args[i], expected.field_types[i]);
                require_assignable(expr.loc, expected.field_types[i], expr.args[i]->type);
            }
            expr.type = expected;
            return;
        }
        if (expr.kind == IrExprKind::Vector && expected.primitive == IrPrimitiveKind::Vector &&
            expected.args.size() == 1) {
            if (expected.array_size != 0 && expr.args.size() > expected.array_size) {
                fail(expr.loc,
                     "vector literal has " + std::to_string(expr.args.size()) +
                     " elements but storage capacity is " + std::to_string(expected.array_size));
            }
            for (auto& item : expr.args) {
                coerce_expr_to_expected(*item, expected.args[0]);
                require_assignable(expr.loc, expected.args[0], item->type);
            }
            expr.type = make_vector_storage_type(
                expr.loc,
                expected.args[0],
                expected.array_size == 0 ? expr.args.size() : expected.array_size
            );
            return;
        }
        if (expr.kind == IrExprKind::Vector && expected.primitive == IrPrimitiveKind::Array &&
            expected.args.size() == 1) {
            if (expr.args.size() != expected.array_size) {
                fail(expr.loc,
                     "array literal has " + std::to_string(expr.args.size()) +
                     " elements but type expects " + std::to_string(expected.array_size));
            }
            for (auto& item : expr.args) {
                coerce_expr_to_expected(*item, expected.args[0]);
                require_assignable(expr.loc, expected.args[0], item->type);
            }
            expr.type = expected;
            return;
        }
        if (expr.kind == IrExprKind::Match) {
            for (auto& arm : ir_expr_match_arms(expr)) {
                if (is_diverging_control_flow_value(*arm.value)) continue;
                coerce_expr_to_expected(*arm.value, expected);
                require_assignable(arm.loc, expected, arm.value->type);
            }
            expr.type = expected;
            return;
        }
        if (expr.kind == IrExprKind::If) {
            if (!is_diverging_control_flow_value(*ir_expr_if_then_value(expr))) {
                coerce_expr_to_expected(*ir_expr_if_then_value(expr), expected);
                require_assignable(expr.loc, expected, ir_expr_if_then_value(expr)->type);
            }
            if (!is_diverging_control_flow_value(*ir_expr_if_else_value(expr))) {
                coerce_expr_to_expected(*ir_expr_if_else_value(expr), expected);
                require_assignable(expr.loc, expected, ir_expr_if_else_value(expr)->type);
            }
            expr.type = expected;
            return;
        }
        if (expr.kind == IrExprKind::Block) {
            if (ir_expr_block_value(expr) &&
                ir_expr_block_label(expr).empty() &&
                is_diverging_control_flow_value(*ir_expr_block_value(expr))) {
                expr.type = expected;
                return;
            }
            coerce_expr_to_expected(*ir_expr_block_value(expr), expected);
            require_assignable(expr.loc, expected, ir_expr_block_value(expr)->type);
            if (!ir_expr_block_label(expr).empty()) {
                coerce_labeled_break_values(
                    ir_expr_block_body(expr), ir_expr_block_label(expr), expected);
            }
            expr.type = expected;
            return;
        }
        if (is_float_literal(expr)) {
            coerce_float_expr_to_expected(expr, expected);
            return;
        }
        if (!is_integer_literal(expr)) return;
        IrType target = literal_value_type_for_expected(expected);
        if (!is_value_integer_type(target)) return;
        if (!integer_literal_fits(expr, expected)) {
            fail(expr.loc, "integer literal " + integer_literal_name(expr) + " is out of range for " + type_name(target));
        }
        expr.type = target;
    }

    static void coerce_float_expr_to_expected(IrExpr& expr, const IrType& expected) {
        if (!is_float_literal(expr)) return;
        IrType target = literal_value_type_for_expected(expected);
        if (!is_value_float_type(target)) return;
        expr.type = target;
    }

    static void coerce_integer_binary_operands(IrExpr& lhs, IrExpr& rhs) {
        if (!is_value_integer_type(lhs.type) || !is_value_integer_type(rhs.type)) return;
        if (same_type(lhs.type, rhs.type)) {
            if (is_integer_literal(lhs)) coerce_expr_to_expected(lhs, lhs.type);
            if (is_integer_literal(rhs)) coerce_expr_to_expected(rhs, rhs.type);
            return;
        }
        if (is_integer_literal(lhs) && !is_integer_literal(rhs)) {
            coerce_expr_to_expected(lhs, rhs.type);
        } else if (!is_integer_literal(lhs) && is_integer_literal(rhs)) {
            coerce_expr_to_expected(rhs, lhs.type);
        }
    }

    static void coerce_float_binary_operands(IrExpr& lhs, IrExpr& rhs) {
        if (!is_value_float_type(lhs.type) || !is_value_float_type(rhs.type)) return;
        if (same_type(lhs.type, rhs.type)) return;
        if (is_float_literal(lhs) && !is_float_literal(rhs)) {
            coerce_float_expr_to_expected(lhs, rhs.type);
        } else if (!is_float_literal(lhs) && is_float_literal(rhs)) {
            coerce_float_expr_to_expected(rhs, lhs.type);
        }
    }

    static void coerce_condition_to_bool(SourceLocation loc, IrExprPtr& expr) {
        if (expr->type.qualifier == TypeQualifier::Value && expr->type.primitive == IrPrimitiveKind::Bool) return;
        if (!is_value_integer_type(expr->type)) {
            require_boolish(loc, expr->type);
        }

        auto condition = std::make_unique<IrExpr>();
        condition->kind = IrExprKind::Binary;
        condition->loc = loc;
        condition->op = IrBinaryOp::Ne;
        condition->type = bool_type(loc);
        set_ir_expr_right(*condition, make_integer_zero(loc, expr->type));
        set_ir_expr_left(*condition, std::move(expr));
        expr = std::move(condition);
    }

    [[noreturn]] static void fail(SourceLocation loc, const std::string& message) {
        throw CompileError(where(loc) + ": " + message);
    }
};

IrProgram check_program(const Program& program, SemaOptions options) {
    SemanticChecker checker(program, options);
    return checker.check();
}

} // namespace ari
