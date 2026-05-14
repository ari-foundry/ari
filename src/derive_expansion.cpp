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
    if (path == "Eq" || path == "std::Eq") return "Eq";
    if (path == "PartialEq" || path == "std::PartialEq") return "PartialEq";
    if (path == "Ord" || path == "std::Ord") return "Ord";
    if (path == "PartialOrd" || path == "std::PartialOrd") return "PartialOrd";
    fail_derive_expansion(
        loc,
        "unsupported derive '" + path +
            "'; supported derives: Debug, Copy, Clone, Default, Eq, PartialEq, Ord, PartialOrd");
}

struct DeriveRequest {
    std::string name;
    std::string default_case;
    bool has_default_case = false;
    SourceLocation loc;
    SourceLocation default_case_loc;
};

DeriveRequest parse_derive_request(const Attribute& attr, std::size_t& index) {
    SourceLocation name_loc = attr.args[index].loc;
    DeriveRequest request;
    request.name = canonical_derive_name(name_loc, parse_derive_path(attr, index));
    request.loc = name_loc;
    if (index < attr.args.size() && attr.args[index].kind == TokenKind::LParen) {
        SourceLocation marker_loc = attr.args[index].loc;
        if (request.name != "Default") {
            fail_derive_expansion(marker_loc, request.name + " derive does not accept arguments");
        }
        ++index;
        if (index >= attr.args.size() || attr.args[index].kind != TokenKind::Identifier) {
            fail_derive_expansion(marker_loc, "Default derive case marker expects one enum case name");
        }
        request.default_case = attr.args[index].text;
        request.default_case_loc = attr.args[index].loc;
        request.has_default_case = true;
        ++index;
        if (index >= attr.args.size() || attr.args[index].kind != TokenKind::RParen) {
            fail_derive_expansion(marker_loc, "Default derive case marker expects one enum case name");
        }
        ++index;
    }
    return request;
}

std::vector<DeriveRequest> derive_requests(const std::vector<Attribute>& attributes) {
    std::vector<DeriveRequest> requests;
    std::set<std::string> seen;
    for (const auto& attr : attributes) {
        if (attr.name != "derive") continue;
        std::size_t index = 0;
        while (index < attr.args.size()) {
            DeriveRequest request = parse_derive_request(attr, index);
            if (!seen.insert(request.name).second) {
                fail_derive_expansion(attr.loc, "duplicate derive '" + request.name + "'");
            }
            requests.push_back(std::move(request));
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
    return requests;
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

StmtPtr make_if_return_stmt(SourceLocation loc, ExprPtr condition, bool value) {
    auto stmt = std::make_unique<Stmt>();
    stmt->kind = StmtKind::If;
    stmt->loc = loc;
    stmt->condition = std::move(condition);
    std::vector<StmtPtr> then_body;
    then_body.push_back(make_return_stmt(loc, make_ast_bool_expr(loc, value)));
    set_stmt_then_body(*stmt, std::move(then_body));
    return stmt;
}

FunctionDecl make_derived_method_with_body(const std::string& module_name,
                                           const std::string& name,
                                           std::vector<Param> params,
                                           TypeRef return_type,
                                           std::vector<StmtPtr> body,
                                           SourceLocation loc) {
    FunctionDecl method;
    method.name = qualify_generated_name(module_name, name);
    method.module_name = module_name;
    method.loc = loc;
    method.params = std::move(params);
    method.return_type = std::move(return_type);
    method.has_return_type = true;
    method.has_body = true;
    method.body = std::move(body);
    return method;
}

FunctionDecl make_derived_method(const std::string& module_name,
                                 const std::string& name,
                                 std::vector<Param> params,
                                 TypeRef return_type,
                                 ExprPtr return_value,
                                 SourceLocation loc) {
    std::vector<StmtPtr> body;
    body.push_back(make_return_stmt(loc, std::move(return_value)));
    return make_derived_method_with_body(
        module_name,
        name,
        std::move(params),
        std::move(return_type),
        std::move(body),
        loc);
}

TypeRef self_type_ref(SourceLocation loc) {
    return simple_type_ref("Self", loc);
}

TypeRef bool_type_ref(SourceLocation loc) {
    return simple_type_ref("bool", loc);
}

FunctionDecl make_clone_method(const std::string& module_name, SourceLocation loc) {
    Param self;
    self.name = "self";
    self.type = self_type_ref(loc);
    std::vector<Param> params;
    params.push_back(std::move(self));
    return make_derived_method(
        module_name,
        "clone",
        std::move(params),
        self_type_ref(loc),
        make_ast_name_expr(loc, "self"),
        loc);
}

ImplDecl make_trait_derive_impl(const std::string& trait_name,
                                const std::string& type_name,
                                const std::string& module_name,
                                const std::vector<GenericParam>& generics,
                                SourceLocation loc,
                                std::vector<TypeRef> trait_args = {}) {
    ImplDecl impl;
    impl.module_name = module_name;
    impl.has_trait = true;
    impl.generics = generics;
    impl.trait_type = simple_type_ref("std::" + trait_name, loc);
    impl.trait_type.args = std::move(trait_args);
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

bool is_matching_trait_ref(const TypeRef& type, const std::string& trait_name, const std::string& generic_name) {
    if (type.qualifier != TypeQualifier::Value ||
        type.args.size() != 1 ||
        type.is_dyn_object ||
        type.nullable ||
        type.is_macro_invocation ||
        type.has_associated_projection ||
        (type.name != trait_name && type.name != "std::" + trait_name)) {
        return false;
    }
    const TypeRef& arg = type.args.front();
    return arg.qualifier == TypeQualifier::Value &&
           arg.name == generic_name &&
           arg.args.empty() &&
           !arg.is_dyn_object &&
           !arg.nullable &&
           !arg.is_macro_invocation &&
           !arg.has_associated_projection;
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

bool generic_is_used_in_types(const std::vector<TypeRef>& types, const std::string& generic_name) {
    for (const auto& type : types) {
        if (type_ref_mentions_name(type, generic_name)) return true;
    }
    return false;
}

bool generic_is_used_in_enum_payloads(const EnumDecl& decl, const std::string& generic_name) {
    for (const auto& item : decl.cases) {
        if (generic_is_used_in_types(item.payloads, generic_name)) return true;
    }
    return false;
}

TypeRef trait_self_constraint(const std::string& trait_name, const GenericParam& generic) {
    TypeRef constraint = simple_type_ref("std::" + trait_name, generic.loc);
    constraint.args.push_back(simple_type_ref(generic.name, generic.loc));
    return constraint;
}

template <typename IsGenericUsed>
std::vector<GenericParam> default_impl_generics_for(const std::vector<GenericParam>& source_generics,
                                                    IsGenericUsed is_generic_used) {
    std::vector<GenericParam> generics = source_generics;
    for (auto& generic : generics) {
        if (!is_generic_used(generic.name)) continue;
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

std::vector<GenericParam> default_impl_generics(const StructDecl& decl) {
    return default_impl_generics_for(decl.generics, [&](const std::string& generic_name) {
        return generic_is_used_in_fields(decl.fields, generic_name);
    });
}

std::vector<GenericParam> default_impl_generics(const EnumDecl& decl, const EnumCase& item) {
    return default_impl_generics_for(decl.generics, [&](const std::string& generic_name) {
        return generic_is_used_in_types(item.payloads, generic_name);
    });
}

std::vector<GenericParam> field_trait_impl_generics(const StructDecl& decl,
                                                    const std::string& trait_name,
                                                    const std::string& derive_name) {
    std::vector<GenericParam> generics = decl.generics;
    for (auto& generic : generics) {
        if (!generic_is_used_in_fields(decl.fields, generic.name)) continue;
        if (!generic.has_constraint) {
            generic.constraint = trait_self_constraint(trait_name, generic);
            generic.has_constraint = true;
            continue;
        }
        if (!is_matching_trait_ref(generic.constraint, trait_name, generic.name)) {
            fail_derive_expansion(
                generic.loc,
                derive_name + " derive cannot add the required std::" + trait_name +
                    " bound to generic parameter '" + generic.name +
                    "' because it already has a constraint");
        }
    }
    return generics;
}

std::vector<GenericParam> enum_payload_trait_impl_generics(const EnumDecl& decl,
                                                           const std::string& trait_name,
                                                           const std::string& derive_name) {
    std::vector<GenericParam> generics = decl.generics;
    for (auto& generic : generics) {
        if (!generic_is_used_in_enum_payloads(decl, generic.name)) continue;
        if (!generic.has_constraint) {
            generic.constraint = trait_self_constraint(trait_name, generic);
            generic.has_constraint = true;
            continue;
        }
        if (!is_matching_trait_ref(generic.constraint, trait_name, generic.name)) {
            fail_derive_expansion(
                generic.loc,
                derive_name + " derive cannot add the required std::" + trait_name +
                    " bound to generic parameter '" + generic.name +
                    "' because it already has a constraint");
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
        self_type_ref(decl.loc),
        make_default_struct_value(decl),
        decl.loc);
}

ImplDecl make_default_derive_impl(const StructDecl& decl) {
    std::vector<GenericParam> generics = default_impl_generics(decl);
    ImplDecl impl = make_trait_derive_impl("Default", decl.name, decl.module_name, generics, decl.loc);
    impl.methods.push_back(make_default_method(decl));
    return impl;
}

const EnumCase& default_enum_case(const EnumDecl& decl, const DeriveRequest& derive) {
    if (!derive.has_default_case) {
        fail_derive_expansion(
            derive.loc,
            "Default derive for enums requires a case marker such as @derive(Default(CaseName))");
    }
    for (const auto& item : decl.cases) {
        if (item.name == derive.default_case) return item;
    }
    fail_derive_expansion(
        derive.default_case_loc,
        "Default derive case '" + derive.default_case + "' does not name a case of enum '" + decl.name + "'");
}

ExprPtr make_default_enum_value(const EnumDecl& decl, const EnumCase& item) {
    std::vector<ExprPtr> values;
    values.reserve(item.payloads.size());
    for (const auto& payload : item.payloads) {
        values.push_back(make_default_call(payload, item.loc));
    }
    ExprPtr value = make_ast_call_expr(item.loc, item.name, nullptr, std::move(values));
    set_expr_type_args(*value, generic_type_args(decl.generics));
    return value;
}

FunctionDecl make_default_method(const EnumDecl& decl, const EnumCase& item) {
    return make_derived_method(
        decl.module_name,
        "default",
        {},
        self_type_ref(decl.loc),
        make_default_enum_value(decl, item),
        decl.loc);
}

ImplDecl make_default_derive_impl(const EnumDecl& decl, const DeriveRequest& derive) {
    const EnumCase& item = default_enum_case(decl, derive);
    std::vector<GenericParam> generics = default_impl_generics(decl, item);
    ImplDecl impl = make_trait_derive_impl("Default", decl.name, decl.module_name, generics, decl.loc);
    impl.methods.push_back(make_default_method(decl, item));
    return impl;
}

ExprPtr make_field_access(SourceLocation loc, const std::string& base_name, const StructField& field) {
    ExprPtr base = make_ast_name_expr(loc, base_name);
    if (!field.name.empty()) {
        bool numeric = true;
        for (char ch : field.name) {
            if (ch < '0' || ch > '9') {
                numeric = false;
                break;
            }
        }
        if (numeric) {
            return make_ast_tuple_index_expr(loc, std::move(base), static_cast<std::uint64_t>(std::stoull(field.name)));
        }
    }
    return make_ast_field_access_expr(loc, std::move(base), field.name);
}

ExprPtr make_equality_call(const std::string& trait_name, const StructField& field) {
    std::vector<ExprPtr> args;
    args.push_back(make_field_access(field.loc, "self", field));
    args.push_back(make_field_access(field.loc, "other", field));

    ExprPtr call = make_ast_call_expr(field.loc, "std::" + trait_name + "::eq", nullptr, std::move(args));
    std::vector<TypeRef> trait_args;
    trait_args.push_back(field.type);
    set_expr_receiver_type_args(*call, std::move(trait_args));
    return call;
}

ExprPtr make_equality_struct_value(const StructDecl& decl, const std::string& trait_name) {
    ExprPtr value = make_ast_bool_expr(decl.loc, true);
    for (const auto& field : decl.fields) {
        value = make_ast_binary_expr(
            field.loc,
            TokenKind::AmpAmp,
            std::move(value),
            make_equality_call(trait_name, field));
    }
    return value;
}

std::vector<Param> binary_self_params(SourceLocation loc) {
    Param self;
    self.name = "self";
    self.type = self_type_ref(loc);
    Param other;
    other.name = "other";
    other.type = self_type_ref(loc);
    std::vector<Param> params;
    params.push_back(std::move(self));
    params.push_back(std::move(other));
    return params;
}

FunctionDecl make_equality_method(const StructDecl& decl, const std::string& trait_name) {
    return make_derived_method(
        decl.module_name,
        "eq",
        binary_self_params(decl.loc),
        bool_type_ref(decl.loc),
        make_equality_struct_value(decl, trait_name),
        decl.loc);
}

TypeRef declared_type_ref(const std::string& type_name,
                          const std::vector<GenericParam>& generics,
                          SourceLocation loc) {
    TypeRef type = simple_type_ref(type_name, loc);
    type.args = generic_type_args(generics);
    return type;
}

ImplDecl make_equality_derive_impl(const StructDecl& decl, const std::string& trait_name) {
    std::vector<GenericParam> generics = field_trait_impl_generics(decl, trait_name, trait_name);
    std::vector<TypeRef> trait_args;
    trait_args.push_back(declared_type_ref(decl.name, decl.generics, decl.loc));
    ImplDecl impl = make_trait_derive_impl(trait_name, decl.name, decl.module_name, generics, decl.loc, std::move(trait_args));
    impl.methods.push_back(make_equality_method(decl, trait_name));
    return impl;
}

ExprPtr make_equality_enum_value(SourceLocation loc) {
    return make_ast_binary_expr(
        loc,
        TokenKind::EqEq,
        make_ast_name_expr(loc, "self"),
        make_ast_name_expr(loc, "other"));
}

bool enum_has_payloads(const EnumDecl& decl) {
    for (const auto& item : decl.cases) {
        if (!item.payloads.empty()) return true;
    }
    return false;
}

std::string enum_payload_binding_name(const std::string& prefix, std::size_t index) {
    return "__ari_derive_" + prefix + "_" + std::to_string(index);
}

Pattern make_binding_pattern(SourceLocation loc, std::string name) {
    Pattern pattern;
    pattern.kind = PatternKind::Binding;
    pattern.loc = loc;
    pattern.payload_name = std::move(name);
    return pattern;
}

Pattern make_wildcard_pattern(SourceLocation loc) {
    Pattern pattern;
    pattern.kind = PatternKind::Wildcard;
    pattern.loc = loc;
    return pattern;
}

Pattern make_enum_case_pattern(const EnumCase& item, const std::string& binding_prefix) {
    Pattern pattern;
    pattern.kind = PatternKind::EnumCase;
    pattern.case_name = item.name;
    pattern.loc = item.loc;
    if (item.payloads.empty()) return pattern;

    pattern.has_payload_pattern = true;
    if (item.payloads.size() == 1) {
        std::string name = enum_payload_binding_name(binding_prefix, 0);
        pattern.has_payload_binding = true;
        pattern.payload_name = name;
        pattern.payload_pattern = std::make_unique<Pattern>(make_binding_pattern(item.loc, std::move(name)));
        return pattern;
    }

    Pattern tuple;
    tuple.kind = PatternKind::Tuple;
    tuple.loc = item.loc;
    tuple.elements.reserve(item.payloads.size());
    for (std::size_t i = 0; i < item.payloads.size(); ++i) {
        tuple.elements.push_back(make_binding_pattern(item.loc, enum_payload_binding_name(binding_prefix, i)));
    }
    pattern.payload_pattern = std::make_unique<Pattern>(std::move(tuple));
    return pattern;
}

ExprPtr make_named_equality_call(const std::string& trait_name,
                                 TypeRef type,
                                 SourceLocation loc,
                                 const std::string& left_name,
                                 const std::string& right_name) {
    std::vector<ExprPtr> args;
    args.push_back(make_ast_name_expr(loc, left_name));
    args.push_back(make_ast_name_expr(loc, right_name));

    ExprPtr call = make_ast_call_expr(loc, "std::" + trait_name + "::eq", nullptr, std::move(args));
    std::vector<TypeRef> trait_args;
    trait_args.push_back(std::move(type));
    set_expr_receiver_type_args(*call, std::move(trait_args));
    return call;
}

ExprPtr make_equality_enum_payload_value(const EnumCase& item,
                                         const std::string& trait_name,
                                         const std::string& left_prefix,
                                         const std::string& right_prefix) {
    ExprPtr value = make_ast_bool_expr(item.loc, true);
    for (std::size_t i = 0; i < item.payloads.size(); ++i) {
        value = make_ast_binary_expr(
            item.loc,
            TokenKind::AmpAmp,
            std::move(value),
            make_named_equality_call(
                trait_name,
                item.payloads[i],
                item.loc,
                enum_payload_binding_name(left_prefix, i),
                enum_payload_binding_name(right_prefix, i)));
    }
    return value;
}

ExprPtr make_equality_enum_other_match(const EnumCase& item,
                                       const std::string& trait_name,
                                       const std::string& left_prefix,
                                       const std::string& right_prefix) {
    std::vector<ExprMatchArm> arms;
    ExprMatchArm same_case;
    same_case.pattern = make_enum_case_pattern(item, right_prefix);
    same_case.loc = item.loc;
    same_case.value = make_equality_enum_payload_value(item, trait_name, left_prefix, right_prefix);
    arms.push_back(std::move(same_case));

    ExprMatchArm different_case;
    different_case.pattern = make_wildcard_pattern(item.loc);
    different_case.loc = item.loc;
    different_case.value = make_ast_bool_expr(item.loc, false);
    arms.push_back(std::move(different_case));
    return make_ast_match_expr(item.loc, make_ast_name_expr(item.loc, "other"), std::move(arms));
}

ExprPtr make_equality_enum_match_value(const EnumDecl& decl, const std::string& trait_name) {
    std::vector<ExprMatchArm> arms;
    arms.reserve(decl.cases.size());
    for (std::size_t i = 0; i < decl.cases.size(); ++i) {
        const auto& item = decl.cases[i];
        std::string left_prefix = "left_" + std::to_string(i);
        std::string right_prefix = "right_" + std::to_string(i);
        ExprMatchArm arm;
        arm.pattern = make_enum_case_pattern(item, left_prefix);
        arm.loc = item.loc;
        arm.value = make_equality_enum_other_match(item, trait_name, left_prefix, right_prefix);
        arms.push_back(std::move(arm));
    }
    return make_ast_match_expr(decl.loc, make_ast_name_expr(decl.loc, "self"), std::move(arms));
}

FunctionDecl make_equality_method(const EnumDecl& decl, const std::string& trait_name) {
    ExprPtr return_value = enum_has_payloads(decl)
                               ? make_equality_enum_match_value(decl, trait_name)
                               : make_equality_enum_value(decl.loc);
    return make_derived_method(
        decl.module_name,
        "eq",
        binary_self_params(decl.loc),
        bool_type_ref(decl.loc),
        std::move(return_value),
        decl.loc);
}

ImplDecl make_equality_derive_impl(const EnumDecl& decl, const std::string& trait_name) {
    std::vector<GenericParam> generics = enum_payload_trait_impl_generics(decl, trait_name, trait_name);
    std::vector<TypeRef> trait_args;
    trait_args.push_back(declared_type_ref(decl.name, decl.generics, decl.loc));
    ImplDecl impl =
        make_trait_derive_impl(trait_name, decl.name, decl.module_name, generics, decl.loc, std::move(trait_args));
    impl.methods.push_back(make_equality_method(decl, trait_name));
    return impl;
}

ExprPtr make_ordering_call(const std::string& trait_name,
                           const StructField& field,
                           const std::string& left_base,
                           const std::string& right_base) {
    std::vector<ExprPtr> args;
    args.push_back(make_field_access(field.loc, left_base, field));
    args.push_back(make_field_access(field.loc, right_base, field));

    ExprPtr call = make_ast_call_expr(field.loc, "std::" + trait_name + "::lt", nullptr, std::move(args));
    std::vector<TypeRef> trait_args;
    trait_args.push_back(field.type);
    set_expr_receiver_type_args(*call, std::move(trait_args));
    return call;
}

std::vector<StmtPtr> make_ordering_method_body(const StructDecl& decl, const std::string& trait_name) {
    std::vector<StmtPtr> body;
    for (const auto& field : decl.fields) {
        body.push_back(make_if_return_stmt(
            field.loc,
            make_ordering_call(trait_name, field, "self", "other"),
            true));
        body.push_back(make_if_return_stmt(
            field.loc,
            make_ordering_call(trait_name, field, "other", "self"),
            false));
    }
    body.push_back(make_return_stmt(decl.loc, make_ast_bool_expr(decl.loc, false)));
    return body;
}

FunctionDecl make_ordering_method(const StructDecl& decl, const std::string& trait_name) {
    return make_derived_method_with_body(
        decl.module_name,
        "lt",
        binary_self_params(decl.loc),
        bool_type_ref(decl.loc),
        make_ordering_method_body(decl, trait_name),
        decl.loc);
}

ImplDecl make_ordering_derive_impl(const StructDecl& decl, const std::string& trait_name) {
    std::vector<GenericParam> generics = field_trait_impl_generics(decl, trait_name, trait_name);
    std::vector<TypeRef> trait_args;
    trait_args.push_back(declared_type_ref(decl.name, decl.generics, decl.loc));
    ImplDecl impl =
        make_trait_derive_impl(trait_name, decl.name, decl.module_name, generics, decl.loc, std::move(trait_args));
    impl.methods.push_back(make_ordering_method(decl, trait_name));
    return impl;
}

[[noreturn]] void fail_enum_ordering_derive(const EnumDecl& decl, const std::string& trait_name) {
    fail_derive_expansion(
        decl.loc,
        trait_name + " derive for enums requires an explicit enum ordering policy, which is not supported yet");
}

} // namespace

std::vector<ImplDecl> expand_derive_impls_for_struct(const StructDecl& decl) {
    std::vector<ImplDecl> impls;
    for (const auto& derive : derive_requests(decl.attributes)) {
        const std::string& name = derive.name;
        if (name == "Debug") {
            impls.push_back(make_trait_derive_impl("Debug", decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Copy") {
            impls.push_back(make_trait_derive_impl("Copy", decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Clone") {
            impls.push_back(make_clone_derive_impl(decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Default") {
            if (derive.has_default_case) {
                fail_derive_expansion(
                    derive.default_case_loc,
                    "Default derive case marker is only valid for enum derives");
            }
            impls.push_back(make_default_derive_impl(decl));
        } else if (name == "Eq") {
            impls.push_back(make_equality_derive_impl(decl, "Eq"));
        } else if (name == "PartialEq") {
            impls.push_back(make_equality_derive_impl(decl, "PartialEq"));
        } else if (name == "Ord") {
            impls.push_back(make_ordering_derive_impl(decl, "Ord"));
        } else if (name == "PartialOrd") {
            impls.push_back(make_ordering_derive_impl(decl, "PartialOrd"));
        }
    }
    return impls;
}

std::vector<ImplDecl> expand_derive_impls_for_enum(const EnumDecl& decl) {
    std::vector<ImplDecl> impls;
    for (const auto& derive : derive_requests(decl.attributes)) {
        const std::string& name = derive.name;
        if (name == "Debug") {
            impls.push_back(make_trait_derive_impl("Debug", decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Copy") {
            impls.push_back(make_trait_derive_impl("Copy", decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Clone") {
            impls.push_back(make_clone_derive_impl(decl.name, decl.module_name, decl.generics, decl.loc));
        } else if (name == "Default") {
            impls.push_back(make_default_derive_impl(decl, derive));
        } else if (name == "Eq") {
            impls.push_back(make_equality_derive_impl(decl, "Eq"));
        } else if (name == "PartialEq") {
            impls.push_back(make_equality_derive_impl(decl, "PartialEq"));
        } else if (name == "Ord") {
            fail_enum_ordering_derive(decl, "Ord");
        } else if (name == "PartialOrd") {
            fail_enum_ordering_derive(decl, "PartialOrd");
        }
    }
    return impls;
}

} // namespace ari
