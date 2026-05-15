#include "documents.hpp"

#include "../ari_tooling/diagnostic.hpp"

#include <cctype>
#include <sstream>

namespace ari::lsp {
namespace {

int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

std::string percent_decode(const std::string& text) {
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '%' && i + 2 < text.size()) {
            int hi = hex_value(text[i + 1]);
            int lo = hex_value(text[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        out.push_back(text[i]);
    }
    return out;
}

} // namespace

std::string uri_to_path(const std::string& uri) {
    const std::string prefix = "file://";
    if (uri.rfind(prefix, 0) != 0) return uri;
    return percent_decode(uri.substr(prefix.size()));
}

std::string path_to_uri(const std::string& path) {
    return "file://" + path;
}

std::string diagnostics_json_array(const std::vector<tooling::Diagnostic>& diagnostics) {
    std::ostringstream out;
    out << "[";
    bool first = true;
    for (const tooling::Diagnostic& diagnostic : diagnostics) {
        if (!first) out << ",";
        first = false;
        int line = diagnostic.line > 0 ? diagnostic.line - 1 : 0;
        int column = diagnostic.column > 0 ? diagnostic.column - 1 : 0;
        out << "{";
        out << "\"range\":{";
        out << "\"start\":{\"line\":" << line << ",\"character\":" << column << "},";
        out << "\"end\":{\"line\":" << line << ",\"character\":" << (column + 1) << "}";
        out << "},";
        out << "\"severity\":" << tooling::lsp_severity(diagnostic.severity) << ",";
        out << "\"source\":\"" << tooling::json_escape(diagnostic.source) << "\",";
        out << "\"message\":\"" << tooling::json_escape(diagnostic.message) << "\"";
        out << "}";
    }
    out << "]";
    return out.str();
}

std::string diagnostics_notification(const std::string& uri, const std::vector<tooling::Diagnostic>& diagnostics) {
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"method\":\"textDocument/publishDiagnostics\",";
    out << "\"params\":{";
    out << "\"uri\":\"" << tooling::json_escape(uri) << "\",";
    out << "\"diagnostics\":" << diagnostics_json_array(diagnostics);
    out << "}}";
    return out.str();
}

std::string diagnostics_report_response(const std::string& id, const std::vector<tooling::Diagnostic>& diagnostics) {
    std::ostringstream out;
    out << "{";
    out << "\"jsonrpc\":\"2.0\",";
    out << "\"id\":" << (id.empty() ? "null" : id) << ",";
    out << "\"result\":{";
    out << "\"kind\":\"full\",";
    out << "\"items\":" << diagnostics_json_array(diagnostics);
    out << "}}";
    return out.str();
}

} // namespace ari::lsp
