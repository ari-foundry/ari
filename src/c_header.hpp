#pragma once

#include "ir.hpp"

#include <string>

namespace ari {

std::string emit_c_header(const IrProgram& program);

} // namespace ari
