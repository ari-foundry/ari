#pragma once

#include <string>

namespace ari {

// Transitional diagnostic artifact for the current string-based CompileError path.
std::string classify_diagnostic_code(const std::string& message);
std::string diagnostic_code_family(const std::string& code);

std::string dump_diagnostic_message(const std::string& severity,
                                    const std::string& code,
                                    const std::string& message,
                                    const std::string& source_name);

} // namespace ari
