#pragma once

#include "ast.hpp"

namespace ari {

bool is_take_place_expression(const Expr& expr);
const char* take_place_expectation();

} // namespace ari
