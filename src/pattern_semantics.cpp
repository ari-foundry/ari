#include "pattern_semantics.hpp"

#include "common.hpp"

#include <memory>
#include <string>
#include <utility>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

Pattern clone_pattern_without_children(const Pattern& pattern) {
    Pattern copy;
    copy.kind = pattern.kind;
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
    copy.alias_name = pattern.alias_name;
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

} // namespace ari
