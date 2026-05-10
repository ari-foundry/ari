#pragma once

#include "ast.hpp"
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
bool trait_method_param_is_self_receiver(const TypeRef& param);
bool trait_method_has_self_receiver(const std::vector<TypeRef>& params);

} // namespace ari
