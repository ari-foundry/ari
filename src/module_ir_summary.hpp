#pragma once

#include "ir.hpp"
#include "module_cache.hpp"

#include <string>

namespace ari {

ModuleCacheIrSummary make_module_cache_ir_summary(const ModuleCacheSource& source,
                                                  const IrProgram& ir);
void attach_module_cache_ir_summaries(ModuleCache& cache, const IrProgram& ir);
void require_valid_module_cache_ir_summary_payload(const ModuleCacheIrSummary& summary,
                                                   const std::string& display_path);

} // namespace ari
