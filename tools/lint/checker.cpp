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
    return LintResult{
        file,
        process.exit_code,
        process.output,
        tooling::parse_ari_diagnostics(process.output, file),
    };
}

} // namespace ari::lint
