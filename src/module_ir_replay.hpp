#pragma once

#include "ast.hpp"
#include "ir.hpp"
#include "module_ir_summary.hpp"

#include <set>
#include <string>
#include <vector>

namespace ari {

std::set<std::string> module_cache_ir_function_names(
    const std::vector<ModuleCacheIrFunctionSummary>& functions);

std::vector<IrFunction> replay_module_cache_ir_functions(
    const std::vector<ModuleCacheIrFunctionSummary>& functions,
    const Program& declarations);

} // namespace ari
