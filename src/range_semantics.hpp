#pragma once

#include "ir.hpp"

#include <string>

namespace ari {

bool is_prelude_range_type_name(const std::string& name);
bool is_prelude_range_type(const IrType& type);
IrType make_prelude_range_type(SourceLocation loc, bool inclusive, IrType bound);
IrType make_prelude_range_type(SourceLocation loc, bool inclusive);

} // namespace ari
