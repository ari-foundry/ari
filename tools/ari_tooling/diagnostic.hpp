#pragma once

#include <string>
#include <optional>
#include <vector>

namespace ari::tooling {

enum class DiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4,
};

struct Diagnostic {
    std::string file;
    int line = 1;
    int column = 1;
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string message;
    std::string source = "ari";
    std::string code;
    int end_line = 0;
    int end_column = 0;
};

std::vector<Diagnostic> parse_ari_diagnostics(const std::string& text, const std::string& fallback_file);

std::string json_escape(const std::string& text);
std::optional<DiagnosticSeverity> parse_severity_name(const std::string& text);
std::string severity_name(DiagnosticSeverity severity);
int lsp_severity(DiagnosticSeverity severity);

} // namespace ari::tooling
