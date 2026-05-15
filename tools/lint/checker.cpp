#include "checker.hpp"

#include "../ari_tooling/process.hpp"

namespace ari::lint {

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
    std::vector<tooling::Diagnostic> native_diagnostics = run_native_rules(config.rule_settings, file);
    diagnostics.insert(diagnostics.end(), native_diagnostics.begin(), native_diagnostics.end());
    if (process.exit_code != 0 && diagnostics.empty()) {
        diagnostics.push_back(tooling::Diagnostic{
            file,
            1,
            1,
            tooling::DiagnosticSeverity::Error,
            process.output.empty() ? "compiler check failed" : process.output,
            "ari",
            "",
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
