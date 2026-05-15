#pragma once

#include "rules.hpp"

#include <optional>
#include <string>
#include <vector>

namespace ari::lint {

struct RuleConfigLoadResult {
    RuleSettings settings;
    std::vector<std::string> errors;
};

bool apply_rule_setting(RuleSettings& settings, const std::string& setting_text, std::string* error);
std::optional<std::string> find_rule_config_for_file(const std::string& source_path);
RuleConfigLoadResult load_rule_config(const std::string& path);

} // namespace ari::lint
