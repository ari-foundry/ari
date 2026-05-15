#include "documents.hpp"

#include "../ari_tooling/diagnostic.hpp"

#include <cctype>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <utility>
#include <unistd.h>

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

std::string dirname_of(const std::string& path) {
    std::size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) return ".";
    if (slash == 0) return "/";
    return path.substr(0, slash);
}

std::string basename_of(const std::string& path) {
    std::size_t slash = path.find_last_of('/');
    if (slash == std::string::npos) return path;
    return path.substr(slash + 1);
}

std::string extension_of(const std::string& path) {
    std::string base = basename_of(path);
    std::size_t dot = base.find_last_of('.');
    if (dot == std::string::npos) return ".ari";
    return base.substr(dot);
}

void write_all(int fd, const std::string& text) {
    const char* data = text.data();
    std::size_t remaining = text.size();
    while (remaining > 0) {
        ssize_t count = write(fd, data, remaining);
        if (count < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string("write failed: ") + std::strerror(errno));
        }
        data += count;
        remaining -= static_cast<std::size_t>(count);
    }
}

} // namespace

void DocumentStore::set(const std::string& uri, std::string text) {
    documents_[uri] = std::move(text);
}

void DocumentStore::erase(const std::string& uri) {
    documents_.erase(uri);
}

std::optional<std::string> DocumentStore::text(const std::string& uri) const {
    auto it = documents_.find(uri);
    if (it == documents_.end()) return std::nullopt;
    return it->second;
}

TemporaryDocumentFile::TemporaryDocumentFile(const std::string& original_path, const std::string& text) {
    std::string suffix = extension_of(original_path);
    std::string name = basename_of(original_path);
    std::string templ = dirname_of(original_path) + "/.ari-lsp-" + name + "-XXXXXX" + suffix;
    int fd = mkstemps(templ.data(), static_cast<int>(suffix.size()));
    if (fd < 0) {
        throw std::runtime_error(std::string("mkstemps failed: ") + std::strerror(errno));
    }
    try {
        write_all(fd, text);
    } catch (...) {
        close(fd);
        unlink(templ.c_str());
        throw;
    }
    if (close(fd) != 0) {
        unlink(templ.c_str());
        throw std::runtime_error(std::string("close failed: ") + std::strerror(errno));
    }
    path_ = templ;
}

TemporaryDocumentFile::~TemporaryDocumentFile() {
    if (!path_.empty()) unlink(path_.c_str());
}

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
        int end_line = diagnostic.end_line > 0 ? diagnostic.end_line - 1 : line;
        int end_column = diagnostic.end_column > 0 ? diagnostic.end_column - 1 : column + 1;
        out << "{";
        out << "\"range\":{";
        out << "\"start\":{\"line\":" << line << ",\"character\":" << column << "},";
        out << "\"end\":{\"line\":" << end_line << ",\"character\":" << end_column << "}";
        out << "},";
        out << "\"severity\":" << tooling::lsp_severity(diagnostic.severity) << ",";
        out << "\"source\":\"" << tooling::json_escape(diagnostic.source) << "\",";
        if (!diagnostic.code.empty()) {
            out << "\"code\":\"" << tooling::json_escape(diagnostic.code) << "\",";
        }
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
