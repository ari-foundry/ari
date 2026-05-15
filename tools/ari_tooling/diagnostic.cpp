#include "diagnostic.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

namespace ari::tooling {
namespace {

DiagnosticSeverity parse_severity(const std::string& text) {
    if (text == "warning") return DiagnosticSeverity::Warning;
    if (text == "note") return DiagnosticSeverity::Information;
    if (text == "hint") return DiagnosticSeverity::Hint;
    return DiagnosticSeverity::Error;
}

std::string trim_carriage_return(std::string line) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return line;
}

} // namespace

std::vector<Diagnostic> parse_ari_diagnostics(const std::string& text, const std::string& fallback_file) {
    std::vector<Diagnostic> diagnostics;
    std::istringstream in(text);
    std::string line;
    const std::regex ari_pattern(R"(^ari: (error|warning|note|hint): ([0-9]+):([0-9]+): (.*)$)");
    const std::regex file_pattern(R"(^(.+):([0-9]+):([0-9]+): (error|warning|note|hint): (.*)$)");
    const std::regex general_pattern(R"(^ari: (error|warning|note|hint): (.*)$)");

    while (std::getline(in, line)) {
        line = trim_carriage_return(line);
        std::smatch match;
        if (std::regex_match(line, match, ari_pattern)) {
            diagnostics.push_back(Diagnostic{
                fallback_file,
                std::max(1, std::stoi(match[2].str())),
                std::max(1, std::stoi(match[3].str())),
                parse_severity(match[1].str()),
                match[4].str(),
                "ari",
            });
            continue;
        }
        if (std::regex_match(line, match, file_pattern)) {
            diagnostics.push_back(Diagnostic{
                match[1].str(),
                std::max(1, std::stoi(match[2].str())),
                std::max(1, std::stoi(match[3].str())),
                parse_severity(match[4].str()),
                match[5].str(),
                "ari",
            });
            continue;
        }
        if (std::regex_match(line, match, general_pattern)) {
            diagnostics.push_back(Diagnostic{
                fallback_file,
                1,
                1,
                parse_severity(match[1].str()),
                match[2].str(),
                "ari",
            });
        }
    }
    return diagnostics;
}

std::string json_escape(const std::string& text) {
    std::string out;
    out.reserve(text.size() + 8);
    static const char* hex = "0123456789abcdef";
    for (unsigned char c : text) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    out += "\\u00";
                    out.push_back(hex[(c >> 4) & 0xf]);
                    out.push_back(hex[c & 0xf]);
                } else {
                    out.push_back(static_cast<char>(c));
                }
                break;
        }
    }
    return out;
}

std::string severity_name(DiagnosticSeverity severity) {
    switch (severity) {
        case DiagnosticSeverity::Warning: return "warning";
        case DiagnosticSeverity::Information: return "note";
        case DiagnosticSeverity::Hint: return "hint";
        case DiagnosticSeverity::Error: return "error";
    }
    return "error";
}

int lsp_severity(DiagnosticSeverity severity) {
    return static_cast<int>(severity);
}

} // namespace ari::tooling
