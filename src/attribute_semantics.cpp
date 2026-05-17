#include "attribute_semantics.hpp"

#include "common.hpp"

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
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
        if (!item.payloads.empty()) {
            fail(item.loc, "attribute '@repr(C)' currently supports only fieldless enums");
        }
    }
}

} // namespace ari
