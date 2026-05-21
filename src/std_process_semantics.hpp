#pragma once

#include "ir.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace ari {

bool is_std_process_output_zone_handle_type(const IrType& type);
std::optional<std::size_t> std_process_output_zone_handle_source_field_index(const IrType& type);
std::vector<std::vector<std::size_t>> std_process_output_zone_handle_storage_field_path_indices(const IrType& type);

} // namespace ari
