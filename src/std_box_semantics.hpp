#pragma once

#include "ir.hpp"

#include <cstddef>
#include <optional>

namespace ari {

bool is_std_box_handle_type(const IrType& type);
std::optional<std::size_t> std_box_zone_handle_source_field_index(const IrType& type);

} // namespace ari
