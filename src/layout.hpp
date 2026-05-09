#pragma once

#include "ir.hpp"

#include <cstdint>

namespace ari {

bool ari_layout_size_bytes(const IrType& type, std::uint64_t& out);
bool ari_layout_align_bytes(const IrType& type, std::uint64_t& out);

} // namespace ari
