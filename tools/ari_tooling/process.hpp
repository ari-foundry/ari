#pragma once

#include <string>
#include <vector>

namespace ari::tooling {

struct ProcessResult {
    int exit_code = 1;
    std::string output;
};

ProcessResult run_process(const std::vector<std::string>& args);

} // namespace ari::tooling
