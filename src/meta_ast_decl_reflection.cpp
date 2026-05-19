#include "meta_ast_decl_reflection.hpp"

#include "module_path.hpp"
#include "type_semantics.hpp"

#include <string>
#include <utility>
#include <vector>

namespace ari {
namespace {

struct DeclSummary {
    std::string kind;
    std::string name;
    bool is_public = false;
    std::string return_type;
    std::string trait_type;
    std::size_t generic_count = 0;
    std::size_t param_count = 0;
    std::size_t field_count = 0;
    std::size_t case_count = 0;
    std::size_t method_count = 0;
    std::size_t associated_type_count = 0;
    std::vector<std::string> generics;
    std::vector<MetaAstNameTypeSummary> params;
    std::vector<MetaAstNameTypeSummary> fields;
    std::vector<MetaAstEnumCaseSummary> cases;
    std::vector<MetaAstCallableSummary> methods;
    std::vector<MetaAstNameTypeSummary> associated_types;
};

struct DeclShape {
    std::size_t generic_count = 0;
    std::size_t param_count = 0;
    std::size_t field_count = 0;
    std::size_t case_count = 0;
    std::size_t method_count = 0;
    std::size_t associated_type_count = 0;
};

struct DeclMembers {
    std::string return_type;
    std::string trait_type;
    std::vector<std::string> generics;
    std::vector<MetaAstNameTypeSummary> params;
    std::vector<MetaAstNameTypeSummary> fields;
    std::vector<MetaAstEnumCaseSummary> cases;
    std::vector<MetaAstCallableSummary> methods;
    std::vector<MetaAstNameTypeSummary> associated_types;
};

void append_summary(std::vector<DeclSummary>& summaries,
                    std::string kind,
                    std::string name,
                    bool is_public,
                    DeclShape shape = {},
                    DeclMembers members = {}) {
    summaries.push_back({
        std::move(kind),
        std::move(name),
        is_public,
        std::move(members.return_type),
        std::move(members.trait_type),
        shape.generic_count,
        shape.param_count,
        shape.field_count,
        shape.case_count,
        shape.method_count,
        shape.associated_type_count,
        std::move(members.generics),
        std::move(members.params),
        std::move(members.fields),
        std::move(members.cases),
        std::move(members.methods),
        std::move(members.associated_types)
    });
}

void add_shape_to_input(MetaAstDeclInput& input, const DeclSummary& summary) {
    input.generic_count += summary.generic_count;
    input.param_count += summary.param_count;
    input.field_count += summary.field_count;
    input.case_count += summary.case_count;
    input.method_count += summary.method_count;
    input.associated_type_count += summary.associated_type_count;
}

void append_members_to_input(MetaAstDeclInput& input, const DeclSummary& summary) {
    input.generics.insert(input.generics.end(), summary.generics.begin(), summary.generics.end());
    input.params.insert(input.params.end(), summary.params.begin(), summary.params.end());
    input.fields.insert(input.fields.end(), summary.fields.begin(), summary.fields.end());
    input.cases.insert(input.cases.end(), summary.cases.begin(), summary.cases.end());
    input.methods.insert(input.methods.end(), summary.methods.begin(), summary.methods.end());
    input.associated_types.insert(
        input.associated_types.end(),
        summary.associated_types.begin(),
        summary.associated_types.end());
}

std::vector<std::string> generic_names(const std::vector<GenericParam>& generics) {
    std::vector<std::string> names;
    names.reserve(generics.size());
    for (const auto& generic : generics) names.push_back(generic.name);
    return names;
}

std::vector<MetaAstNameTypeSummary> param_summaries(const std::vector<Param>& params) {
    std::vector<MetaAstNameTypeSummary> summaries;
    summaries.reserve(params.size());
    for (const auto& param : params) {
        summaries.push_back({param.name, type_ref_key(param.type)});
    }
    return summaries;
}

std::vector<MetaAstNameTypeSummary> field_summaries(const std::vector<StructField>& fields) {
    std::vector<MetaAstNameTypeSummary> summaries;
    summaries.reserve(fields.size());
    for (const auto& field : fields) {
        summaries.push_back({field.name, type_ref_key(field.type)});
    }
    return summaries;
}

std::vector<MetaAstEnumCaseSummary> case_summaries(const std::vector<EnumCase>& cases) {
    std::vector<MetaAstEnumCaseSummary> summaries;
    summaries.reserve(cases.size());
    for (const auto& item : cases) {
        MetaAstEnumCaseSummary summary;
        summary.name = item.name;
        summary.payload_types.reserve(item.payloads.size());
        for (const auto& payload : item.payloads) {
            summary.payload_types.push_back(type_ref_key(payload));
        }
        summaries.push_back(std::move(summary));
    }
    return summaries;
}

MetaAstCallableSummary callable_summary(const FunctionDecl& decl) {
    MetaAstCallableSummary summary;
    summary.name = qualified_basename(decl.name);
    if (decl.has_return_type) summary.return_type = type_ref_key(decl.return_type);
    summary.generic_count = decl.generics.size();
    summary.params = param_summaries(decl.params);
    return summary;
}

std::vector<MetaAstCallableSummary> method_summaries(const std::vector<FunctionDecl>& methods) {
    std::vector<MetaAstCallableSummary> summaries;
    summaries.reserve(methods.size());
    for (const auto& method : methods) summaries.push_back(callable_summary(method));
    return summaries;
}

std::vector<MetaAstNameTypeSummary> trait_associated_type_summaries(
    const std::vector<TraitDecl::AssociatedType>& associated_types) {
    std::vector<MetaAstNameTypeSummary> summaries;
    summaries.reserve(associated_types.size());
    for (const auto& item : associated_types) summaries.push_back({item.name, ""});
    return summaries;
}

std::vector<MetaAstNameTypeSummary> impl_associated_type_summaries(
    const std::vector<ImplDecl::AssociatedTypeWitness>& associated_types) {
    std::vector<MetaAstNameTypeSummary> summaries;
    summaries.reserve(associated_types.size());
    for (const auto& item : associated_types) summaries.push_back({item.name, type_ref_key(item.type)});
    return summaries;
}

} // namespace

MetaAstDeclInput summarize_meta_ast_decl_input(const std::vector<Token>& input_tokens,
                                               const Program& parsed_input) {
    std::vector<DeclSummary> summaries;
    for (const auto& decl : parsed_input.uses) {
        append_summary(summaries, "use", decl.alias.empty() ? qualified_basename(decl.path) : decl.alias, decl.is_public);
    }
    for (const auto& decl : parsed_input.module_imports) {
        append_summary(summaries, "module_import", qualified_basename(decl.name), decl.is_public);
    }
    for (const auto& decl : parsed_input.modules) {
        append_summary(summaries, "module", qualified_basename(decl.name), decl.is_public);
    }
    for (const auto& decl : parsed_input.item_macros) {
        append_summary(summaries, "item_macro", decl.name, decl.is_public);
    }
    for (const auto& decl : parsed_input.constants) {
        append_summary(summaries, "const", qualified_basename(decl.name), decl.is_public);
    }
    for (const auto& decl : parsed_input.type_aliases) {
        DeclMembers members;
        members.return_type = type_ref_key(decl.target);
        members.generics = generic_names(decl.generics);
        append_summary(
            summaries,
            "type",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size()},
            std::move(members));
    }
    for (const auto& decl : parsed_input.functions) {
        DeclMembers members;
        if (decl.has_return_type) members.return_type = type_ref_key(decl.return_type);
        members.generics = generic_names(decl.generics);
        members.params = param_summaries(decl.params);
        append_summary(
            summaries,
            decl.meta ? "meta_fn" : "fn",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size(), decl.params.size()},
            std::move(members));
    }
    for (const auto& decl : parsed_input.structs) {
        DeclMembers members;
        members.generics = generic_names(decl.generics);
        members.fields = field_summaries(decl.fields);
        append_summary(
            summaries,
            "struct",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size(), 0, decl.fields.size()},
            std::move(members));
    }
    for (const auto& decl : parsed_input.enums) {
        DeclMembers members;
        members.generics = generic_names(decl.generics);
        members.cases = case_summaries(decl.cases);
        append_summary(
            summaries,
            "enum",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size(), 0, 0, decl.cases.size()},
            std::move(members));
    }
    for (const auto& decl : parsed_input.traits) {
        DeclMembers members;
        members.generics = generic_names(decl.generics);
        members.methods = method_summaries(decl.methods);
        members.associated_types = trait_associated_type_summaries(decl.associated_types);
        append_summary(
            summaries,
            "trait",
            qualified_basename(decl.name),
            decl.is_public,
            DeclShape{decl.generics.size(), 0, 0, 0, decl.methods.size(), decl.associated_types.size()},
            std::move(members));
    }
    for (const auto& decl : parsed_input.impls) {
        DeclMembers members;
        if (decl.has_trait) members.trait_type = type_ref_key(decl.trait_type);
        members.generics = generic_names(decl.generics);
        members.methods = method_summaries(decl.methods);
        members.associated_types = impl_associated_type_summaries(decl.associated_type_witnesses);
        append_summary(
            summaries,
            "impl",
            type_ref_key(decl.for_type),
            decl.is_public,
            DeclShape{decl.generics.size(), 0, 0, 0, decl.methods.size(), decl.associated_type_witnesses.size()},
            std::move(members));
    }

    MetaAstDeclInput input;
    input.tokens = input_tokens;
    input.count = summaries.size();
    for (const auto& summary : summaries) {
        add_shape_to_input(input, summary);
        append_members_to_input(input, summary);
    }
    if (summaries.empty()) {
        input.kind = "empty";
        return input;
    }
    if (summaries.size() == 1) {
        input.kind = summaries.front().kind;
        input.name = summaries.front().name;
        input.is_public = summaries.front().is_public;
        input.return_type = summaries.front().return_type;
        input.trait_type = summaries.front().trait_type;
        return input;
    }
    input.kind = "mixed";
    return input;
}

} // namespace ari
