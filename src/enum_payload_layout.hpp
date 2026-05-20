#pragma once

#include "ir.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace ari {

IrType enum_tag_storage_type(SourceLocation loc);
IrType enum_payload_storage_type(SourceLocation loc);
IrType enum_payload_byte_storage_type(SourceLocation loc, std::uint64_t size_bytes);
IrType enum_payload_slot_storage_type(SourceLocation loc, const IrType& payload_type);

const IrType* enum_payload_slot_scalar_lane_type(const IrType& slot_type);
std::optional<std::uint32_t> enum_payload_slot_scalar_lane_index(const IrType& slot_type);
bool enum_payload_slot_uses_byte_storage(const IrType& slot_type, const IrType& payload_type);
bool enum_payload_slot_uses_scalar_lane(const IrType& slot_type, const IrType& payload_type);
bool merge_enum_payload_slot_storage_type(SourceLocation loc,
                                          IrType& slot_type,
                                          const IrType& payload_type,
                                          std::string& error);

} // namespace ari
