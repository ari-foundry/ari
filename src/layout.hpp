#pragma once

#include "ir.hpp"

#include <cstdint>
#include <vector>

namespace ari {

bool ari_has_aggregate_enum_layout(const IrType& type);
bool ari_is_aggregate_layout_type(const IrType& type);
const std::vector<IrType>& ari_aggregate_field_types(const IrType& type);
bool ari_layout_field_count(const IrType& type, std::uint64_t& out);
bool ari_layout_size_bytes(const IrType& type, std::uint64_t& out);
bool ari_layout_align_bytes(const IrType& type, std::uint64_t& out);
bool ari_layout_field_offset_bytes(const IrType& type, std::uint64_t index, std::uint64_t& out);

} // namespace ari
