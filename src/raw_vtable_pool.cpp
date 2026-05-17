#include "raw_vtable_pool.hpp"

#include "common.hpp"

#include <cstdint>
#include <limits>

namespace ari {

namespace {

void align_image(std::vector<std::uint8_t>& image, std::size_t alignment) {
    while (alignment != 0 && image.size() % alignment != 0) image.push_back(0);
}

void append_u64(std::vector<std::uint8_t>& image, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        image.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xff));
    }
}

void patch_i32(std::vector<std::uint8_t>& image, std::size_t at, std::int32_t value) {
    std::uint32_t bits = static_cast<std::uint32_t>(value);
    for (int i = 0; i < 4; ++i) {
        image[at + static_cast<std::size_t>(i)] =
            static_cast<std::uint8_t>((bits >> (i * 8)) & 0xff);
    }
}

} // namespace

void RawVTablePool::emit_tables(std::vector<std::uint8_t>& image,
                                const std::vector<IrTraitObjectVTable>& tables) {
    for (const auto& table : tables) {
        if (offsets_.count(table.name)) continue;
        align_image(image, 8);
        offsets_[table.name] = image.size();

        if (table.methods.empty()) {
            append_u64(image, 0);
            continue;
        }

        for (const auto& method : table.methods) {
            std::size_t entry_offset = image.size();
            append_u64(image, 0);
            entry_patches_.push_back(RawVTableEntryPatch{entry_offset, method.thunk_name});
        }
    }
}

void RawVTablePool::patch_references(std::vector<std::uint8_t>& image,
                                     const std::vector<RawVTableReference>& references) const {
    for (const auto& reference : references) {
        auto found = offsets_.find(reference.name);
        if (found == offsets_.end()) {
            throw CompileError("raw trait-object vtable '" + reference.name + "' was not emitted");
        }
        std::int64_t rel = static_cast<std::int64_t>(found->second) -
                           static_cast<std::int64_t>(reference.imm_offset + 4);
        if (rel < std::numeric_limits<std::int32_t>::min() ||
            rel > std::numeric_limits<std::int32_t>::max()) {
            throw CompileError("raw trait-object vtable is too far from its reference");
        }
        patch_i32(image, reference.imm_offset, static_cast<std::int32_t>(rel));
    }
}

} // namespace ari
