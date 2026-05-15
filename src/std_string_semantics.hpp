#pragma once

#include "ir.hpp"

#include <cstddef>
#include <optional>
#include <string>

namespace ari {

bool is_std_string_raw_handle_type(const IrType& type);
bool is_std_string_handle_type(const IrType& type);
bool is_std_string_zone_handle_type(const IrType& type);
std::optional<std::size_t> std_string_raw_handle_data_field_index(const IrType& type);
std::optional<std::size_t> std_string_zone_handle_source_field_index(const IrType& type);
bool std_string_pointer_result_preserves_receiver_zone(const IrExpr& call);
bool std_string_extern_builtin_allows_zone_pointer_argument(const std::string& function_name,
                                                           std::size_t arg_index);

} // namespace ari
