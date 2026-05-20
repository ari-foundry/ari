#pragma once

#include "ir.hpp"

#include <string>

namespace ari {

// Stable typed-IR artifact used before LLVM/backend checks.
std::string dump_ir_program(const IrProgram& program, const std::string& source_name);

} // namespace ari
