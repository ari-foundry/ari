#include "diagnostic_dump.hpp"

#include <cctype>
#include <sstream>
#include <string>

namespace ari {

namespace {

static std::string quote(const std::string& text) {
    std::string escaped = "\"";
    for (unsigned char c : text) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                if (c < 0x20 || c == 0x7f) {
                    const char* digits = "0123456789abcdef";
                    escaped += "\\x";
                    escaped.push_back(digits[(c >> 4) & 0xf]);
                    escaped.push_back(digits[c & 0xf]);
                } else {
                    escaped.push_back(static_cast<char>(c));
                }
                break;
        }
    }
    escaped += "\"";
    return escaped;
}

static bool decimal_prefix(const std::string& text, std::size_t start, std::size_t end) {
    if (start == end) return false;
    for (std::size_t i = start; i < end; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
    }
    return true;
}

static bool parse_location_prefix(const std::string& message,
                                  int& line,
                                  int& column,
                                  std::string& rest) {
    std::size_t first_colon = message.find(':');
    if (first_colon == std::string::npos) return false;
    std::size_t second_colon = message.find(':', first_colon + 1);
    if (second_colon == std::string::npos) return false;
    if (!decimal_prefix(message, 0, first_colon) ||
        !decimal_prefix(message, first_colon + 1, second_colon)) {
        return false;
    }

    line = std::stoi(message.substr(0, first_colon));
    column = std::stoi(message.substr(first_colon + 1, second_colon - first_colon - 1));
    std::size_t text_start = second_colon + 1;
    if (text_start < message.size() && message[text_start] == ' ') ++text_start;
    rest = message.substr(text_start);
    return true;
}

static bool contains(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

} // namespace

std::string classify_diagnostic_code(const std::string& message) {
    int line = 0;
    int column = 0;
    std::string diagnostic = message;
    (void)parse_location_prefix(message, line, column, diagnostic);

    // This classifier is intentionally conservative while Ari still reports
    // CompileError as text. New structured diagnostics should replace these
    // patterns with explicit codes at the throw site.
    if (contains(diagnostic, "unexpected character")) return "L0001";
    if (contains(diagnostic, "cannot find module file") ||
        contains(diagnostic, "module cache") ||
        contains(diagnostic, "module metadata")) {
        return "M0001";
    }
    if (contains(diagnostic, "borrow") ||
        contains(diagnostic, "moved binding") ||
        contains(diagnostic, "use after move") ||
        contains(diagnostic, "drop")) {
        return "O0001";
    }
    if (contains(diagnostic, "unknown type") ||
        contains(diagnostic, "unknown trait") ||
        contains(diagnostic, "trait bound") ||
        contains(diagnostic, "cannot infer") ||
        contains(diagnostic, "type mismatch") ||
        contains(diagnostic, "no matching") ||
        contains(diagnostic, "ambiguous")) {
        return "T0001";
    }
    if (contains(diagnostic, "expected") ||
        contains(diagnostic, "unexpected token") ||
        contains(diagnostic, "unterminated")) {
        return "P0001";
    }
    if (contains(diagnostic, "IR") || contains(diagnostic, "lowering")) return "I0001";
    if (contains(diagnostic, "LLVM backend") ||
        contains(diagnostic, "artifact") ||
        contains(diagnostic, "object")) {
        return "B0001";
    }
    return "ari/compiler";
}

std::string dump_diagnostic_message(const std::string& severity,
                                    const std::string& code,
                                    const std::string& message,
                                    const std::string& source_name) {
    int line = 0;
    int column = 0;
    std::string diagnostic = message;
    std::ostringstream out;
    out << "diagnostic " << severity << " code=" << code;
    if (parse_location_prefix(message, line, column, diagnostic)) {
        out << " message=" << quote(diagnostic)
            << " @ " << source_name << ":" << line << ":" << column << "\n";
    } else {
        out << " message=" << quote(diagnostic) << "\n";
    }
    return out.str();
}

} // namespace ari
