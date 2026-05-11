#pragma once

#include "ir.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace ari {

std::optional<std::size_t> borrow_return_param_index(const std::vector<IrType>& params,
                                                     const IrType& result);

} // namespace ari
