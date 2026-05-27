#include "declaration_index_dump.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

struct DeclarationLine {
    std::string key;
    std::string text;
};

std::string module_name_text(const std::string& module_name) {
    return module_name.empty() ? "<root>" : module_name;
}

std::string visibility_text(bool is_public) {
    return is_public ? "pub" : "private";
}

std::string bool_text(bool value) {
    return value ? "true" : "false";
}

std::string qualifier_text(TypeQualifier qualifier) {
    switch (qualifier) {
        case TypeQualifier::Value: return "";
        case TypeQualifier::Own: return "own ";
        case TypeQualifier::Ref: return "ref ";
        case TypeQualifier::MutRef: return "ref mut ";
        case TypeQualifier::Ptr: return "ptr ";
    }
    return "";
}

std::string type_text(const TypeRef& type) {
    std::string text = qualifier_text(type.qualifier);
    if (type.is_union_by) {
        text += "union by ";
        for (std::size_t i = 0; i < type.union_by_selector.size(); ++i) {
            if (i > 0) text += ".";
            text += type.union_by_selector[i];
        }
        text += " { ";
        for (std::size_t i = 0; i < type.union_by_arm_names.size(); ++i) {
            if (i > 0) text += ", ";
            text += type.union_by_arm_names[i] + " => ";
            if (i < type.union_by_arm_types.size()) text += type_text(type.union_by_arm_types[i]);
        }
        text += " }";
        if (type.nullable) text += "?";
        return text;
    }
    if (type.is_dyn_object) text += "dyn ";
    text += type.name.empty() ? "<infer>" : type.name;
    if (!type.args.empty()) {
        text += "[";
        for (std::size_t i = 0; i < type.args.size(); ++i) {
            if (i > 0) text += ", ";
            text += type_text(type.args[i]);
        }
        text += "]";
    }
    if (type.has_associated_projection) text += "::" + type.associated_projection;
    if (type.array_size != 0) text += "[" + std::to_string(type.array_size) + "]";
    if (type.nullable) text += "?";
    if (type.is_macro_invocation) text += "!";
    return text;
}

std::string generic_text(const std::vector<GenericParam>& generics) {
    if (generics.empty()) return "[]";
    std::string text = "[";
    for (std::size_t i = 0; i < generics.size(); ++i) {
        if (i > 0) text += ", ";
        text += generics[i].name;
        if (generics[i].has_constraint) text += ": " + type_text(generics[i].constraint);
    }
    text += "]";
    return text;
}

std::string params_text(const std::vector<Param>& params) {
    if (params.empty()) return "[]";
    std::string text = "[";
    for (std::size_t i = 0; i < params.size(); ++i) {
        if (i > 0) text += ", ";
        const Param& param = params[i];
        text += param.name.empty() ? "<pattern>" : param.name;
        text += ":";
        text += type_text(param.type);
    }
    text += "]";
    return text;
}

std::string fields_text(const std::vector<StructField>& fields) {
    if (fields.empty()) return "[]";
    std::string text = "[";
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) text += ", ";
        const StructField& field = fields[i];
        text += field.name + ":";
        if (field.mutable_field) text += "mut ";
        text += type_text(field.type);
    }
    text += "]";
    return text;
}

std::string enum_cases_text(const std::vector<EnumCase>& cases) {
    if (cases.empty()) return "[]";
    std::string text = "[";
    for (std::size_t i = 0; i < cases.size(); ++i) {
        if (i > 0) text += ", ";
        const EnumCase& enum_case = cases[i];
        text += enum_case.name;
        if (!enum_case.payloads.empty()) {
            text += "(";
            for (std::size_t payload = 0; payload < enum_case.payloads.size(); ++payload) {
                if (payload > 0) text += ", ";
                text += type_text(enum_case.payloads[payload]);
            }
            text += ")";
        }
    }
    text += "]";
    return text;
}

std::string associated_types_text(const std::vector<TraitDecl::AssociatedType>& associated_types) {
    if (associated_types.empty()) return "[]";
    std::string text = "[";
    for (std::size_t i = 0; i < associated_types.size(); ++i) {
        if (i > 0) text += ", ";
        text += associated_types[i].name;
    }
    text += "]";
    return text;
}

std::string associated_witnesses_text(const std::vector<ImplDecl::AssociatedTypeWitness>& witnesses) {
    if (witnesses.empty()) return "[]";
    std::string text = "[";
    for (std::size_t i = 0; i < witnesses.size(); ++i) {
        if (i > 0) text += ", ";
        text += witnesses[i].name + "=" + type_text(witnesses[i].type);
    }
    text += "]";
    return text;
}

std::string methods_text(const std::vector<FunctionDecl>& methods) {
    if (methods.empty()) return "[]";
    std::string text = "[";
    for (std::size_t i = 0; i < methods.size(); ++i) {
        if (i > 0) text += ", ";
        const FunctionDecl& method = methods[i];
        text += method.name;
        text += "(" + params_text(method.params) + ")";
        text += "->";
        text += method.has_return_type ? type_text(method.return_type) : "void";
        text += method.has_body ? "{body}" : "{decl}";
    }
    text += "]";
    return text;
}

std::string location_text(SourceLocation loc, const std::map<std::string, std::string>& path_by_module,
                          const std::string& module_name) {
    if (span_has_source(loc.span) && span_has_valid_order(loc.span)) {
        SourceLocation span_loc = source_location_for_span(loc.span);
        if (has_source_name(span_loc)) loc = std::move(span_loc);
    } else if (loc.source_name.empty() && valid_source_id(loc.source_id)) {
        if (const SourceFile* source = find_source_file(loc.source_id)) {
            loc.source_name = source->display_name;
        }
    }
    auto it = path_by_module.find(module_name);
    std::string path = !loc.source_name.empty()
                           ? loc.source_name
                           : (it == path_by_module.end() ? "<unknown>" : it->second);
    std::string text = path + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.column);
    if (valid_source_id(loc.source_id)) text += " source_id=" + source_id_text(loc.source_id);
    if (has_byte_span(loc)) {
        Span span = span_from_location(loc);
        text += " bytes=" + std::to_string(span.start) + ".." + std::to_string(span.end);
    }
    return text;
}

std::string location_key(SourceLocation loc) {
    std::string key = std::to_string(loc.line) + "\t" + std::to_string(loc.column);
    if (valid_source_id(loc.source_id)) key += "\t" + source_id_text(loc.source_id);
    if (has_byte_span(loc)) {
        Span span = span_from_location(loc);
        key += "\t" + std::to_string(span.start) + "\t" + std::to_string(span.end);
    }
    return key;
}

std::string decl_key(const std::string& module_name, const std::string& kind,
                     const std::string& name, SourceLocation loc) {
    return module_name + "\t" + kind + "\t" + name + "\t" + location_key(loc);
}

void add_line(std::vector<DeclarationLine>& lines,
              const std::string& module_name,
              const std::string& kind,
              const std::string& name,
              SourceLocation loc,
              std::string text) {
    lines.push_back(DeclarationLine{decl_key(module_name, kind, name, loc), std::move(text)});
}

std::string base_decl_text(const std::map<std::string, std::string>& path_by_module,
                           const std::string& module_name,
                           const std::string& kind,
                           const std::string& name,
                           bool is_public,
                           SourceLocation loc) {
    std::ostringstream out;
    out << "  Decl module=" << module_name_text(module_name)
        << " kind=" << kind
        << " name=" << name
        << " visibility=" << visibility_text(is_public)
        << " loc=" << location_text(loc, path_by_module, module_name);
    return out.str();
}

} // namespace

std::string dump_declaration_index(const Program& program,
                                   const ModuleMetadata& metadata,
                                   const std::string& source_name) {
    std::map<std::string, std::string> path_by_module;
    for (const auto& source : metadata.sources) {
        path_by_module[source.module_name] = source.path;
    }

    std::vector<DeclarationLine> lines;
    for (const auto& decl : program.module_imports) {
        std::string text = base_decl_text(path_by_module, decl.module_name, "module-import", decl.name,
                                          decl.is_public, decl.loc);
        text += " local=" + decl.local_name;
        add_line(lines, decl.module_name, "module-import", decl.name, decl.loc, std::move(text));
    }
    for (const auto& decl : program.uses) {
        std::string name = decl.is_glob ? decl.path + "::*" : decl.path;
        std::string text = base_decl_text(path_by_module, decl.module_name, "use", name,
                                          decl.is_public, decl.loc);
        text += " alias=" + decl.alias;
        add_line(lines, decl.module_name, "use", name, decl.loc, std::move(text));
    }
    for (const auto& decl : program.type_aliases) {
        std::string text = base_decl_text(path_by_module, decl.module_name, "type", decl.name,
                                          decl.is_public, decl.loc);
        text += " generics=" + generic_text(decl.generics);
        text += " target=" + type_text(decl.target);
        add_line(lines, decl.module_name, "type", decl.name, decl.loc, std::move(text));
    }
    for (const auto& decl : program.structs) {
        std::string text = base_decl_text(path_by_module, decl.module_name, "struct", decl.name,
                                          decl.is_public, decl.loc);
        text += " generics=" + generic_text(decl.generics);
        text += " tuple=" + bool_text(decl.tuple_struct);
        text += " fields=" + fields_text(decl.fields);
        add_line(lines, decl.module_name, "struct", decl.name, decl.loc, std::move(text));
    }
    for (const auto& decl : program.enums) {
        std::string text = base_decl_text(path_by_module, decl.module_name, "enum", decl.name,
                                          decl.is_public, decl.loc);
        text += " generics=" + generic_text(decl.generics);
        text += " cases=" + enum_cases_text(decl.cases);
        add_line(lines, decl.module_name, "enum", decl.name, decl.loc, std::move(text));
    }
    for (const auto& decl : program.traits) {
        std::string text = base_decl_text(path_by_module, decl.module_name, "trait", decl.name,
                                          decl.is_public, decl.loc);
        text += " generics=" + generic_text(decl.generics);
        text += " associated_types=" + associated_types_text(decl.associated_types);
        text += " methods=" + methods_text(decl.methods);
        add_line(lines, decl.module_name, "trait", decl.name, decl.loc, std::move(text));
    }
    for (const auto& decl : program.impls) {
        std::string name = decl.has_trait ? type_text(decl.trait_type) + " for " + type_text(decl.for_type)
                                          : type_text(decl.for_type);
        std::string text = base_decl_text(path_by_module, decl.module_name,
                                          decl.has_trait ? "trait-impl" : "impl",
                                          name, decl.is_public, decl.loc);
        text += " generics=" + generic_text(decl.generics);
        text += " associated_witnesses=" + associated_witnesses_text(decl.associated_type_witnesses);
        text += " methods=" + methods_text(decl.methods);
        add_line(lines, decl.module_name, decl.has_trait ? "trait-impl" : "impl", name, decl.loc, std::move(text));
    }
    for (const auto& decl : program.functions) {
        std::string kind = decl.is_extern ? "extern-" + decl.extern_abi + "-fn" : (decl.meta ? "meta-fn" : "fn");
        std::string text = base_decl_text(path_by_module, decl.module_name, kind, decl.name,
                                          decl.is_public, decl.loc);
        text += " generics=" + generic_text(decl.generics);
        text += " params=" + params_text(decl.params);
        text += " return=" + (decl.has_return_type ? type_text(decl.return_type) : "void");
        text += " body=" + bool_text(decl.has_body);
        if (decl.is_variadic) text += " variadic=true";
        if (!decl.extern_link_name.empty()) text += " link_name=" + decl.extern_link_name;
        add_line(lines, decl.module_name, kind, decl.name, decl.loc, std::move(text));
    }

    std::sort(lines.begin(), lines.end(), [](const auto& left, const auto& right) {
        if (left.key != right.key) return left.key < right.key;
        return left.text < right.text;
    });

    std::ostringstream out;
    out << "DeclarationIndex source=" << source_name
        << " modules=" << metadata.sources.size()
        << " declarations=" << lines.size()
        << " resolver_surface=imports,uses,declarations"
        << " order=module-kind-name-location-text\n";
    for (const auto& line : lines) {
        out << line.text << "\n";
    }
    return out.str();
}

} // namespace ari
