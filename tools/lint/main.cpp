#include "checker.hpp"

#include "../ari_tooling/diagnostic.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

void usage() {
    std::cerr << "usage: ari-lint [--ari PATH] [--json] [--list-rules] [--rule RULE=SEVERITY] [-I DIR] FILE...\n";
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
                  << ": ";
        if (!diagnostic.code.empty()) {
            std::cout << "[" << diagnostic.code << "] ";
        }
        std::cout << diagnostic.message << "\n";
    }
}

void print_rules() {
    for (const ari::lint::RuleDescriptor& rule : ari::lint::registered_rules()) {
        std::cout << rule.code
                  << "\tdefault=" << ari::tooling::severity_name(rule.default_severity)
                  << "\t" << rule.description << "\n";
    }
}

bool apply_rule_setting(ari::lint::LintConfig& config, const std::string& setting_text) {
    std::size_t equals = setting_text.find('=');
    std::string code = equals == std::string::npos ? setting_text : setting_text.substr(0, equals);
    std::optional<ari::lint::RuleSetting> setting = ari::lint::parse_rule_setting(setting_text);
    if (!setting) {
        std::cerr << "ari-lint: error: invalid rule setting '" << setting_text << "'\n";
        return false;
    }
    config.rule_settings[ari::lint::normalize_rule_code(code)] = *setting;
    return true;
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
            int end_line = diagnostic.end_line > 0 ? diagnostic.end_line : diagnostic.line;
            int end_column = diagnostic.end_column > 0 ? diagnostic.end_column : diagnostic.column + 1;
            std::cout << "\"endLine\":" << end_line << ",";
            std::cout << "\"endColumn\":" << end_column << ",";
            std::cout << "\"severity\":\"" << ari::tooling::json_escape(ari::tooling::severity_name(diagnostic.severity)) << "\",";
            std::cout << "\"message\":\"" << ari::tooling::json_escape(diagnostic.message) << "\",";
            std::cout << "\"source\":\"" << ari::tooling::json_escape(diagnostic.source) << "\"";
            if (!diagnostic.code.empty()) {
                std::cout << ",\"code\":\"" << ari::tooling::json_escape(diagnostic.code) << "\"";
            }
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
    bool list_rules = false;
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
        if (arg == "--list-rules") {
            list_rules = true;
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

    if (list_rules) {
        print_rules();
        return 0;
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
