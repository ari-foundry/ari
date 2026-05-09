#pragma once

#include "ast.hpp"

#include <vector>

namespace ari {

bool pattern_has_binding(const Pattern& pattern);
bool pattern_contains_or(const Pattern& pattern);
std::vector<Pattern> expand_or_pattern_alternatives(const Pattern& pattern);

} // namespace ari
