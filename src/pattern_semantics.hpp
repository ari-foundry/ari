#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
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

struct ProductPatternIrrefutabilityHooks {
    std::function<const Pattern&(const Pattern&)> expand_pattern;
    std::function<ForPatternStructInfo(SourceLocation, const std::string&, const IrType&)>
        require_struct_pattern_type;
    std::function<std::size_t(SourceLocation, const IrType&, const std::string&)>
        struct_field_index;
    std::function<bool(const IrType&)> is_runtime_sequence_subject;
};

struct PatternAlternativeSet {
    std::vector<Pattern> alternatives;
    bool contains_or = false;
};

struct RuntimeSequenceReferencePatternPlan {
    std::optional<std::uint64_t> known_owner_vec_length;
    bool dynamic_owner_suffix_uses_whole_borrow = false;
};

struct RuntimeSequenceValuePatternPlan {
    std::optional<std::uint64_t> known_owner_vec_length;
};

using RuntimeSequenceKnownLengthLookup = std::function<std::optional<std::uint64_t>()>;

Pattern clone_pattern(const Pattern& pattern);
bool pattern_has_binding(const Pattern& pattern);
bool pattern_has_reference_binding_mode(const Pattern& pattern);
bool pattern_has_mutable_reference_binding_mode(const Pattern& pattern);
bool pattern_contains_or(const Pattern& pattern);
bool pattern_contains_array_pattern(const Pattern& pattern);
bool runtime_sequence_array_pattern_is_irrefutable(const Pattern& pattern);
bool product_pattern_condition_is_irrefutable(const Pattern& pattern,
                                              const IrType& source_type,
                                              const ProductPatternIrrefutabilityHooks& hooks);
std::vector<Pattern> expand_or_pattern_alternatives(const Pattern& pattern);
PatternAlternativeSet pattern_alternatives(const Pattern& pattern);
void require_irrefutable_non_iterator_for_pattern(const Pattern& pattern,
                                                  const IrType& value_type,
                                                  const ForPatternValidationHooks& hooks);
std::size_t tuple_pattern_field_index(const Pattern& pattern,
                                      std::size_t field_count,
                                      std::size_t pattern_index);
const Pattern* positional_product_field_pattern(const Pattern& pattern,
                                                std::size_t field_count,
                                                std::size_t field_index);
RuntimeSequenceReferencePatternPlan plan_runtime_sequence_reference_pattern(
    const Pattern& pattern,
    const IrType& shape_type,
    bool direct_binding,
    const RuntimeSequenceKnownLengthLookup& known_direct_vec_length);
RuntimeSequenceValuePatternPlan plan_runtime_sequence_value_pattern(
    const Pattern& pattern,
    const IrType& shape_type,
    bool direct_binding,
    const RuntimeSequenceKnownLengthLookup& known_direct_vec_length);

} // namespace ari
