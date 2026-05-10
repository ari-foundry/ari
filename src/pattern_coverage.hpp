#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ari {

struct ScalarMatchCoverage {
    bool has_wildcard = false;
    std::set<std::string> covered_patterns;
    std::vector<std::pair<std::uint64_t, std::uint64_t>> integer_intervals;
};

void note_integer_coverage(ScalarMatchCoverage& coverage,
                           const IrType& match_type,
                           std::uint64_t value,
                           bool negative);
void note_integer_range_coverage(ScalarMatchCoverage& coverage,
                                 const IrType& match_type,
                                 const Pattern& pattern);
bool integer_range_coverage_interval(const Pattern& pattern,
                                     const IrType& match_type,
                                     std::uint64_t& start,
                                     std::uint64_t& end);
bool integer_interval_is_fully_covered(const ScalarMatchCoverage& coverage,
                                       std::uint64_t start,
                                       std::uint64_t end);
bool integer_coverage_is_exhaustive(
    const IrType& match_type,
    const std::vector<std::pair<std::uint64_t, std::uint64_t>>& intervals
);
std::uint64_t integer_pattern_order_value(std::uint64_t value,
                                          bool negative,
                                          const IrType& match_type);
std::uint64_t integer_pattern_max_order_value(const IrType& match_type);

} // namespace ari
