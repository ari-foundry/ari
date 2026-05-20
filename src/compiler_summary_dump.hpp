#pragma once

#include "ast.hpp"
#include "ir.hpp"
#include "module_metadata.hpp"

#include <cstddef>
#include <string>

namespace ari {

// Small pass-level artifact for compiler-development checks. It intentionally
// summarizes counts and stage boundaries instead of duplicating token/syntax/IR dumps.
std::string dump_compiler_pass_summary(const std::string& source_name,
                                       std::size_t token_count,
                                       const Program& program,
                                       const ModuleMetadata& metadata,
                                       const IrProgram& ir);

} // namespace ari
