#include "documents.hpp"
#include "json_rpc.hpp"
#include "symbols.hpp"

#include "../ari_tooling/diagnostic.hpp"
#include "../lint/checker.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Config {
    ari::lint::LintConfig lint;
};

std::string default_ari_path() {
    if (const char* env = std::getenv("ARI_COMPILER")) return env;
    return "build/ari";
}

void usage() {
    std::cerr << "usage: ari-lsp [--ari PATH] [--rule RULE=SEVERITY] [-I DIR]\n";
}

std::vector<ari::tooling::Diagnostic> check_file(const Config& config, const std::string& path) {
    return ari::lint::run_lint(config.lint, path).diagnostics;
}

void remap_diagnostic_file(std::vector<ari::tooling::Diagnostic>& diagnostics,
                           const std::string& from,
                           const std::string& to) {
    for (ari::tooling::Diagnostic& diagnostic : diagnostics) {
        if (diagnostic.file.empty() || diagnostic.file == from) {
            diagnostic.file = to;
        }
    }
}

std::vector<ari::tooling::Diagnostic> check_document(const Config& config,
                                                     const ari::lsp::DocumentStore& documents,
                                                     const std::string& uri) {
    if (uri.empty()) {
        return {ari::tooling::Diagnostic{
            "",
            1,
            1,
            ari::tooling::DiagnosticSeverity::Error,
            "missing textDocument.uri",
            "ari-lsp",
            "",
            0,
            0,
        }};
    }
    std::string path = ari::lsp::uri_to_path(uri);
    try {
        if (std::optional<std::string> text = documents.text(uri)) {
            ari::lsp::TemporaryDocumentFile temp(path, *text);
            std::vector<ari::tooling::Diagnostic> diagnostics = check_file(config, temp.path());
            remap_diagnostic_file(diagnostics, temp.path(), path);
            return diagnostics;
        }
        return check_file(config, path);
    } catch (const std::exception& ex) {
        return {ari::tooling::Diagnostic{
            path,
            1,
            1,
            ari::tooling::DiagnosticSeverity::Error,
            ex.what(),
            "ari-lsp",
            "",
            0,
            0,
        }};
    }
}

void update_document_text(ari::lsp::DocumentStore& documents, const std::string& body, const std::string& uri) {
    if (uri.empty()) return;
    std::string raw_text = ari::lsp::json_raw_field(body, "text");
    if (!raw_text.empty() && raw_text.front() == '"') {
        documents.set(uri, ari::lsp::json_string_field(body, "text"));
    }
}

void publish_for_document(const Config& config, const ari::lsp::DocumentStore& documents, const std::string& uri) {
    if (uri.empty()) return;
    ari::lsp::write_message(std::cout, ari::lsp::diagnostics_notification(uri, check_document(config, documents, uri)));
}

std::string read_file_text(const std::string& path) {
    std::ifstream in(path);
    if (!in) return "";
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}

std::string text_for_uri(const ari::lsp::DocumentStore& documents, const std::string& uri) {
    if (std::optional<std::string> text = documents.text(uri)) return *text;
    return read_file_text(ari::lsp::uri_to_path(uri));
}

std::string response_result(const std::string& id, const std::string& result) {
    return "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"result\":" + result + "}";
}

std::string response_error(const std::string& id, int code, const std::string& message) {
    return "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"error\":{\"code\":" + std::to_string(code) +
           ",\"message\":\"" + ari::tooling::json_escape(message) + "\"}}";
}

int int_field_or_zero(const std::string& body, const std::string& field) {
    std::string raw = ari::lsp::json_raw_field(body, field);
    if (raw.empty()) return 0;
    return std::stoi(raw);
}

bool apply_rule_setting(Config& config, const std::string& setting_text) {
    std::size_t equals = setting_text.find('=');
    std::string code = equals == std::string::npos ? setting_text : setting_text.substr(0, equals);
    std::optional<ari::lint::RuleSetting> setting = ari::lint::parse_rule_setting(setting_text);
    if (!setting) {
        std::cerr << "ari-lsp: error: invalid rule setting '" << setting_text << "'\n";
        return false;
    }
    config.lint.rule_settings[ari::lint::normalize_rule_code(code)] = *setting;
    return true;
}

} // namespace

int main(int argc, char** argv) {
    Config config;
    config.lint.ari_path = default_ari_path();
    ari::lsp::DocumentStore documents;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            usage();
            return 0;
        }
        if (arg == "--ari") {
            if (i + 1 >= argc) {
                usage();
                return 2;
            }
            config.lint.ari_path = argv[++i];
            continue;
        }
        if (arg == "--rule") {
            if (i + 1 >= argc) {
                usage();
                return 2;
            }
            if (!apply_rule_setting(config, argv[++i])) return 2;
            continue;
        }
        if (arg.rfind("--rule=", 0) == 0) {
            if (!apply_rule_setting(config, arg.substr(7))) return 2;
            continue;
        }
        if (arg == "-I") {
            if (i + 1 >= argc) {
                usage();
                return 2;
            }
            config.lint.module_paths.push_back(argv[++i]);
            continue;
        }
        if (arg.rfind("-I", 0) == 0 && arg.size() > 2) {
            config.lint.module_paths.push_back(arg.substr(2));
            continue;
        }
        usage();
        return 2;
    }

    bool shutdown = false;
    while (auto message = ari::lsp::read_message(std::cin)) {
        const std::string& body = message->body;
        std::string method = ari::lsp::json_string_field(body, "method");
        std::string id = ari::lsp::json_raw_field(body, "id");

        if (method == "initialize") {
            std::string capabilities =
                "{\"capabilities\":{"
                "\"textDocumentSync\":1,"
                "\"diagnosticProvider\":{\"interFileDependencies\":true,\"workspaceDiagnostics\":false},"
                "\"documentSymbolProvider\":true,"
                "\"hoverProvider\":true,"
                "\"definitionProvider\":true,"
                "\"completionProvider\":{\"resolveProvider\":false,\"triggerCharacters\":[\".\",\":\"]}"
                "},\"serverInfo\":{\"name\":\"ari-lsp\",\"version\":\"0.1.0-dev\"}}";
            ari::lsp::write_message(std::cout, response_result(id.empty() ? "null" : id, capabilities));
            continue;
        }
        if (method == "shutdown") {
            shutdown = true;
            ari::lsp::write_message(std::cout, response_result(id.empty() ? "null" : id, "null"));
            continue;
        }
        if (method == "exit") {
            return shutdown ? 0 : 1;
        }
        if (method == "textDocument/didOpen" ||
            method == "textDocument/didSave" ||
            method == "textDocument/didChange") {
            std::string uri = ari::lsp::json_string_field(body, "uri");
            update_document_text(documents, body, uri);
            publish_for_document(config, documents, uri);
            continue;
        }
        if (method == "textDocument/diagnostic") {
            std::vector<ari::tooling::Diagnostic> diagnostics =
                check_document(config, documents, ari::lsp::json_string_field(body, "uri"));
            ari::lsp::write_message(std::cout, ari::lsp::diagnostics_report_response(id, diagnostics));
            continue;
        }
        if (method == "textDocument/documentSymbol") {
            std::string uri = ari::lsp::json_string_field(body, "uri");
            ari::lsp::write_message(std::cout, ari::lsp::document_symbols_response(id, text_for_uri(documents, uri)));
            continue;
        }
        if (method == "textDocument/hover") {
            std::string uri = ari::lsp::json_string_field(body, "uri");
            ari::lsp::write_message(
                std::cout,
                ari::lsp::hover_response(
                    id,
                    text_for_uri(documents, uri),
                    int_field_or_zero(body, "line"),
                    int_field_or_zero(body, "character")));
            continue;
        }
        if (method == "textDocument/definition") {
            std::string uri = ari::lsp::json_string_field(body, "uri");
            ari::lsp::write_message(
                std::cout,
                ari::lsp::definition_response(
                    id,
                    text_for_uri(documents, uri),
                    uri,
                    int_field_or_zero(body, "line"),
                    int_field_or_zero(body, "character")));
            continue;
        }
        if (method == "textDocument/completion") {
            std::string uri = ari::lsp::json_string_field(body, "uri");
            ari::lsp::write_message(
                std::cout,
                ari::lsp::completion_response(
                    id,
                    text_for_uri(documents, uri),
                    int_field_or_zero(body, "line"),
                    int_field_or_zero(body, "character")));
            continue;
        }
        if (method == "textDocument/didClose") {
            std::string uri = ari::lsp::json_string_field(body, "uri");
            if (!uri.empty()) {
                documents.erase(uri);
                ari::lsp::write_message(std::cout, ari::lsp::diagnostics_notification(uri, {}));
            }
            continue;
        }

        if (!id.empty()) {
            ari::lsp::write_message(std::cout, response_error(id, -32601, "method not found"));
        }
    }

    return 0;
}
