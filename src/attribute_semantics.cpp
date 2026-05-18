#include "attribute_semantics.hpp"

#include "borrow_return_semantics.hpp"
#include "cfg_eval.hpp"
#include "common.hpp"

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

void require_attribute_args(const Attribute& attr) {
    if (!attr.has_args || attr.args.empty()) {
        fail(attr.loc, "attribute '@" + attr.name + "' expects arguments");
    }
}

void reject_attribute_args(const Attribute& attr) {
    if (attr.has_args) {
        fail(attr.loc, "attribute '@" + attr.name + "' does not take arguments");
    }
}

bool type_ref_contains_own(const TypeRef& type) {
    if (type.qualifier == TypeQualifier::Own) return true;
    for (const auto& arg : type.args) {
        if (type_ref_contains_own(arg)) return true;
    }
    return false;
}

} // namespace

const Attribute* find_attribute(const std::vector<Attribute>& attributes, const std::string& name) {
    for (const auto& attr : attributes) {
        if (attr.name == name) return &attr;
    }
    return nullptr;
}

bool is_builtin_attribute_name(const std::string& name) {
    return name == "derive" ||
           name == "deprecated" ||
           name == "export" ||
           name == "no_mangle" ||
           name == "borrow_return" ||
           name == "repr" ||
           name == "test" ||
           name == "cfg";
}

bool attribute_has_single_identifier_argument(const Attribute& attr, const std::string& expected) {
    return attr.args.size() == 1 &&
           attr.args[0].kind == TokenKind::Identifier &&
           attr.args[0].text == expected;
}

std::optional<bool> attribute_single_bool_argument_value(const Attribute& attr) {
    if (attr.args.size() != 1) return std::nullopt;
    if (attr.args[0].kind == TokenKind::KwTrue) return true;
    if (attr.args[0].kind == TokenKind::KwFalse) return false;
    return std::nullopt;
}

void validate_builtin_attribute(const Attribute& attr,
                                const std::string& target_kind,
                                const std::set<std::string>& cfg_features,
                                const std::string& target_triple) {
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
        (void)cfg_attribute_enabled(attr, cfg_features, target_triple);
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

void validate_repr_c_struct_fields(const StructDecl& decl) {
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

void validate_repr_c_enum_cases(const EnumDecl& decl) {
    const Attribute* repr = find_attribute(decl.attributes, "repr");
    if (!repr) return;
    for (const auto& item : decl.cases) {
        if (!item.payloads.empty() && !decl.generics.empty()) {
            fail(item.loc,
                 "attribute '@repr(C)' generic payload enums are not supported yet; use a non-generic enum or an explicit C wrapper");
        }
        for (const auto& payload : item.payloads) {
            if (type_ref_contains_own(payload)) {
                fail(payload.loc,
                     "attribute '@repr(C)' enum payloads cannot use own; expose ownership through an explicit ptr/ref ABI");
            }
        }
    }
}

} // namespace ari
