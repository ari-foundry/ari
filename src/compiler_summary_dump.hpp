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

// Input-free pass catalog for compiler contributors. Unlike
// dump_compiler_pass_summary, this does not inspect one program; it documents
// the compiler's stable pass boundaries, ownership, inputs, outputs, and first
// focused checks.
std::string dump_compiler_pass_catalog();

// Single-pass view used by the CLI when routing a compiler change to the first
// owning layer before choosing a fixture or artifact.
std::string dump_compiler_pass_explanation(const std::string& pass_name);

// Input-free test-bucket catalog for compiler contributors. It keeps fixture
// placement rules discoverable from the compiler binary itself, so tests stay
// grouped by the behavior they prove instead of by the file that changed.
std::string dump_compiler_test_bucket_catalog();

// Single-bucket view used when deciding where a focused compiler test belongs.
std::string dump_compiler_test_bucket_explanation(const std::string& bucket_name);

// Input-free implementation queue for normal compiler development. It keeps the
// near-term roadmap available from the compiler binary with first files,
// artifacts, and checks attached to each work item.
std::string dump_compiler_work_item_catalog();

// Single-work-item view used when selecting the next small compiler change.
std::string dump_compiler_work_item_explanation(const std::string& item_name);

// Deterministic inventory of the compiler's current public capability surface.
// This is intentionally coarse-grained: it gives contributors a stable map of
// implemented, partial, planned, and intentionally rejected features without
// pretending to be a full language specification.
std::string dump_compiler_capability_inventory(const std::string& target_triple,
                                               bool implicit_std);

// Single-capability view for CLI triage. It lets a contributor ask "what owns
// this feature surface?" without producing a file-backed inventory artifact.
std::string dump_compiler_capability_explanation(const std::string& capability_name);

} // namespace ari
