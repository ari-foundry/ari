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
