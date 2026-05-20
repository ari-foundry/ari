#pragma once

#include <string>

namespace ari {

// Transitional diagnostic artifact for the current string-based CompileError path.
std::string dump_diagnostic_message(const std::string& severity,
                                    const std::string& code,
                                    const std::string& message,
                                    const std::string& source_name);

} // namespace ari
