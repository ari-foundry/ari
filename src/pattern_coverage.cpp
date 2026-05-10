#include "pattern_coverage.hpp"

#include "type_semantics.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace ari {
namespace {

void add_integer_coverage_interval(ScalarMatchCoverage& coverage,
                                   std::uint64_t start,
                                   std::uint64_t end) {
    if (start > end) return;
    coverage.integer_intervals.push_back({start, end});
    std::sort(coverage.integer_intervals.begin(), coverage.integer_intervals.end());

    std::vector<std::pair<std::uint64_t, std::uint64_t>> merged;
    for (const auto& interval : coverage.integer_intervals) {
        if (merged.empty()) {
            merged.push_back(interval);
            continue;
        }
        auto& last = merged.back();
        bool adjacent = last.second != std::numeric_limits<std::uint64_t>::max() &&
                        interval.first == last.second + 1;
        if (interval.first <= last.second || adjacent) {
            if (interval.second > last.second) last.second = interval.second;
        } else {
            merged.push_back(interval);
        }
    }
    coverage.integer_intervals = std::move(merged);
}

std::string join_product_pattern_parts(const std::vector<std::string>& parts,
                                       const std::string& separator) {
    std::string out;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) out += separator;
        out += parts[i];
    }
    return out;
}

std::string ordered_integer_product_literal(std::uint64_t ordered_value,
                                            const IrType& type) {
    std::string suffix = type_name(type);
    if (is_unsigned_integer_primitive(type.primitive)) {
        return std::to_string(ordered_value) + suffix;
    }

    std::uint64_t bias = signed_negative_limit(type.primitive);
    if (ordered_value < bias) {
        return "-" + std::to_string(bias - ordered_value) + suffix;
    }
    return std::to_string(ordered_value - bias) + suffix;
}

bool product_interval_covers_domain(const ProductInterval& interval,
                                    const IrType& type) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        return interval.start == 0 && interval.end == 1;
    }
    if (is_value_integer_type(type)) {
        return interval.start == 0 && interval.end == integer_pattern_max_order_value(type);
    }
    return false;
}

const std::vector<IrType>& product_coverage_field_types(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::Struct ||
        type.primitive == IrPrimitiveKind::Array) {
        return type.field_types;
    }
    return type.args;
}

std::string format_product_missing_case(const IrType& type,
                                        const ProductRect& rect,
                                        std::size_t& dimension,
                                        const std::set<std::string>& tuple_struct_names) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        if (dimension >= rect.size()) return "_";
        const ProductInterval interval = rect[dimension++];
        if (product_interval_covers_domain(interval, type)) return "_";
        if (interval.start == interval.end) return interval.start == 0 ? "false" : "true";
        return "_";
    }
    if (is_value_integer_type(type)) {
        if (dimension >= rect.size()) return "_";
        const ProductInterval interval = rect[dimension++];
        if (product_interval_covers_domain(interval, type)) return "_";
        if (interval.start == interval.end) {
            return ordered_integer_product_literal(interval.start, type);
        }
        return ordered_integer_product_literal(interval.start, type) + "..=" +
               ordered_integer_product_literal(interval.end, type);
    }
    if (type.primitive == IrPrimitiveKind::Tuple ||
        type.primitive == IrPrimitiveKind::Array ||
        type.primitive == IrPrimitiveKind::Struct) {
        std::vector<std::string> parts;
        const std::vector<IrType>& fields = product_coverage_field_types(type);
        parts.reserve(fields.size());
        for (const auto& field_type : fields) {
            parts.push_back(format_product_missing_case(field_type, rect, dimension, tuple_struct_names));
        }

        if (type.primitive == IrPrimitiveKind::Tuple) {
            return "(" + join_product_pattern_parts(parts, ", ") + ")";
        }
        if (type.primitive == IrPrimitiveKind::Array) {
            return "[" + join_product_pattern_parts(parts, ", ") + "]";
        }

        if (tuple_struct_names.count(type.name)) {
            return type.name + "(" + join_product_pattern_parts(parts, ", ") + ")";
        }

        std::vector<std::string> named_parts;
        named_parts.reserve(parts.size());
        for (std::size_t i = 0; i < parts.size(); ++i) {
            std::string field_name = i < type.field_names.size()
                ? type.field_names[i]
                : ("_" + std::to_string(i));
            named_parts.push_back(field_name + ": " + parts[i]);
        }
        return type.name + " { " + join_product_pattern_parts(named_parts, ", ") + " }";
    }
    return "_";
}

} // namespace

std::string bool_product_value(bool value) {
    return value ? "b1" : "b0";
}

std::string integer_product_value(std::uint64_t ordered_value) {
    return "i" + std::to_string(ordered_value);
}

bool finite_scalar_product_domain(const IrType& type, std::vector<std::string>& out) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        out = {bool_product_value(false), bool_product_value(true)};
        return true;
    }
    if (!is_value_integer_type(type)) return false;

    std::uint64_t max = integer_pattern_max_order_value(type);
    if (max >= kMaxFiniteProductCoverageValues) return false;
    out.clear();
    out.reserve(static_cast<std::size_t>(max + 1));
    for (std::uint64_t value = 0; value <= max; ++value) {
        out.push_back(integer_product_value(value));
    }
    return true;
}

std::string product_coverage_join(const std::string& prefix, const std::string& suffix) {
    if (prefix.empty()) return suffix;
    return prefix + "\x1f" + suffix;
}

bool combine_finite_product_domains(const std::vector<std::vector<std::string>>& domains,
                                    std::vector<std::string>& out) {
    std::vector<std::string> result{""};
    for (const auto& domain : domains) {
        if (domain.empty()) return false;
        if (result.size() > kMaxFiniteProductCoverageValues / domain.size()) return false;
        std::vector<std::string> next;
        next.reserve(result.size() * domain.size());
        for (const auto& prefix : result) {
            for (const auto& value : domain) {
                next.push_back(product_coverage_join(prefix, value));
            }
        }
        result = std::move(next);
    }
    out = std::move(result);
    return true;
}

std::string product_missing_case_hint(const IrType& match_type,
                                      const ProductMatchCoverage& coverage,
                                      const std::set<std::string>& tuple_struct_names) {
    if (!coverage.has_symbolic_universe) return "";
    ProductRect missing;
    if (!product_rect_first_gap(coverage.symbolic_universe, coverage.covered_symbolic_products, missing)) {
        return "";
    }
    std::size_t dimension = 0;
    return format_product_missing_case(match_type, missing, dimension, tuple_struct_names);
}

void note_integer_coverage(ScalarMatchCoverage& coverage,
                           const IrType& match_type,
                           std::uint64_t value,
                           bool negative) {
    std::uint64_t point = integer_pattern_order_value(value, negative, match_type);
    add_integer_coverage_interval(coverage, point, point);
}

void note_integer_range_coverage(ScalarMatchCoverage& coverage,
                                 const IrType& match_type,
                                 const Pattern& pattern) {
    std::uint64_t start = 0;
    std::uint64_t end = 0;
    if (!integer_range_coverage_interval(pattern, match_type, start, end)) return;
    add_integer_coverage_interval(coverage, start, end);
}

bool integer_range_coverage_interval(const Pattern& pattern,
                                     const IrType& match_type,
                                     std::uint64_t& start,
                                     std::uint64_t& end) {
    start = integer_pattern_order_value(pattern.int_value, pattern.int_negative, match_type);
    end = integer_pattern_order_value(pattern.range_end_value, pattern.range_end_negative, match_type);
    if (!pattern.range_inclusive) {
        if (start >= end) return false;
        --end;
    }
    return start <= end;
}

bool integer_interval_is_fully_covered(const ScalarMatchCoverage& coverage,
                                       std::uint64_t start,
                                       std::uint64_t end) {
    if (start > end) return false;
    for (const auto& interval : coverage.integer_intervals) {
        if (interval.first <= start && end <= interval.second) return true;
        if (start < interval.first) return false;
    }
    return false;
}

bool integer_coverage_is_exhaustive(
    const IrType& match_type,
    const std::vector<std::pair<std::uint64_t, std::uint64_t>>& intervals
) {
    if (!is_value_integer_type(match_type) || intervals.empty()) return false;
    return intervals.front().first == 0 &&
           intervals.front().second == integer_pattern_max_order_value(match_type);
}

std::uint64_t integer_pattern_order_value(std::uint64_t value,
                                          bool negative,
                                          const IrType& match_type) {
    if (is_unsigned_integer_primitive(match_type.primitive)) return value;
    std::uint64_t bias = signed_negative_limit(match_type.primitive);
    return negative ? bias - value : bias + value;
}

std::uint64_t integer_pattern_max_order_value(const IrType& match_type) {
    if (is_unsigned_integer_primitive(match_type.primitive)) {
        return unsigned_max(match_type.primitive);
    }
    return signed_negative_limit(match_type.primitive) + signed_positive_max(match_type.primitive);
}

} // namespace ari
