#pragma once

#include "ir.hpp"

#include <string>

namespace ari {

std::string emit_llvm_ir(const IrProgram& program);

} // namespace ari
