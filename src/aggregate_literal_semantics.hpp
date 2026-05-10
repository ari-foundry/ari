#pragma once

#include "ir.hpp"

#include <cstddef>
#include <string>

namespace ari {

const IrType* tuple_literal_expected_element_type(const IrType* expected,
                                                  std::size_t arity,
                                                  std::size_t index);

const IrType* vector_literal_expected_element_type(const IrType* expected);

bool expected_type_matches_struct_literal(const IrType* expected,
                                          const std::string& struct_name,
                                          std::size_t generic_arity,
                                          std::size_t field_count);

} // namespace ari
