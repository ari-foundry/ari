#pragma once

#include "ast.hpp"

#include <optional>
#include <string>
#include <vector>

namespace ari {

const Attribute* find_attribute(const std::vector<Attribute>& attributes, const std::string& name);
bool attribute_has_single_identifier_argument(const Attribute& attr, const std::string& expected);
std::optional<bool> attribute_single_bool_argument_value(const Attribute& attr);

} // namespace ari
