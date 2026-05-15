#include "rules.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace ari::lint {
namespace {

constexpr const char* trailing_whitespace_code = "lint/trailing-whitespace";

bool is_trailing_space(char c) {
    return c == ' ' || c == '\t';
}

std::optional<RuleDescriptor> find_rule(const std::string& code) {
    for (const RuleDescriptor& rule : registered_rules()) {
        if (rule.code == code) return rule;
    }
    return std::nullopt;
}

RuleSetting setting_for(const RuleSettings& settings, const RuleDescriptor& rule) {
    auto it = settings.find(rule.code);
    if (it != settings.end()) return it->second;
    return RuleSetting{true, rule.default_severity};
}

std::vector<tooling::Diagnostic> check_trailing_whitespace(const RuleSettings& settings, const std::string& file) {
    const RuleDescriptor rule = find_rule(trailing_whitespace_code).value();
    RuleSetting setting = setting_for(settings, rule);
    if (!setting.enabled) return {};

    std::ifstream in(file);
    if (!in) return {};

    std::vector<tooling::Diagnostic> diagnostics;
    std::string line;
    int line_number = 1;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::size_t end = line.size();
        while (end > 0 && is_trailing_space(line[end - 1])) --end;
        if (end != line.size()) {
            diagnostics.push_back(tooling::Diagnostic{
                file,
                line_number,
                static_cast<int>(end + 1),
                setting.severity,
                "trailing whitespace",
                "ari-lint",
                rule.code,
                line_number,
                static_cast<int>(line.size() + 1),
            });
        }
        ++line_number;
    }
    return diagnostics;
}

} // namespace

std::vector<RuleDescriptor> registered_rules() {
    return {
        RuleDescriptor{
            trailing_whitespace_code,
            "Trailing whitespace",
            "Reports spaces or tabs at the end of a source line.",
            tooling::DiagnosticSeverity::Warning,
        },
    };
}

std::string normalize_rule_code(const std::string& code) {
    if (code.find('/') != std::string::npos) return code;
    return "lint/" + code;
}

std::optional<RuleSetting> parse_rule_setting(const std::string& text) {
    std::size_t equals = text.find('=');
    if (equals == std::string::npos) return std::nullopt;

    std::string code = normalize_rule_code(text.substr(0, equals));
    std::string value = text.substr(equals + 1);
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (!find_rule(code)) return std::nullopt;
    if (value == "off") return RuleSetting{false, tooling::DiagnosticSeverity::Warning};
    if (auto severity = tooling::parse_severity_name(value)) return RuleSetting{true, *severity};
    return std::nullopt;
}

std::vector<tooling::Diagnostic> run_native_rules(const RuleSettings& settings, const std::string& file) {
    std::vector<tooling::Diagnostic> diagnostics;
    std::vector<tooling::Diagnostic> trailing = check_trailing_whitespace(settings, file);
    diagnostics.insert(diagnostics.end(), trailing.begin(), trailing.end());
    return diagnostics;
}

} // namespace ari::lint
