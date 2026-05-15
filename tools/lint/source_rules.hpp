#pragma once

#include "rules.hpp"

#include <string>
#include <vector>

namespace ari::lint {

std::vector<tooling::Diagnostic> check_trailing_whitespace(const RuleSettings& settings,
                                                           const std::string& file);
std::vector<tooling::Diagnostic> check_missing_final_newline(const RuleSettings& settings,
                                                             const std::string& file);

} // namespace ari::lint
