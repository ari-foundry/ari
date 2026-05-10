#pragma once

#include "ir.hpp"

#include <cstddef>
#include <string>

namespace ari {

bool expected_type_matches_enum_constructor(const IrType* expected,
                                            const std::string& enum_name,
                                            std::size_t generic_arity);

} // namespace ari
