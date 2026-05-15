#include "config.hpp"

#include <cctype>
#include <fstream>

namespace ari::lint {
namespace {

std::string trim(const std::string& text) {
    std::size_t begin = 0;
    while (begin < text.size() && std::isspace(static_cast<unsigned char>(text[begin]))) ++begin;
    std::size_t end = text.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(begin, end - begin);
}

std::string strip_comment(const std::string& line) {
    std::size_t comment = line.find('#');
    if (comment == std::string::npos) return line;
    return line.substr(0, comment);
}

} // namespace

bool apply_rule_setting(RuleSettings& settings, const std::string& setting_text, std::string* error) {
    std::size_t equals = setting_text.find('=');
    if (equals == std::string::npos) {
        if (error) *error = "expected RULE=SEVERITY";
        return false;
    }

    std::string code = trim(setting_text.substr(0, equals));
    std::string severity = trim(setting_text.substr(equals + 1));
    std::optional<RuleSetting> setting = parse_rule_setting(code + "=" + severity);
    if (!setting) {
        if (error) *error = "unknown rule or severity";
        return false;
    }
    settings[normalize_rule_code(code)] = *setting;
    return true;
}

RuleConfigLoadResult load_rule_config(const std::string& path) {
    RuleConfigLoadResult result;
    std::ifstream in(path);
    if (!in) {
        result.errors.push_back(path + ": cannot open lint config");
        return result;
    }

    std::string line;
    int line_number = 1;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string setting_text = trim(strip_comment(line));
        if (setting_text.empty()) {
            ++line_number;
            continue;
        }
        std::string error;
        if (!apply_rule_setting(result.settings, setting_text, &error)) {
            result.errors.push_back(path + ":" + std::to_string(line_number) + ": " + error);
        }
        ++line_number;
    }
    return result;
}

} // namespace ari::lint
