#pragma once

#include "ir.hpp"

#include <string>
#include <vector>

namespace ari {

void append_ir_summary_body_shape(std::string& out, const std::vector<IrStmtPtr>& body);
void append_ir_summary_body_tree(std::string& out, const std::vector<IrStmtPtr>& body);

} // namespace ari
