#pragma once

#include "ast.hpp"

#include <set>
#include <string>

namespace ari {

bool cfg_attribute_enabled(const Attribute& attr, const std::set<std::string>& features = {});

} // namespace ari
