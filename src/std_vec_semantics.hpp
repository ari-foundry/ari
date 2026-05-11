#pragma once

#include "ir.hpp"

#include <cstddef>
#include <optional>

namespace ari {

bool is_std_vec_raw_handle_type(const IrType& type);
bool is_std_vec_handle_type(const IrType& type);
bool is_std_vec_zone_handle_type(const IrType& type);
std::optional<std::size_t> std_vec_raw_handle_data_field_index(const IrType& type);
std::optional<std::size_t> std_vec_zone_handle_source_field_index(const IrType& type);

} // namespace ari
