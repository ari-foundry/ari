#pragma once

#include "ir.hpp"

#include <string>
#include <vector>

namespace ari {

std::string trait_application_display(const std::string& trait_name, const std::vector<IrType>& trait_args);
std::string trait_method_display(const std::string& trait_name,
                                 const std::vector<IrType>& trait_args,
                                 const std::string& method_name);
std::string trait_impl_key(const std::string& trait_name,
                           const std::vector<IrType>& trait_args,
                           const IrType& self_type);

} // namespace ari
