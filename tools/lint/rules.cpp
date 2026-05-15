#include "rules.hpp"
#include "source_rules.hpp"

#include <algorithm>
#include <cctype>

namespace ari::lint {
namespace {

std::string trim(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(begin, end - begin);
}

std::optional<RuleDescriptor> find_rule(const std::string& code) {
    for (const RuleDescriptor& rule : registered_rules()) {
        if (rule.code == code) return rule;
    }
    return std::nullopt;
}

} // namespace

std::vector<RuleDescriptor> registered_rules() {
    return {
        RuleDescriptor{
            rule_trailing_whitespace,
            "Trailing whitespace",
            "Reports spaces or tabs at the end of a source line.",
            tooling::DiagnosticSeverity::Warning,
        },
        RuleDescriptor{
            rule_missing_final_newline,
            "Missing final newline",
            "Reports non-empty source files that do not end with a newline.",
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

    std::string code = normalize_rule_code(trim(text.substr(0, equals)));
    std::string value = trim(text.substr(equals + 1));
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
    std::vector<tooling::Diagnostic> final_newline = check_missing_final_newline(settings, file);
    diagnostics.insert(diagnostics.end(), final_newline.begin(), final_newline.end());
    return diagnostics;
}

} // namespace ari::lint
