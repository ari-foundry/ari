#include "documents.hpp"
#include "json_rpc.hpp"

#include "../ari_tooling/process.hpp"
#include "../ari_tooling/diagnostic.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Config {
    std::string ari_path = "build/ari";
    std::vector<std::string> module_paths;
};

std::string default_ari_path() {
    if (const char* env = std::getenv("ARI_COMPILER")) return env;
    return "build/ari";
}

void usage() {
    std::cerr << "usage: ari-lsp [--ari PATH] [-I DIR]\n";
}

std::vector<ari::tooling::Diagnostic> check_file(const Config& config, const std::string& path) {
    std::vector<std::string> args;
    args.push_back(config.ari_path);
    for (const std::string& module_path : config.module_paths) {
        args.push_back("-I");
        args.push_back(module_path);
    }
    args.push_back(path);
    args.push_back("--check");
    ari::tooling::ProcessResult result = ari::tooling::run_process(args);
    if (result.exit_code == 0) return {};
    std::vector<ari::tooling::Diagnostic> diagnostics = ari::tooling::parse_ari_diagnostics(result.output, path);
    if (diagnostics.empty()) {
        diagnostics.push_back(ari::tooling::Diagnostic{
            path,
            1,
            1,
            ari::tooling::DiagnosticSeverity::Error,
            result.output.empty() ? "compiler check failed" : result.output,
            "ari",
        });
    }
    return diagnostics;
}

void publish_for_uri(const Config& config, const std::string& uri) {
    if (uri.empty()) return;
    std::string path = ari::lsp::uri_to_path(uri);
    std::vector<ari::tooling::Diagnostic> diagnostics;
    try {
        diagnostics = check_file(config, path);
    } catch (const std::exception& ex) {
        diagnostics.push_back(ari::tooling::Diagnostic{
            path,
            1,
            1,
            ari::tooling::DiagnosticSeverity::Error,
            ex.what(),
            "ari-lsp",
        });
    }
    ari::lsp::write_message(std::cout, ari::lsp::diagnostics_notification(uri, diagnostics));
}

std::vector<ari::tooling::Diagnostic> diagnostics_for_uri(const Config& config, const std::string& uri) {
    if (uri.empty()) {
        return {ari::tooling::Diagnostic{
            "",
            1,
            1,
            ari::tooling::DiagnosticSeverity::Error,
            "missing textDocument.uri",
            "ari-lsp",
        }};
    }
    std::string path = ari::lsp::uri_to_path(uri);
    try {
        return check_file(config, path);
    } catch (const std::exception& ex) {
        return {ari::tooling::Diagnostic{
            path,
            1,
            1,
            ari::tooling::DiagnosticSeverity::Error,
            ex.what(),
            "ari-lsp",
        }};
    }
}

std::string response_result(const std::string& id, const std::string& result) {
    return "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"result\":" + result + "}";
}

std::string response_error(const std::string& id, int code, const std::string& message) {
    return "{\"jsonrpc\":\"2.0\",\"id\":" + id + ",\"error\":{\"code\":" + std::to_string(code) +
           ",\"message\":\"" + ari::tooling::json_escape(message) + "\"}}";
}

} // namespace

int main(int argc, char** argv) {
    Config config;
    config.ari_path = default_ari_path();

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
            config.ari_path = argv[++i];
            continue;
        }
        if (arg == "-I") {
            if (i + 1 >= argc) {
                usage();
                return 2;
            }
            config.module_paths.push_back(argv[++i]);
            continue;
        }
        if (arg.rfind("-I", 0) == 0 && arg.size() > 2) {
            config.module_paths.push_back(arg.substr(2));
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
                "\"diagnosticProvider\":{\"interFileDependencies\":true,\"workspaceDiagnostics\":false}"
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
            publish_for_uri(config, ari::lsp::json_string_field(body, "uri"));
            continue;
        }
        if (method == "textDocument/diagnostic") {
            std::vector<ari::tooling::Diagnostic> diagnostics =
                diagnostics_for_uri(config, ari::lsp::json_string_field(body, "uri"));
            ari::lsp::write_message(std::cout, ari::lsp::diagnostics_report_response(id, diagnostics));
            continue;
        }
        if (method == "textDocument/didClose") {
            std::string uri = ari::lsp::json_string_field(body, "uri");
            if (!uri.empty()) {
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
