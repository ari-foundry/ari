#pragma once

#include "ir.hpp"
#include "target.hpp"

#include <string>

namespace ari {

bool c_abi_type_alias(const std::string& name,
                      const TargetInfo& target,
                      IrPrimitiveKind& primitive,
                      std::string& canonical);

} // namespace ari
