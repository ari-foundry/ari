#include "raw_string_pool.hpp"

#include "common.hpp"

#include <cstdint>
#include <limits>

namespace ari {

namespace {

void align_image(std::vector<std::uint8_t>& image, std::size_t alignment) {
    while (alignment != 0 && image.size() % alignment != 0) image.push_back(0);
}

void patch_i32(std::vector<std::uint8_t>& image, std::size_t at, std::int32_t value) {
    std::uint32_t bits = static_cast<std::uint32_t>(value);
    for (int i = 0; i < 4; ++i) {
        image[at + static_cast<std::size_t>(i)] =
            static_cast<std::uint8_t>((bits >> (i * 8)) & 0xff);
    }
}

} // namespace

std::size_t RawStringPool::string_offset(std::vector<std::uint8_t>& image,
                                         const std::string& value) {
    auto found = offsets_.find(value);
    if (found != offsets_.end()) return found->second;

    align_image(image, 8);
    std::size_t offset = image.size();
    image.insert(image.end(), value.begin(), value.end());
    image.push_back(0);
    offsets_[value] = offset;
    return offset;
}

void RawStringPool::patch_references(std::vector<std::uint8_t>& image,
                                     const std::vector<RawStringReference>& references) {
    for (const auto& reference : references) {
        std::size_t target = string_offset(image, reference.value);
        std::int64_t rel = static_cast<std::int64_t>(target) -
                           static_cast<std::int64_t>(reference.imm_offset + 4);
        if (rel < std::numeric_limits<std::int32_t>::min() ||
            rel > std::numeric_limits<std::int32_t>::max()) {
            throw CompileError("raw string literal storage is too far from its reference");
        }
        patch_i32(image, reference.imm_offset, static_cast<std::int32_t>(rel));
    }
}

} // namespace ari
