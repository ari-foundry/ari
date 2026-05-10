#pragma once

#include "ir.hpp"

#include <string>

namespace ari {

bool c_abi_type_alias(const std::string& name, IrPrimitiveKind& primitive, std::string& canonical);

} // namespace ari
