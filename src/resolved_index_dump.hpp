#pragma once

#include "ir.hpp"

#include <string>

namespace ari {

std::string dump_resolved_index(const IrProgram& program, const std::string& source_name);

} // namespace ari
