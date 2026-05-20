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

// Deterministic stage-order artifact for compiler-development triage. This
// keeps the artifact ladder visible from the compiler binary itself, before a
// reviewer has to inspect LLVM or linked executable behavior.
std::string dump_compiler_stage_plan(const std::string& source_name,
                                     const std::string& target_triple,
                                     bool implicit_std,
                                     std::size_t module_search_path_count,
                                     std::size_t cfg_feature_count);

} // namespace ari
