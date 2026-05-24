#pragma once

#include "ast.hpp"
#include "common.hpp"

#include <string>
#include <vector>

namespace ari {

// Source-shaped parser artifact used by focused compiler regression tests.
std::string dump_syntax(const Program& program, const std::string& source_name);
std::string dump_syntax(const Program& program,
                        const std::string& source_name,
                        const std::vector<CompileError>& parse_diagnostics);

} // namespace ari
