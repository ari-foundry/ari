#include "checker.hpp"
#include "config.hpp"

#include "../ari_tooling/process.hpp"

#include <optional>
#include <utility>

namespace ari::lint {
namespace {

RuleSettings effective_rule_settings(const LintConfig& config,
                                     const std::string& file,
                                     std::vector<tooling::Diagnostic>& diagnostics) {
    RuleSettings settings;
    if (config.discover_rule_config) {
        if (std::optional<std::string> path = find_rule_config_for_file(file)) {
            RuleConfigLoadResult loaded = load_rule_config(*path);
            settings = std::move(loaded.settings);
            for (const std::string& error : loaded.errors) {
                diagnostics.push_back(tooling::Diagnostic{
                    file,
                    1,
                    1,
                    tooling::DiagnosticSeverity::Error,
                    error,
                    "ari-lint",
                    "lint/config",
                    0,
                    0,
                });
            }
        }
    }
    for (const auto& entry : config.rule_settings) {
        settings[entry.first] = entry.second;
    }
    return settings;
}

} // namespace

LintResult run_lint(const LintConfig& config, const std::string& file) {
    std::vector<std::string> args;
    args.push_back(config.ari_path);
    for (const std::string& module_path : config.module_paths) {
        args.push_back("-I");
        args.push_back(module_path);
    }
    args.push_back(file);
    args.push_back("--check");

    tooling::ProcessResult process = tooling::run_process(args);
    std::vector<tooling::Diagnostic> diagnostics = tooling::parse_ari_diagnostics(process.output, file);
    RuleSettings settings = effective_rule_settings(config, file, diagnostics);
    std::vector<tooling::Diagnostic> native_diagnostics = run_native_rules(settings, file);
    diagnostics.insert(diagnostics.end(), native_diagnostics.begin(), native_diagnostics.end());
    if (process.exit_code != 0 && diagnostics.empty()) {
        diagnostics.push_back(tooling::Diagnostic{
            file,
            1,
            1,
            tooling::DiagnosticSeverity::Error,
            process.output.empty() ? "compiler check failed" : process.output,
            "ari",
            "ari/compiler-check-failed",
            0,
            0,
        });
    }
    return LintResult{
        file,
        process.exit_code,
        process.output,
        diagnostics,
    };
}

} // namespace ari::lint
