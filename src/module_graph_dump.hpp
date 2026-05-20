#pragma once

#include "module_metadata.hpp"

#include <string>

namespace ari {

std::string dump_module_graph(const ModuleMetadata& metadata, const std::string& source_name);

} // namespace ari
