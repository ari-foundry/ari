#include "attribute_semantics.hpp"

namespace ari {

const Attribute* find_attribute(const std::vector<Attribute>& attributes, const std::string& name) {
    for (const auto& attr : attributes) {
        if (attr.name == name) return &attr;
    }
    return nullptr;
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

} // namespace ari
