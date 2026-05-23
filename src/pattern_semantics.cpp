#include "pattern_semantics.hpp"

#include "common.hpp"
#include "layout.hpp"
#include "slice_semantics.hpp"
#include "type_semantics.hpp"
#include "vector_semantics.hpp"

#include <memory>
#include <set>
#include <string>
#include <utility>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(loc, message);
}

[[noreturn]] void fail_refutable_for_pattern(SourceLocation loc) {
    fail(loc,
         "refutable for-loop patterns require for-let filters on Iterator[T]; non-iterator loop heads currently support irrefutable binding and alias patterns, with aggregate destructuring for list and vector items");
}

Pattern clone_pattern_without_children(const Pattern& pattern) {
    Pattern copy;
    copy.kind = pattern.kind;
    copy.binding_mode = pattern.binding_mode;
    copy.case_name = pattern.case_name;
    copy.has_payload_pattern = pattern.has_payload_pattern;
    copy.has_payload_binding = pattern.has_payload_binding;
    copy.payload_name = pattern.payload_name;
    copy.int_negative = pattern.int_negative;
    copy.literal_suffix = pattern.literal_suffix;
    copy.range_end_value = pattern.range_end_value;
    copy.range_end_negative = pattern.range_end_negative;
    copy.range_end_suffix = pattern.range_end_suffix;
    copy.range_inclusive = pattern.range_inclusive;
    copy.field_names = pattern.field_names;
    copy.has_rest = pattern.has_rest;
    copy.rest_index = pattern.rest_index;
    copy.rest_alias_name = pattern.rest_alias_name;
    copy.rest_alias_loc = pattern.rest_alias_loc;
    copy.alias_name = pattern.alias_name;
    copy.is_macro_invocation = pattern.is_macro_invocation;
    copy.macro_tokens = pattern.macro_tokens;
    copy.loc = pattern.loc;
    switch (pattern.kind) {
        case PatternKind::IntegerLiteral:
        case PatternKind::Range:
            copy.int_value = pattern.int_value;
            break;
        case PatternKind::BoolLiteral:
            copy.bool_value = pattern.bool_value;
            break;
        default:
            break;
    }
    return copy;
}

constexpr std::size_t kMaxOrPatternExpansions = 64;

void append_pattern_expansion(std::vector<Pattern>& out, Pattern pattern, SourceLocation loc) {
    if (out.size() >= kMaxOrPatternExpansions) {
        fail(loc, "or-pattern expands to too many alternatives");
    }
    out.push_back(std::move(pattern));
}

const std::vector<IrType>& aggregate_field_types(const IrType& type) {
    return ari_aggregate_field_types(type);
}

void require_for_pattern_hooks(SourceLocation loc, const ForPatternValidationHooks& hooks) {
    if (!hooks.require_struct_pattern_type || !hooks.struct_field_index) {
        fail(loc, "internal error: incomplete for-loop pattern validation hooks");
    }
}

void require_product_pattern_hooks(SourceLocation loc, const ProductPatternIrrefutabilityHooks& hooks) {
    if (!hooks.expand_pattern ||
        !hooks.require_struct_pattern_type ||
        !hooks.struct_field_index ||
        !hooks.is_runtime_sequence_subject) {
        fail(loc, "internal error: incomplete product-pattern validation hooks");
    }
}

void require_tuple_pattern_arity(const Pattern& pattern,
                                 const IrType& source_type,
                                 const std::vector<IrType>& fields) {
    const char* pattern_name = pattern.kind == PatternKind::Array ? "array" : "tuple";
    if (!pattern.rest_alias_name.empty()) {
        fail(pattern.rest_alias_loc,
             std::string(pattern_name) +
                 " rest bindings currently require a Vec[T] or Slice[T] value");
    }
    if (pattern.has_rest) {
        if (pattern.elements.size() > fields.size()) {
            fail(pattern.loc,
                 std::string(pattern_name) + " match pattern has " +
                     std::to_string(pattern.elements.size()) +
                     " non-rest elements but value has " + std::to_string(fields.size()));
        }
        return;
    }
    if (pattern.elements.size() != fields.size()) {
        fail(pattern.loc,
             std::string(pattern_name) + " match pattern has " +
                 std::to_string(pattern.elements.size()) +
                 " elements but value has " + std::to_string(fields.size()));
    }
    (void)source_type;
}

void require_irrefutable_tuple_struct_for_pattern(const Pattern& pattern,
                                                  const IrType& value_type,
                                                  const ForPatternValidationHooks& hooks) {
    ForPatternStructInfo info = hooks.require_struct_pattern_type(pattern.loc, pattern.case_name, value_type);
    if (!info.tuple_struct) {
        fail(pattern.loc, "tuple-struct for-loop pattern requires a tuple struct");
    }
    if (!pattern.has_payload_pattern || !pattern.payload_pattern) {
        fail(pattern.loc, "tuple-struct for-loop pattern '" + pattern.case_name + "' requires positional fields");
    }

    const std::vector<IrType>& fields = aggregate_field_types(value_type);
    const Pattern& payload = *pattern.payload_pattern;
    if (payload.kind == PatternKind::Tuple) {
        require_tuple_pattern_arity(payload, value_type, fields);
        for (std::size_t i = 0; i < payload.elements.size(); ++i) {
            require_irrefutable_non_iterator_for_pattern(
                payload.elements[i],
                fields[tuple_pattern_field_index(payload, fields.size(), i)],
                hooks
            );
        }
        return;
    }
    if (fields.size() != 1) {
        fail(payload.loc,
             "tuple-struct for-loop pattern for '" + info.name + "' has 1 field but value has " +
                 std::to_string(fields.size()));
    }
    require_irrefutable_non_iterator_for_pattern(payload, fields[0], hooks);
}

} // namespace

Pattern clone_pattern(const Pattern& pattern) {
    Pattern copy = clone_pattern_without_children(pattern);
    if (pattern.payload_pattern) {
        copy.payload_pattern = std::make_unique<Pattern>(clone_pattern(*pattern.payload_pattern));
    }
    if (pattern.alias_pattern) {
        copy.alias_pattern = std::make_unique<Pattern>(clone_pattern(*pattern.alias_pattern));
    }
    copy.alternatives.reserve(pattern.alternatives.size());
    for (const auto& alternative : pattern.alternatives) {
        copy.alternatives.push_back(clone_pattern(alternative));
    }
    copy.elements.reserve(pattern.elements.size());
    for (const auto& element : pattern.elements) {
        copy.elements.push_back(clone_pattern(element));
    }
    return copy;
}

bool pattern_has_binding(const Pattern& pattern) {
    if (pattern.kind == PatternKind::Binding) return true;
    if (pattern.kind == PatternKind::Alias) return true;
    if (!pattern.rest_alias_name.empty()) return true;
    if (pattern.alias_pattern && pattern_has_binding(*pattern.alias_pattern)) return true;
    if (pattern.payload_pattern && pattern_has_binding(*pattern.payload_pattern)) return true;
    for (const auto& alternative : pattern.alternatives) {
        if (pattern_has_binding(alternative)) return true;
    }
    for (const auto& element : pattern.elements) {
        if (pattern_has_binding(element)) return true;
    }
    return false;
}

bool pattern_has_reference_binding_mode(const Pattern& pattern) {
    if (pattern.binding_mode != BindingMode::Value) return true;
    if (pattern.alias_pattern && pattern_has_reference_binding_mode(*pattern.alias_pattern)) return true;
    if (pattern.payload_pattern && pattern_has_reference_binding_mode(*pattern.payload_pattern)) return true;
    for (const auto& alternative : pattern.alternatives) {
        if (pattern_has_reference_binding_mode(alternative)) return true;
    }
    for (const auto& element : pattern.elements) {
        if (pattern_has_reference_binding_mode(element)) return true;
    }
    return false;
}

bool pattern_has_mutable_reference_binding_mode(const Pattern& pattern) {
    if (pattern.binding_mode == BindingMode::RefMut) return true;
    if (pattern.alias_pattern && pattern_has_mutable_reference_binding_mode(*pattern.alias_pattern)) return true;
    if (pattern.payload_pattern && pattern_has_mutable_reference_binding_mode(*pattern.payload_pattern)) return true;
    for (const auto& alternative : pattern.alternatives) {
        if (pattern_has_mutable_reference_binding_mode(alternative)) return true;
    }
    for (const auto& element : pattern.elements) {
        if (pattern_has_mutable_reference_binding_mode(element)) return true;
    }
    return false;
}

bool pattern_contains_or(const Pattern& pattern) {
    if (pattern.kind == PatternKind::Or) return true;
    if (pattern.alias_pattern && pattern_contains_or(*pattern.alias_pattern)) return true;
    if (pattern.payload_pattern && pattern_contains_or(*pattern.payload_pattern)) return true;
    for (const auto& alternative : pattern.alternatives) {
        if (pattern_contains_or(alternative)) return true;
    }
    for (const auto& element : pattern.elements) {
        if (pattern_contains_or(element)) return true;
    }
    return false;
}

bool pattern_contains_array_pattern(const Pattern& pattern) {
    if (pattern.kind == PatternKind::Array) return true;
    if (pattern.alias_pattern && pattern_contains_array_pattern(*pattern.alias_pattern)) return true;
    if (pattern.payload_pattern && pattern_contains_array_pattern(*pattern.payload_pattern)) return true;
    for (const auto& alternative : pattern.alternatives) {
        if (pattern_contains_array_pattern(alternative)) return true;
    }
    for (const auto& element : pattern.elements) {
        if (pattern_contains_array_pattern(element)) return true;
    }
    return false;
}

bool runtime_sequence_array_pattern_is_irrefutable(const Pattern& pattern) {
    switch (pattern.kind) {
        case PatternKind::Wildcard:
        case PatternKind::Binding:
            return true;
        case PatternKind::Alias:
            return pattern.alias_pattern &&
                   runtime_sequence_array_pattern_is_irrefutable(*pattern.alias_pattern);
        case PatternKind::Or:
            for (const auto& alternative : pattern.alternatives) {
                if (runtime_sequence_array_pattern_is_irrefutable(alternative)) return true;
            }
            return false;
        case PatternKind::Array:
            return pattern.has_rest && pattern.elements.empty();
        case PatternKind::IntegerLiteral:
        case PatternKind::BoolLiteral:
        case PatternKind::Range:
        case PatternKind::EnumCase:
        case PatternKind::Tuple:
        case PatternKind::Struct:
            return false;
    }
    return false;
}

bool product_pattern_condition_is_irrefutable(const Pattern& pattern,
                                              const IrType& source_type,
                                              const ProductPatternIrrefutabilityHooks& hooks) {
    require_product_pattern_hooks(pattern.loc, hooks);
    const Pattern& effective_pattern = hooks.expand_pattern(pattern);
    if (&effective_pattern != &pattern) {
        return product_pattern_condition_is_irrefutable(effective_pattern, source_type, hooks);
    }
    switch (pattern.kind) {
        case PatternKind::Wildcard:
        case PatternKind::Binding:
            return true;
        case PatternKind::Alias:
            return pattern.alias_pattern &&
                   product_pattern_condition_is_irrefutable(*pattern.alias_pattern, source_type, hooks);
        case PatternKind::Or:
            for (const auto& alternative : pattern.alternatives) {
                if (product_pattern_condition_is_irrefutable(alternative, source_type, hooks)) return true;
            }
            return false;
        case PatternKind::Tuple:
        case PatternKind::Array: {
            if (pattern.kind == PatternKind::Array &&
                hooks.is_runtime_sequence_subject(source_type)) {
                return runtime_sequence_array_pattern_is_irrefutable(pattern);
            }
            IrPrimitiveKind expected = pattern.kind == PatternKind::Array
                ? IrPrimitiveKind::Array
                : IrPrimitiveKind::Tuple;
            if (source_type.primitive != expected) return false;
            const std::vector<IrType>& fields = aggregate_field_types(source_type);
            require_tuple_pattern_arity(pattern, source_type, fields);
            for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
                std::size_t field_index = tuple_pattern_field_index(pattern, fields.size(), i);
                if (!product_pattern_condition_is_irrefutable(pattern.elements[i], fields[field_index], hooks)) {
                    return false;
                }
            }
            return true;
        }
        case PatternKind::Struct: {
            if (source_type.primitive != IrPrimitiveKind::Struct) return false;
            hooks.require_struct_pattern_type(pattern.loc, pattern.case_name, source_type);
            if (pattern.field_names.size() != pattern.elements.size()) {
                throw CompileError("internal error: struct match pattern field/value arity mismatch");
            }
            for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
                std::size_t field_index = hooks.struct_field_index(
                    pattern.elements[i].loc,
                    source_type,
                    pattern.field_names[i]);
                if (!product_pattern_condition_is_irrefutable(
                        pattern.elements[i],
                        source_type.field_types[field_index],
                        hooks)) {
                    return false;
                }
            }
            return true;
        }
        case PatternKind::EnumCase: {
            if (source_type.primitive != IrPrimitiveKind::Struct) return false;
            ForPatternStructInfo info = hooks.require_struct_pattern_type(pattern.loc, pattern.case_name, source_type);
            if (!info.tuple_struct || !pattern.has_payload_pattern || !pattern.payload_pattern) return false;
            const std::vector<IrType>& fields = aggregate_field_types(source_type);
            const Pattern& payload = *pattern.payload_pattern;
            if (payload.kind == PatternKind::Tuple) {
                require_tuple_pattern_arity(payload, source_type, fields);
                for (std::size_t i = 0; i < payload.elements.size(); ++i) {
                    std::size_t field_index = tuple_pattern_field_index(payload, fields.size(), i);
                    if (!product_pattern_condition_is_irrefutable(payload.elements[i], fields[field_index], hooks)) {
                        return false;
                    }
                }
                return true;
            }
            return fields.size() == 1 &&
                   product_pattern_condition_is_irrefutable(payload, fields[0], hooks);
        }
        case PatternKind::IntegerLiteral:
        case PatternKind::BoolLiteral:
        case PatternKind::Range:
            return false;
    }
    return false;
}

std::vector<Pattern> expand_or_pattern_alternatives(const Pattern& pattern) {
    if (pattern.kind == PatternKind::Or) {
        std::vector<Pattern> out;
        for (const auto& alternative : pattern.alternatives) {
            std::vector<Pattern> expanded = expand_or_pattern_alternatives(alternative);
            for (auto& item : expanded) append_pattern_expansion(out, std::move(item), pattern.loc);
        }
        return out;
    }

    std::vector<Pattern> expanded;
    expanded.push_back(clone_pattern_without_children(pattern));

    if (pattern.alias_pattern) {
        std::vector<Pattern> child_expansions = expand_or_pattern_alternatives(*pattern.alias_pattern);
        std::vector<Pattern> next;
        for (const auto& base : expanded) {
            for (const auto& child : child_expansions) {
                Pattern copy = clone_pattern(base);
                copy.alias_pattern = std::make_unique<Pattern>(clone_pattern(child));
                append_pattern_expansion(next, std::move(copy), pattern.loc);
            }
        }
        expanded = std::move(next);
    }

    if (pattern.payload_pattern) {
        std::vector<Pattern> child_expansions = expand_or_pattern_alternatives(*pattern.payload_pattern);
        std::vector<Pattern> next;
        for (const auto& base : expanded) {
            for (const auto& child : child_expansions) {
                Pattern copy = clone_pattern(base);
                copy.payload_pattern = std::make_unique<Pattern>(clone_pattern(child));
                append_pattern_expansion(next, std::move(copy), pattern.loc);
            }
        }
        expanded = std::move(next);
    }

    for (const auto& element : pattern.elements) {
        std::vector<Pattern> child_expansions = expand_or_pattern_alternatives(element);
        std::vector<Pattern> next;
        for (const auto& base : expanded) {
            for (const auto& child : child_expansions) {
                Pattern copy = clone_pattern(base);
                copy.elements.push_back(clone_pattern(child));
                append_pattern_expansion(next, std::move(copy), pattern.loc);
            }
        }
        expanded = std::move(next);
    }

    return expanded;
}

PatternAlternativeSet pattern_alternatives(const Pattern& pattern) {
    PatternAlternativeSet result;
    result.contains_or = pattern_contains_or(pattern);
    result.alternatives = expand_or_pattern_alternatives(pattern);
    return result;
}

void require_irrefutable_non_iterator_for_pattern(const Pattern& pattern,
                                                  const IrType& value_type,
                                                  const ForPatternValidationHooks& hooks) {
    require_for_pattern_hooks(pattern.loc, hooks);

    switch (pattern.kind) {
        case PatternKind::Wildcard:
        case PatternKind::Binding:
            return;
        case PatternKind::Tuple: {
            if (value_type.primitive != IrPrimitiveKind::Tuple) {
                fail(pattern.loc, "tuple for-loop pattern requires a tuple element, got " + type_name(value_type));
            }
            const std::vector<IrType>& fields = aggregate_field_types(value_type);
            require_tuple_pattern_arity(pattern, value_type, fields);
            for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
                require_irrefutable_non_iterator_for_pattern(
                    pattern.elements[i],
                    fields[tuple_pattern_field_index(pattern, fields.size(), i)],
                    hooks
                );
            }
            return;
        }
        case PatternKind::Array: {
            if (value_type.primitive != IrPrimitiveKind::Array) {
                fail(pattern.loc, "array for-loop pattern requires an array element, got " + type_name(value_type));
            }
            const std::vector<IrType>& fields = aggregate_field_types(value_type);
            require_tuple_pattern_arity(pattern, value_type, fields);
            for (std::size_t i = 0; i < pattern.elements.size(); ++i) {
                require_irrefutable_non_iterator_for_pattern(
                    pattern.elements[i],
                    fields[tuple_pattern_field_index(pattern, fields.size(), i)],
                    hooks
                );
            }
            return;
        }
        case PatternKind::Struct: {
            hooks.require_struct_pattern_type(pattern.loc, pattern.case_name, value_type);
            if (pattern.field_names.size() != pattern.elements.size()) {
                throw CompileError("internal error: struct for-loop pattern field/value arity mismatch");
            }
            if (!pattern.has_rest && pattern.field_names.size() != value_type.field_names.size()) {
                fail(pattern.loc, "struct for-loop pattern must mention all fields or use '..'");
            }
            std::set<std::string> seen_fields;
            for (std::size_t i = 0; i < pattern.field_names.size(); ++i) {
                const std::string& field_name = pattern.field_names[i];
                if (!seen_fields.insert(field_name).second) {
                    fail(pattern.elements[i].loc, "duplicate field '" + field_name + "' in struct for-loop pattern");
                }
                std::size_t field_index = hooks.struct_field_index(pattern.elements[i].loc, value_type, field_name);
                require_irrefutable_non_iterator_for_pattern(pattern.elements[i], value_type.field_types[field_index], hooks);
            }
            return;
        }
        case PatternKind::EnumCase:
            if (value_type.primitive == IrPrimitiveKind::Struct) {
                require_irrefutable_tuple_struct_for_pattern(pattern, value_type, hooks);
                return;
            }
            fail(pattern.loc, "for enum-case patterns are planned for Iterator[T] values but are not supported yet");
            return;
        case PatternKind::Alias:
            if (!pattern.alias_pattern) fail(pattern.loc, "missing aliased for-loop pattern");
            require_irrefutable_non_iterator_for_pattern(*pattern.alias_pattern, value_type, hooks);
            return;
        case PatternKind::IntegerLiteral:
        case PatternKind::BoolLiteral:
        case PatternKind::Range:
        case PatternKind::Or:
            fail_refutable_for_pattern(pattern.loc);
            return;
    }
}

std::size_t tuple_pattern_field_index(const Pattern& pattern,
                                      std::size_t field_count,
                                      std::size_t pattern_index) {
    if (!pattern.has_rest || pattern_index < pattern.rest_index) return pattern_index;
    std::size_t suffix_count = pattern.elements.size() - pattern.rest_index;
    return field_count - suffix_count + (pattern_index - pattern.rest_index);
}

const Pattern* positional_product_field_pattern(const Pattern& pattern,
                                                std::size_t field_count,
                                                std::size_t field_index) {
    if (!pattern.has_rest) return &pattern.elements[field_index];
    if (field_index < pattern.rest_index) return &pattern.elements[field_index];

    std::size_t suffix_count = pattern.elements.size() - pattern.rest_index;
    std::size_t suffix_start = field_count - suffix_count;
    if (field_index >= suffix_start) {
        return &pattern.elements[pattern.rest_index + field_index - suffix_start];
    }
    return nullptr;
}

RuntimeSequenceReferencePatternPlan plan_runtime_sequence_reference_pattern(
    const Pattern& pattern,
    const IrType& shape_type,
    bool direct_binding,
    const RuntimeSequenceKnownLengthLookup& known_direct_vec_length) {
    if (!is_vector_storage_type(shape_type) && !is_prelude_slice_type(shape_type)) {
        fail(pattern.loc,
             "runtime sequence reference patterns currently require direct local Vec[T] storage or Slice[T] view bindings");
    }
    if (!direct_binding && !pattern.rest_alias_name.empty()) {
        fail(pattern.rest_alias_loc,
             "runtime sequence reference rest aliases currently require a direct local Vec[T] or Slice[T] binding");
    }

    RuntimeSequenceReferencePatternPlan plan;
    if (!is_owner_type(shape_type)) return plan;

    if (!is_vector_storage_type(shape_type)) {
        fail(pattern.loc,
             "ownership-carrying runtime sequence reference patterns currently require direct local Vec[T] storage");
    }
    if (!pattern.has_rest) return plan;

    const std::size_t suffix_count = pattern.elements.size() - pattern.rest_index;
    if (suffix_count == 0) return plan;
    if (!known_direct_vec_length) {
        fail(pattern.loc, "internal error: missing runtime sequence known-length lookup");
    }
    plan.known_owner_vec_length = known_direct_vec_length();
    if (!plan.known_owner_vec_length) {
        plan.dynamic_owner_suffix_uses_synthetic_paths = true;
        return plan;
    }

    const std::uint64_t required =
        static_cast<std::uint64_t>(pattern.rest_index + suffix_count);
    if (*plan.known_owner_vec_length < required) {
        fail(pattern.loc,
             "ownership-carrying Vec[T] reference pattern requires a known length of at least " +
                 std::to_string(required));
    }
    return plan;
}

RuntimeSequenceValuePatternPlan plan_runtime_sequence_value_pattern(
    const Pattern& pattern,
    const IrType& shape_type,
    bool direct_binding,
    const RuntimeSequenceKnownLengthLookup& known_direct_vec_length) {
    if (!is_vector_storage_type(shape_type) && !is_prelude_slice_type(shape_type)) {
        fail(pattern.loc,
             "runtime sequence value patterns currently require direct local Vec[T] storage or Slice[T] view bindings");
    }
    if (!direct_binding && !pattern.rest_alias_name.empty()) {
        fail(pattern.rest_alias_loc,
             "runtime sequence value rest aliases currently require a direct local Vec[T] or Slice[T] binding");
    }

    RuntimeSequenceValuePatternPlan plan;
    if (!is_owner_type(shape_type)) return plan;

    if (!is_vector_storage_type(shape_type) || !direct_binding) {
        fail(pattern.loc,
             "ownership-carrying runtime sequence value patterns currently require direct local Vec[T] storage");
    }
    if (!pattern.has_rest) return plan;

    const std::size_t suffix_count = pattern.elements.size() - pattern.rest_index;
    if (!known_direct_vec_length) {
        fail(pattern.loc, "internal error: missing runtime sequence known-length lookup");
    }
    plan.known_owner_vec_length = known_direct_vec_length();
    if (!plan.known_owner_vec_length) {
        if (!pattern.rest_alias_name.empty()) {
            fail(pattern.rest_alias_loc,
                 "unknown-length ownership-carrying Vec[T] value rest aliases are not supported; "
                 "bind suffix elements directly or borrow the rest with a reference pattern");
        }
        plan.dynamic_owner_suffix_uses_runtime_paths = true;
        return plan;
    }

    const std::uint64_t required =
        static_cast<std::uint64_t>(pattern.rest_index + suffix_count);
    if (*plan.known_owner_vec_length < required) {
        fail(pattern.loc,
             "ownership-carrying Vec[T] value pattern requires a known length of at least " +
                 std::to_string(required));
    }
    return plan;
}

} // namespace ari
