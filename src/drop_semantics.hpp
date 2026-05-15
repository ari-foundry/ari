#pragma once

#include "ir.hpp"

#include <cstddef>
#include <string>

namespace ari {

bool is_drop_trait_method_name(const std::string& trait_name, const std::string& method_name);
bool is_valid_drop_method_signature(std::size_t param_count, const IrType& result_type);
std::string invalid_drop_impl_internal_message(const IrType& receiver_type);
std::string ambiguous_drop_impl_message(const IrType& dropped_type);

} // namespace ari
