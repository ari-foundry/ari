#include "checker.hpp"

#include "../ari_tooling/diagnostic.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

void usage() {
    std::cerr << "usage: ari-lint [--ari PATH] [--json] [-I DIR] FILE...\n";
}

std::string default_ari_path() {
    if (const char* env = std::getenv("ARI_COMPILER")) return env;
    return "build/ari";
}

void print_human(const ari::lint::LintResult& result) {
    if (result.diagnostics.empty() && result.exit_code == 0) {
        std::cout << result.file << ": ok\n";
        return;
    }
    if (result.diagnostics.empty()) {
        std::cout << result.file << ":1:1: error: compiler check failed";
        if (!result.raw_output.empty()) std::cout << ": " << result.raw_output;
        if (result.raw_output.empty() || result.raw_output.back() != '\n') std::cout << "\n";
        return;
    }
    for (const ari::tooling::Diagnostic& diagnostic : result.diagnostics) {
        std::cout << (diagnostic.file.empty() ? result.file : diagnostic.file)
                  << ":" << diagnostic.line
                  << ":" << diagnostic.column
                  << ": " << ari::tooling::severity_name(diagnostic.severity)
                  << ": " << diagnostic.message << "\n";
    }
}

void print_json(const std::vector<ari::lint::LintResult>& results) {
    std::cout << "{";
    std::cout << "\"files\":[";
    bool first_file = true;
    for (const ari::lint::LintResult& result : results) {
        if (!first_file) std::cout << ",";
        first_file = false;
        std::cout << "{";
        std::cout << "\"path\":\"" << ari::tooling::json_escape(result.file) << "\",";
        std::cout << "\"exitCode\":" << result.exit_code << ",";
        std::cout << "\"diagnostics\":[";
        bool first_diag = true;
        for (const ari::tooling::Diagnostic& diagnostic : result.diagnostics) {
            if (!first_diag) std::cout << ",";
            first_diag = false;
            std::cout << "{";
            std::cout << "\"file\":\"" << ari::tooling::json_escape(diagnostic.file) << "\",";
            std::cout << "\"line\":" << diagnostic.line << ",";
            std::cout << "\"column\":" << diagnostic.column << ",";
            std::cout << "\"severity\":\"" << ari::tooling::json_escape(ari::tooling::severity_name(diagnostic.severity)) << "\",";
            std::cout << "\"message\":\"" << ari::tooling::json_escape(diagnostic.message) << "\",";
            std::cout << "\"source\":\"" << ari::tooling::json_escape(diagnostic.source) << "\"";
            std::cout << "}";
        }
        std::cout << "]";
        std::cout << "}";
    }
    std::cout << "]}\n";
}

} // namespace

int main(int argc, char** argv) {
    ari::lint::LintConfig config;
    config.ari_path = default_ari_path();
    bool json = false;
    std::vector<std::string> files;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            usage();
            return 0;
        }
        if (arg == "--json") {
            json = true;
            continue;
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
        if (!arg.empty() && arg[0] == '-') {
            usage();
            return 2;
        }
        files.push_back(arg);
    }

    if (files.empty()) {
        usage();
        return 2;
    }

    std::vector<ari::lint::LintResult> results;
    int exit_code = 0;
    try {
        for (const std::string& file : files) {
            ari::lint::LintResult result = ari::lint::run_lint(config, file);
            if (result.exit_code != 0 || !result.diagnostics.empty()) exit_code = 1;
            if (!json) print_human(result);
            results.push_back(std::move(result));
        }
    } catch (const std::exception& ex) {
        std::cerr << "ari-lint: error: " << ex.what() << "\n";
        return 1;
    }

    if (json) print_json(results);
    return exit_code;
}
