#pragma once

#include "rules.hpp"

#include <string>
#include <vector>

namespace ari::lint {

struct RuleConfigLoadResult {
    RuleSettings settings;
    std::vector<std::string> errors;
};

bool apply_rule_setting(RuleSettings& settings, const std::string& setting_text, std::string* error);
RuleConfigLoadResult load_rule_config(const std::string& path);

} // namespace ari::lint
