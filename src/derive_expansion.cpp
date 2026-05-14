#include "meta_expansion.hpp"

#include "ast_builders.hpp"
#include "common.hpp"

#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

[[noreturn]] void fail_derive_expansion(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

std::string parse_derive_path(const Attribute& attr, std::size_t& index) {
    if (index >= attr.args.size() || attr.args[index].kind != TokenKind::Identifier) {
        fail_derive_expansion(attr.loc, "attribute '@derive' expects trait names separated by commas");
    }
    std::string path = attr.args[index].text;
    ++index;
    while (index < attr.args.size() && attr.args[index].kind == TokenKind::ColonColon) {
        ++index;
        if (index >= attr.args.size() || attr.args[index].kind != TokenKind::Identifier) {
            fail_derive_expansion(attr.loc, "attribute '@derive' expects trait names separated by commas");
        }
        path += "::" + attr.args[index].text;
        ++index;
    }
    return path;
}

std::string canonical_derive_name(SourceLocation loc, const std::string& path) {
    if (path == "Debug" || path == "std::Debug") return "Debug";
    if (path == "Copy" || path == "std::Copy") return "Copy";
    if (path == "Clone" || path == "std::Clone") return "Clone";
    if (path == "Default" || path == "std::Default") return "Default";
    fail_derive_expansion(loc, "unsupported derive '" + path + "'; supported derives: Debug, Copy, Clone, Default");
}

std::vector<std::string> derive_names(const std::vector<Attribute>& attributes) {
    std::vector<std::string> names;
    std::set<std::string> seen;
    for (const auto& attr : attributes) {
        if (attr.name != "derive") continue;
        std::size_t index = 0;
        while (index < attr.args.size()) {
            SourceLocation name_loc = attr.args[index].loc;
            std::string name = canonical_derive_name(name_loc, parse_derive_path(attr, index));
            if (!seen.insert(name).second) {
                fail_derive_expansion(attr.loc, "duplicate derive '" + name + "'");
            }
            names.push_back(std::move(name));
            if (index >= attr.args.size()) break;
            if (attr.args[index].kind != TokenKind::Comma) {
                fail_derive_expansion(attr.loc, "attribute '@derive' expects trait names separated by commas");
            }
            ++index;
            if (index >= attr.args.size()) {
                fail_derive_expansion(attr.loc, "attribute '@derive' expects trait names separated by commas");
            }
        }
    }
    return names;
}

TypeRef simple_type_ref(const std::string& name, SourceLocation loc) {
    TypeRef type;
    type.name = name;
    type.loc = loc;
    return type;
}

std::vector<TypeRef> generic_type_args(const std::vector<GenericParam>& generics) {
    std::vector<TypeRef> args;
    args.reserve(generics.size());
    for (const auto& generic : generics) {
        args.push_back(simple_type_ref(generic.name, generic.loc));
    }
    return args;
}

std::string qualify_generated_name(const std::string& module_name, const std::string& name) {
    if (module_name.empty()) return name;
    return module_name + "::" + name;
}

StmtPtr make_return_stmt(SourceLocation loc, ExprPtr value) {
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = StmtKind::Return;
    stmt->loc = loc;
    stmt->expr = std::move(value);
    return stmt;
}

FunctionDecl make_derived_method(const std::string& module_name,
                                 const std::string& name,
                                 std::vector<Param> params,
                                 ExprPtr return_value,
                                 SourceLocation loc) {
    FunctionDecl method;
    method.name = qualify_generated_name(module_name, name);
    method.module_name = module_name;
    method.loc = loc;
    method.params = std::move(params);
    method.return_type = simple_type_ref("Self", loc);
    method.has_return_type = true;
    method.has_body = true;
    method.body.push_back(make_return_stmt(loc, std::move(return_value)));
    return method;
}

FunctionDecl make_clone_method(const std::string& module_name, SourceLocation loc) {
    Param self;
    self.name = "self";
    self.type = simple_type_ref("Self", loc);
    std::vector<Param> params;
    params.push_back(std::move(self));
    return make_derived_method(module_name, "clone", std::move(params), make_ast_name_expr(loc, "self"), loc);
}

ImplDecl make_trait_derive_impl(const std::string& trait_name,
                                const std::string& type_name,
                                const std::string& module_name,
                                const std::vector<GenericParam>& generics,
                                SourceLocation loc) {
    ImplDecl impl;
    impl.module_name = module_name;
    impl.has_trait = true;
    impl.generics = generics;
    impl.trait_type = simple_type_ref("std::" + trait_name, loc);
    impl.for_type = simple_type_ref(type_name, loc);
    impl.for_type.args = generic_type_args(generics);
    return impl;
}

ImplDecl make_clone_derive_impl(const std::string& type_name,
                                const std::string& module_name,
                                const std::vector<GenericParam>& generics,
                                SourceLocation loc) {
    ImplDecl impl = make_trait_derive_impl("Clone", type_name, module_name, generics, loc);
    impl.methods.push_back(make_clone_method(module_name, loc));
    return impl;
}

bool is_default_trait_ref(const TypeRef& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.args.empty() &&
           !type.is_dyn_object &&
           !type.nullable &&
           !type.is_macro_invocation &&
           !type.has_associated_projection &&
           (type.name == "Default" || type.name == "std::Default");
}

bool type_ref_mentions_name(const TypeRef& type, const std::string& name) {
    if (type.qualifier == TypeQualifier::Value && type.name == name && type.args.empty()) return true;
    for (const auto& arg : type.args) {
        if (type_ref_mentions_name(arg, name)) return true;
    }
    return false;
}

bool generic_is_used_in_fields(const std::vector<StructField>& fields, const std::string& generic_name) {
    for (const auto& field : fields) {
        if (type_ref_mentions_name(field.type, generic_name)) return true;
    }
    return false;
}

std::vector<GenericParam> default_impl_generics(const StructDecl& decl) {
    std::vector<GenericParam> generics = decl.generics;
    for (auto& generic : generics) {
        if (!generic_is_used_in_fields(decl.fields, generic.name)) continue;
        if (!generic.has_constraint) {
            generic.constraint = simple_type_ref("std::Default", generic.loc);
            generic.has_constraint = true;
            continue;
        }
        if (!is_default_trait_ref(generic.constraint)) {
            fail_derive_expansion(
                generic.loc,
                "Default derive cannot add the required std::Default bound to generic parameter '" +
                    generic.name + "' because it already has a constraint");
        }
    }
    return generics;
}

ExprPtr make_default_call(TypeRef type, SourceLocation loc) {
    auto call = make_ast_call_expr(loc, "std::Default::default", nullptr, {});
    std::vector<TypeRef> type_args;
    type_args.push_back(std::move(type));
    set_expr_type_args(*call, std::move(type_args));
    return call;
}

ExprPtr make_default_struct_value(const StructDecl& decl) {
    std::vector<ExprPtr> values;
    values.reserve(decl.fields.size());
    for (const auto& field : decl.fields) {
        values.push_back(make_default_call(field.type, field.loc));
    }

    if (decl.tuple_struct) {
        ExprPtr value = make_ast_call_expr(decl.loc, decl.name, nullptr, std::move(values));
        set_expr_type_args(*value, generic_type_args(decl.generics));
        return value;
    }

    std::vector<std::string> field_names;
    field_names.reserve(decl.fields.size());
    for (const auto& field : decl.fields) field_names.push_back(field.name);
    return make_ast_struct_literal_expr(
        decl.loc,
        decl.name,
        generic_type_args(decl.generics),
        std::move(field_names),
        std::move(values));
}

FunctionDecl make_default_method(const StructDecl& decl) {
    return make_derived_method(
        decl.module_name,
        "default",
        {},
        make_default_struct_value(decl),
        decl.loc);
}

ImplDecl make_default_derive_impl(const StructDecl& decl) {
    std::vector<GenericParam> generics = default_impl_generics(decl);
    ImplDecl impl = make_trait_derive_impl("Default", decl.name, decl.module_name, generics, decl.loc);
    impl.methods.push_back(make_default_method(decl));
    return impl;
}

} // namespace

std::vector<ImplDecl> expand_derive_impls_for_struct(const StructDecl& decl) {
    std::vector<ImplDecl> impls;
    for (const auto& name : derive_names(decl.attributes)) {
        if (name == "Debug") {
            impls.push_back(make_trait_derive_impl("Debug", decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Copy") {
            impls.push_back(make_trait_derive_impl("Copy", decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Clone") {
            impls.push_back(make_clone_derive_impl(decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Default") {
            impls.push_back(make_default_derive_impl(decl));
        }
    }
    return impls;
}

std::vector<ImplDecl> expand_derive_impls_for_enum(const EnumDecl& decl) {
    std::vector<ImplDecl> impls;
    for (const auto& name : derive_names(decl.attributes)) {
        if (name == "Debug") {
            impls.push_back(make_trait_derive_impl("Debug", decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Copy") {
            impls.push_back(make_trait_derive_impl("Copy", decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Clone") {
            impls.push_back(make_clone_derive_impl(decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Default") {
            fail_derive_expansion(
                decl.loc,
                "Default derive for enums requires an explicit default case marker, which is not supported yet");
        }
    }
    return impls;
}

} // namespace ari
