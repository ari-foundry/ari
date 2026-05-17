#pragma once

#include "ir.hpp"
#include "module_cache.hpp"
#include "module_ir_summary_body.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ari {

struct ModuleCacheIrParamSummary {
    std::string name;
    std::string type;
};

struct ModuleCacheIrSpecializationArgSummary {
    std::string name;
    std::string type;
};

struct ModuleCacheIrFunctionSummary {
    std::string name;
    std::string module_name;
    std::string link_name;
    std::string return_type;
    std::vector<ModuleCacheIrParamSummary> params;
    std::uint64_t body_statement_count = 0;
    ModuleCacheIrBodySummary body;
    std::string specialization_kind;
    std::string specialization_origin;
    std::vector<ModuleCacheIrSpecializationArgSummary> specialization_args;
    bool shared_export = false;
};

ModuleCacheIrSummary make_module_cache_ir_summary(const ModuleCacheSource& source,
                                                  const IrProgram& ir);
void attach_module_cache_ir_summaries(ModuleCache& cache, const IrProgram& ir);
void require_valid_module_cache_ir_summary_payload(const ModuleCacheIrSummary& summary,
                                                   const std::string& display_path);
std::vector<ModuleCacheIrFunctionSummary>
materialize_module_cache_ir_summary_functions(const ModuleCacheIrSummary& summary,
                                              const std::string& display_path);

} // namespace ari
