#pragma once

#include "../ari_tooling/diagnostic.hpp"
#include "rules.hpp"

#include <string>
#include <vector>

namespace ari::lint {

struct LintConfig {
    std::string ari_path = "build/ari";
    std::vector<std::string> module_paths;
    RuleSettings rule_settings;
};

struct LintResult {
    std::string file;
    int exit_code = 0;
    std::string raw_output;
    std::vector<tooling::Diagnostic> diagnostics;
};

LintResult run_lint(const LintConfig& config, const std::string& file);

} // namespace ari::lint
