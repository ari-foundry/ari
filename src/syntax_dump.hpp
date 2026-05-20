#pragma once

#include "ast.hpp"

#include <string>

namespace ari {

// Source-shaped parser artifact used by focused compiler regression tests.
std::string dump_syntax(const Program& program, const std::string& source_name);

} // namespace ari
