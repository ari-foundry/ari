#pragma once

#include "ir.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ari {

struct ModuleCacheIrFunctionSummary;

struct ModuleCacheIrLayoutDescriptor {
    std::string kind;
    std::string type;
    std::string element_type;
    std::uint64_t slot_count = 0;
};

std::vector<ModuleCacheIrLayoutDescriptor>
module_cache_ir_layout_descriptors(const std::vector<const IrFunction*>& functions);

std::vector<ModuleCacheIrLayoutDescriptor>
module_cache_ir_layout_descriptors(const std::vector<ModuleCacheIrFunctionSummary>& functions);

void validate_module_cache_ir_layout_descriptors(
    const std::vector<ModuleCacheIrLayoutDescriptor>& descriptors,
    const std::vector<ModuleCacheIrFunctionSummary>& functions);

} // namespace ari
