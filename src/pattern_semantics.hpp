#pragma once

#include "ast.hpp"

#include <cstddef>
#include <vector>

namespace ari {

Pattern clone_pattern(const Pattern& pattern);
bool pattern_has_binding(const Pattern& pattern);
bool pattern_contains_or(const Pattern& pattern);
bool pattern_contains_array_pattern(const Pattern& pattern);
bool runtime_sequence_array_pattern_is_irrefutable(const Pattern& pattern);
std::vector<Pattern> expand_or_pattern_alternatives(const Pattern& pattern);
std::size_t tuple_pattern_field_index(const Pattern& pattern,
                                      std::size_t field_count,
                                      std::size_t pattern_index);
const Pattern* positional_product_field_pattern(const Pattern& pattern,
                                                std::size_t field_count,
                                                std::size_t field_index);

} // namespace ari
