#include "product_coverage.hpp"

#include <algorithm>
#include <utility>

namespace ari {

bool product_rect_intersection(const ProductRect& left, const ProductRect& right, ProductRect& out) {
    if (left.size() != right.size()) return false;
    out.clear();
    out.reserve(left.size());
    for (std::size_t i = 0; i < left.size(); ++i) {
        std::uint64_t start = std::max(left[i].start, right[i].start);
        std::uint64_t end = std::min(left[i].end, right[i].end);
        if (start > end) return false;
        out.push_back(ProductInterval{start, end});
    }
    return true;
}

void subtract_product_rect(const ProductRect& target,
                           const ProductRect& cover,
                           std::vector<ProductRect>& out) {
    ProductRect intersection;
    if (!product_rect_intersection(target, cover, intersection)) {
        out.push_back(target);
        return;
    }

    ProductRect middle = target;
    for (std::size_t i = 0; i < target.size(); ++i) {
        if (middle[i].start < intersection[i].start) {
            ProductRect piece = middle;
            piece[i].end = intersection[i].start - 1;
            out.push_back(std::move(piece));
            middle[i].start = intersection[i].start;
        }
        if (intersection[i].end < middle[i].end) {
            ProductRect piece = middle;
            piece[i].start = intersection[i].end + 1;
            out.push_back(std::move(piece));
            middle[i].end = intersection[i].end;
        }
    }
}

bool product_rect_is_covered_by(const ProductRect& target, const std::vector<ProductRect>& covers) {
    std::vector<ProductRect> remaining{target};
    for (const auto& cover : covers) {
        if (remaining.empty()) return true;
        std::vector<ProductRect> next;
        for (const auto& item : remaining) {
            subtract_product_rect(item, cover, next);
        }
        remaining = std::move(next);
    }
    return remaining.empty();
}

bool product_rects_are_covered_by(const std::vector<ProductRect>& targets,
                                  const std::vector<ProductRect>& covers) {
    for (const auto& target : targets) {
        if (!product_rect_is_covered_by(target, covers)) return false;
    }
    return true;
}

bool product_rect_first_gap(const ProductRect& target,
                            const std::vector<ProductRect>& covers,
                            ProductRect& out) {
    std::vector<ProductRect> remaining{target};
    for (const auto& cover : covers) {
        if (remaining.empty()) return false;
        std::vector<ProductRect> next;
        for (const auto& item : remaining) {
            subtract_product_rect(item, cover, next);
        }
        remaining = std::move(next);
    }
    if (remaining.empty()) return false;
    out = remaining.front();
    return true;
}

void append_product_rect(ProductRect& out, const ProductRect& item) {
    out.insert(out.end(), item.begin(), item.end());
}

bool combine_product_rect_domains(const std::vector<std::vector<ProductRect>>& domains,
                                  std::size_t max_rectangles,
                                  std::vector<ProductRect>& out) {
    std::vector<ProductRect> result{ProductRect{}};
    for (const auto& domain : domains) {
        if (domain.empty()) return false;
        if (result.size() > max_rectangles / domain.size()) return false;
        std::vector<ProductRect> next;
        next.reserve(result.size() * domain.size());
        for (const auto& prefix : result) {
            for (const auto& item : domain) {
                ProductRect combined = prefix;
                append_product_rect(combined, item);
                next.push_back(std::move(combined));
            }
        }
        result = std::move(next);
    }
    out = std::move(result);
    return true;
}

} // namespace ari
