#pragma once

#include "../ari_tooling/diagnostic.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace ari::lsp {

class DocumentStore {
public:
    void set(const std::string& uri, std::string text);
    void erase(const std::string& uri);
    std::optional<std::string> text(const std::string& uri) const;

private:
    std::map<std::string, std::string> documents_;
};

class TemporaryDocumentFile {
public:
    TemporaryDocumentFile(const std::string& original_path, const std::string& text);
    ~TemporaryDocumentFile();

    TemporaryDocumentFile(const TemporaryDocumentFile&) = delete;
    TemporaryDocumentFile& operator=(const TemporaryDocumentFile&) = delete;

    const std::string& path() const { return path_; }

private:
    std::string path_;
};

std::string uri_to_path(const std::string& uri);
std::string path_to_uri(const std::string& path);
std::string diagnostics_json_array(const std::vector<tooling::Diagnostic>& diagnostics);
std::string diagnostics_notification(const std::string& uri, const std::vector<tooling::Diagnostic>& diagnostics);
std::string diagnostics_report_response(const std::string& id, const std::vector<tooling::Diagnostic>& diagnostics);

} // namespace ari::lsp
