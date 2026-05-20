#pragma once

#include "ir.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace ari {

bool is_zone_value_type(const IrType& type);
bool is_zone_borrow_type(const IrType& type);
bool is_zone_source_type(const IrType& type);
bool is_zone_pointer_return_type(const IrType& type);

std::optional<std::size_t> zone_pointer_return_param_index(const std::vector<IrType>& params,
                                                           const IrType& result);

} // namespace ari
