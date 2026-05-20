#pragma once

#include "ast.hpp"
#include "module_metadata.hpp"

#include <string>

namespace ari {

std::string dump_declaration_index(const Program& program,
                                   const ModuleMetadata& metadata,
                                   const std::string& source_name);

} // namespace ari
