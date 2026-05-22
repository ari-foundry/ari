#include "diagnostic_dump.hpp"

#include <cctype>
#include <sstream>
#include <string>

namespace ari {

namespace {

struct DiagnosticCatalogEntry {
    const char* code;
    const char* family;
    const char* owner;
    const char* proves;
};

static const DiagnosticCatalogEntry kDiagnosticCatalogEntries[] = {
    {"L0001", "lexer", "src/lexer.cpp", "characters, escapes, and invalid tokens"},
    {"P0001", "parser", "src/parser.cpp", "grammar, delimiters, and recovery"},
    {"M0001", "module", "src/module_loader.cpp", "imports, visibility, metadata, and caches"},
    {"T0001", "type", "src/type_semantics.cpp", "types, traits, methods, and generic constraints"},
    {"O0001", "ownership", "src/ownership_semantics.cpp", "ownership, borrowing, moves, drops, and zones"},
    {"I0001", "ir", "src/ir.hpp", "typed IR lowering and resolved compiler facts"},
    {"B0001", "backend", "src/llvm_codegen.cpp", "LLVM, object, executable, shared library, and artifact emission"},
    {"ari/compiler", "general", "src/driver.cpp", "unclassified transitional CompileError text"},
};

static const DiagnosticCatalogEntry* find_diagnostic_entry(const std::string& code) {
    for (const DiagnosticCatalogEntry& entry : kDiagnosticCatalogEntries) {
        if (entry.code == code) return &entry;
    }
    return nullptr;
}

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

std::string diagnostic_code_family(const std::string& code) {
    if (code == "L0001") return "lexer";
    if (code == "P0001") return "parser";
    if (code == "M0001") return "module";
    if (code == "T0001") return "type";
    if (code == "O0001") return "ownership";
    if (code == "I0001") return "ir";
    if (code == "B0001") return "backend";
    return "general";
}

std::string dump_diagnostic_catalog() {
    std::ostringstream out;
    out << "DiagnosticCatalog version=1 entries="
        << (sizeof(kDiagnosticCatalogEntries) / sizeof(kDiagnosticCatalogEntries[0])) << "\n";
    for (const DiagnosticCatalogEntry& entry : kDiagnosticCatalogEntries) {
        out << "  code=" << entry.code
            << " family=" << entry.family
            << " owner=" << entry.owner
            << " proves=" << quote(entry.proves) << "\n";
    }
    return out.str();
}

std::string dump_diagnostic_explanation(const std::string& code) {
    std::ostringstream out;
    const DiagnosticCatalogEntry* entry = find_diagnostic_entry(code);
    if (entry != nullptr) {
        out << "DiagnosticExplanation version=1 code=" << entry->code
            << " status=known"
            << " family=" << entry->family
            << " owner=" << entry->owner
            << " first_check=\"make check-compiler-artifacts\""
            << " artifact=\"--emit-diagnostics\""
            << " proves=" << quote(entry->proves) << "\n";
        return out.str();
    }

    out << "DiagnosticExplanation version=1 code=" << code
        << " status=unknown"
        << " family=" << diagnostic_code_family(code)
        << " owner=<none>"
        << " first_check=\"make check-compiler-artifacts\""
        << " artifact=\"--emit-diagnostic-catalog\""
        << " proves=\"code is not in the current diagnostic catalog\"\n";
    return out.str();
}

std::string dump_diagnostic_message(const std::string& severity,
                                    const std::string& code,
                                    const CompileError& error,
                                    const std::string& source_name) {
    std::ostringstream out;
    out << "diagnostic " << severity
        << " code=" << code
        << " family=" << diagnostic_code_family(code);
    if (error.has_location()) {
        SourceLocation loc = error.location();
        if (!valid_source_id(loc.source_id) && has_source_name(loc)) {
            loc.source_id = source_id_for_name(loc.source_name);
        }
        if (!valid_source_id(loc.source_id) && !source_name.empty()) {
            loc.source_id = source_id_for_name(source_name);
        }
        if (loc.source_name.empty()) {
            if (const SourceFile* file = find_source_file(loc.source_id)) {
                loc.source_name = file->display_name;
            } else {
                loc.source_name = source_name;
            }
        }
        if (!valid_source_id(loc.source_id)) {
            out << " message=" << quote(error.message()) << "\n";
            return out.str();
        }
        out << " source_id=" << source_id_text(loc.source_id)
            << " source=" << quote(loc.source_name)
            << " line=" << loc.line
            << " column=" << loc.column;
        if (has_byte_span(loc)) {
            Span span = span_from_location(loc);
            out << " byte_start=" << span.start
                << " byte_end=" << span.end;
        }
        out << " message=" << quote(error.message());
        std::string snippet = render_source_snippet(loc);
        if (!snippet.empty()) out << " snippet=" << quote(snippet);
        out << "\n";
    } else {
        out << " message=" << quote(error.message()) << "\n";
    }
    return out.str();
}

} // namespace ari
