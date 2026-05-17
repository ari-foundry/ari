#pragma once

#include "ast.hpp"

#include <optional>
#include <set>
#include <string>
#include <vector>

namespace ari {

const Attribute* find_attribute(const std::vector<Attribute>& attributes, const std::string& name);
bool is_builtin_attribute_name(const std::string& name);
bool attribute_has_single_identifier_argument(const Attribute& attr, const std::string& expected);
std::optional<bool> attribute_single_bool_argument_value(const Attribute& attr);
void validate_builtin_attribute(const Attribute& attr,
                                const std::string& target_kind,
                                const std::set<std::string>& cfg_features,
                                const std::string& target_triple);
void validate_repr_c_struct_fields(const StructDecl& decl);
void validate_repr_c_enum_cases(const EnumDecl& decl);

} // namespace ari
