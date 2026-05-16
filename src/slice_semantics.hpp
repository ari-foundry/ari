#pragma once

#include "ir.hpp"

#include <string>

namespace ari {

bool is_prelude_slice_type(const IrType& type);
IrType make_prelude_slice_type(SourceLocation loc, const IrType& element);
bool slice_pointer_result_preserves_receiver_zone(const IrExpr& call);
void require_slice_element_materializable(SourceLocation loc,
                                          const IrType& element_type,
                                          const std::string& operation);

} // namespace ari
