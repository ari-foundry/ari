#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace ari {

struct ForPatternStructInfo {
    std::string name;
    bool tuple_struct = false;
};

struct ForPatternValidationHooks {
    std::function<ForPatternStructInfo(SourceLocation, const std::string&, const IrType&)>
        require_struct_pattern_type;
    std::function<std::size_t(SourceLocation, const IrType&, const std::string&)>
        struct_field_index;
};

Pattern clone_pattern(const Pattern& pattern);
bool pattern_has_binding(const Pattern& pattern);
bool pattern_has_reference_binding_mode(const Pattern& pattern);
bool pattern_has_mutable_reference_binding_mode(const Pattern& pattern);
bool pattern_contains_or(const Pattern& pattern);
bool pattern_contains_array_pattern(const Pattern& pattern);
bool runtime_sequence_array_pattern_is_irrefutable(const Pattern& pattern);
std::vector<Pattern> expand_or_pattern_alternatives(const Pattern& pattern);
void require_irrefutable_non_iterator_for_pattern(const Pattern& pattern,
                                                  const IrType& value_type,
                                                  const ForPatternValidationHooks& hooks);
std::size_t tuple_pattern_field_index(const Pattern& pattern,
                                      std::size_t field_count,
                                      std::size_t pattern_index);
const Pattern* positional_product_field_pattern(const Pattern& pattern,
                                                std::size_t field_count,
                                                std::size_t field_index);

} // namespace ari
