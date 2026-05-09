#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ari {

struct ProductInterval {
    std::uint64_t start = 0;
    std::uint64_t end = 0;
};

using ProductRect = std::vector<ProductInterval>;

bool product_rect_intersection(const ProductRect& left, const ProductRect& right, ProductRect& out);
void subtract_product_rect(const ProductRect& target, const ProductRect& cover, std::vector<ProductRect>& out);
bool product_rect_is_covered_by(const ProductRect& target, const std::vector<ProductRect>& covers);
bool product_rects_are_covered_by(const std::vector<ProductRect>& targets, const std::vector<ProductRect>& covers);
bool product_rect_first_gap(const ProductRect& target, const std::vector<ProductRect>& covers, ProductRect& out);
void append_product_rect(ProductRect& out, const ProductRect& item);
bool combine_product_rect_domains(const std::vector<std::vector<ProductRect>>& domains,
                                  std::size_t max_rectangles,
                                  std::vector<ProductRect>& out);

} // namespace ari
