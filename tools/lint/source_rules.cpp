#include "source_rules.hpp"

#include <fstream>
#include <optional>
#include <sstream>
#include <utility>

namespace ari::lint {
namespace {

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

bool is_trailing_space(char c) {
    return c == ' ' || c == '\t';
}

std::optional<std::string> read_file(const std::string& file) {
    std::ifstream in(file, std::ios::binary);
    if (!in) return std::nullopt;
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

std::pair<int, int> final_position(const std::string& text) {
    int line = 1;
    int column = 1;
    for (char c : text) {
        if (c == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }
    return {line, column};
}

} // namespace

std::vector<tooling::Diagnostic> check_trailing_whitespace(const RuleSettings& settings,
                                                           const std::string& file) {
    const RuleDescriptor rule = find_rule(rule_trailing_whitespace).value();
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

std::vector<tooling::Diagnostic> check_missing_final_newline(const RuleSettings& settings,
                                                             const std::string& file) {
    const RuleDescriptor rule = find_rule(rule_missing_final_newline).value();
    RuleSetting setting = setting_for(settings, rule);
    if (!setting.enabled) return {};

    std::optional<std::string> text = read_file(file);
    if (!text || text->empty() || text->back() == '\n') return {};

    auto [line, column] = final_position(*text);
    return {
        tooling::Diagnostic{
            file,
            line,
            column,
            setting.severity,
            "missing final newline",
            "ari-lint",
            rule.code,
            line,
            column + 1,
        },
    };
}

} // namespace ari::lint
