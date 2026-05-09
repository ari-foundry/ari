#include "module_ast_summary.hpp"

#include "module_metadata.hpp"
#include "module_path.hpp"

#include <cstdint>
#include <sstream>
#include <string>

namespace ari {
namespace {

void append_field(std::ostringstream& out, const std::string& value) {
    out << value.size() << ':' << value << ';';
}

void append_bool(std::ostringstream& out, bool value) {
    out << (value ? "1;" : "0;");
}

void append_count(std::ostringstream& out, std::uint64_t value) {
    out << value << ';';
}

void append_qualifier(std::ostringstream& out, TypeQualifier qualifier) {
    switch (qualifier) {
        case TypeQualifier::Value: append_field(out, "value"); return;
        case TypeQualifier::Own: append_field(out, "own"); return;
        case TypeQualifier::Ref: append_field(out, "ref"); return;
        case TypeQualifier::MutRef: append_field(out, "ref mut"); return;
        case TypeQualifier::Ptr: append_field(out, "ptr"); return;
    }
}

void append_type(std::ostringstream& out, const TypeRef& type) {
    append_qualifier(out, type.qualifier);
    append_field(out, type.name);
    append_bool(out, type.is_dyn_object);
    append_bool(out, type.nullable);
    append_count(out, type.array_size);
    append_count(out, type.args.size());
    for (const auto& arg : type.args) append_type(out, arg);
}

void append_generics(std::ostringstream& out, const std::vector<GenericParam>& generics) {
    append_count(out, generics.size());
    for (const auto& generic : generics) {
        append_field(out, generic.name);
        append_bool(out, generic.has_constraint);
        if (generic.has_constraint) append_type(out, generic.constraint);
    }
}

void append_attributes(std::ostringstream& out, const std::vector<Attribute>& attributes) {
    append_count(out, attributes.size());
    for (const auto& attr : attributes) {
        append_field(out, attr.name);
        append_bool(out, attr.has_args);
        append_count(out, attr.args.size());
        for (const auto& token : attr.args) {
            append_count(out, static_cast<std::uint64_t>(token.kind));
            append_field(out, token.text);
        }
    }
}

void append_function_signature(std::ostringstream& out, const FunctionDecl& fn) {
    append_field(out, fn.module_name);
    append_field(out, fn.name);
    append_bool(out, fn.meta);
    append_bool(out, fn.is_extern);
    append_bool(out, fn.is_public);
    append_bool(out, fn.is_variadic);
    append_field(out, fn.extern_abi);
    append_field(out, fn.extern_link_name);
    append_bool(out, fn.has_return_type);
    append_bool(out, fn.has_body);
    append_generics(out, fn.generics);
    append_attributes(out, fn.attributes);
    append_count(out, fn.params.size());
    for (const auto& param : fn.params) {
        append_field(out, param.name);
        append_bool(out, param.has_pattern);
        append_type(out, param.type);
    }
    if (fn.has_return_type) append_type(out, fn.return_type);
}

std::string declaration_summary_payload(const Program& program) {
    std::ostringstream out;
    out << "ari-ast-decls-v1;";

    append_count(out, program.uses.size());
    for (const auto& decl : program.uses) {
        append_field(out, decl.module_name);
        append_field(out, decl.path);
        append_field(out, decl.alias);
        append_bool(out, decl.is_public);
        append_bool(out, decl.is_glob);
    }

    append_count(out, program.module_imports.size());
    for (const auto& decl : program.module_imports) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_field(out, decl.local_name);
        append_bool(out, decl.is_public);
    }

    append_count(out, program.modules.size());
    for (const auto& decl : program.modules) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
    }

    append_count(out, program.constants.size());
    for (const auto& decl : program.constants) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_type(out, decl.type);
    }

    append_count(out, program.functions.size());
    for (const auto& fn : program.functions) append_function_signature(out, fn);

    append_count(out, program.structs.size());
    for (const auto& decl : program.structs) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_bool(out, decl.tuple_struct);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.fields.size());
        for (const auto& field : decl.fields) {
            append_field(out, field.name);
            append_bool(out, field.mutable_field);
            append_type(out, field.type);
        }
    }

    append_count(out, program.enums.size());
    for (const auto& decl : program.enums) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.cases.size());
        for (const auto& item : decl.cases) {
            append_field(out, item.name);
            append_count(out, item.payloads.size());
            for (const auto& payload : item.payloads) append_type(out, payload);
        }
    }

    append_count(out, program.traits.size());
    for (const auto& decl : program.traits) {
        append_field(out, decl.module_name);
        append_field(out, decl.name);
        append_bool(out, decl.is_public);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        append_count(out, decl.methods.size());
        for (const auto& method : decl.methods) append_function_signature(out, method);
    }

    append_count(out, program.impls.size());
    for (const auto& decl : program.impls) {
        append_field(out, decl.module_name);
        append_bool(out, decl.is_public);
        append_bool(out, decl.has_trait);
        append_generics(out, decl.generics);
        append_attributes(out, decl.attributes);
        if (decl.has_trait) append_type(out, decl.trait_type);
        append_type(out, decl.for_type);
        append_count(out, decl.methods.size());
        for (const auto& method : decl.methods) append_function_signature(out, method);
    }

    return out.str();
}

} // namespace

ModuleCacheAstSummary make_module_cache_ast_summary(const std::string& path,
                                                    const std::string& content_hash,
                                                    const std::vector<std::string>& module_path,
                                                    const Program& program,
                                                    bool is_root) {
    ModuleCacheAstSummary summary;
    summary.module_name = join_qualified_path(module_path);
    summary.path = path;
    summary.content_hash = content_hash;
    summary.declaration_summary = declaration_summary_payload(program);
    summary.declaration_hash = module_metadata_source_hash(summary.declaration_summary);
    summary.is_root = is_root;
    summary.use_count = program.uses.size();
    summary.module_import_count = program.module_imports.size();
    summary.module_decl_count = program.modules.size();
    summary.constant_count = program.constants.size();
    summary.function_count = program.functions.size();
    summary.struct_count = program.structs.size();
    summary.enum_count = program.enums.size();
    summary.trait_count = program.traits.size();
    summary.impl_count = program.impls.size();
    return summary;
}

} // namespace ari
