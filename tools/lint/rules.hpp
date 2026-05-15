#pragma once

#include "../ari_tooling/diagnostic.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace ari::lint {

struct RuleSetting {
    bool enabled = true;
    tooling::DiagnosticSeverity severity = tooling::DiagnosticSeverity::Warning;
};

struct RuleDescriptor {
    std::string code;
    std::string name;
    std::string description;
    tooling::DiagnosticSeverity default_severity = tooling::DiagnosticSeverity::Warning;
};

using RuleSettings = std::map<std::string, RuleSetting>;

constexpr const char* rule_trailing_whitespace = "lint/trailing-whitespace";
constexpr const char* rule_missing_final_newline = "lint/missing-final-newline";

std::vector<RuleDescriptor> registered_rules();
std::optional<RuleSetting> parse_rule_setting(const std::string& text);
std::string normalize_rule_code(const std::string& code);
std::vector<tooling::Diagnostic> run_native_rules(const RuleSettings& settings, const std::string& file);

} // namespace ari::lint
