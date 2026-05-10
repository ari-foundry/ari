#pragma once

#include "ast.hpp"
#include "ir.hpp"
#include "product_coverage.hpp"

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace ari {

inline constexpr std::size_t kMaxFiniteProductCoverageValues = 4096;

struct ScalarMatchCoverage {
    bool has_wildcard = false;
    std::set<std::string> covered_patterns;
    std::vector<std::pair<std::uint64_t, std::uint64_t>> integer_intervals;
};

struct ProductMatchCoverage {
    bool has_irrefutable_arm = false;
    bool checked_finite_universe = false;
    bool has_finite_universe = false;
    std::size_t universe_size = 0;
    std::set<std::string> covered_products;
    bool checked_symbolic_universe = false;
    bool has_symbolic_universe = false;
    ProductRect symbolic_universe;
    std::vector<ProductRect> covered_symbolic_products;
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
std::string scalar_match_exhaustiveness_error(const IrType& match_type,
                                              const ScalarMatchCoverage& coverage);
std::uint64_t integer_pattern_order_value(std::uint64_t value,
                                          bool negative,
                                          const IrType& match_type);
std::uint64_t integer_pattern_max_order_value(const IrType& match_type);
std::string bool_product_value(bool value);
std::string integer_product_value(std::uint64_t ordered_value);
bool finite_scalar_product_domain(const IrType& type, std::vector<std::string>& out);
bool combine_finite_product_domains(const std::vector<std::vector<std::string>>& domains,
                                    std::vector<std::string>& out);
bool note_finite_product_match_coverage(ProductMatchCoverage& coverage,
                                        const std::vector<std::string>& values);
bool note_symbolic_product_match_coverage(ProductMatchCoverage& coverage,
                                          const std::vector<ProductRect>& rects,
                                          bool finite_handled);
bool product_match_coverage_is_exhaustive(const ProductMatchCoverage& coverage);
std::string product_missing_case_hint(const IrType& match_type,
                                      const ProductMatchCoverage& coverage,
                                      const std::set<std::string>& tuple_struct_names);

} // namespace ari
