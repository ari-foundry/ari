#include "toolchain.hpp"

#include "platform.hpp"

#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

namespace ari {

std::string default_llvm_compiler() {
    if (const char* env = std::getenv("ARI_LLVM_CC")) {
        if (*env) return env;
    }

    const std::vector<std::string> names = {
        "clang", "clang-21", "clang-20", "clang-19", "clang-18", "clang-17", "clang-16", "clang-15", "clang-14"
    };
    for (const auto& name : names) {
        std::optional<std::string> found = platform::find_executable_on_path(name);
        if (found) return *found;
    }

    const std::vector<std::string> absolute_paths = {
        "/usr/bin/clang",
        "/usr/bin/clang-21",
        "/usr/bin/clang-20",
        "/usr/bin/clang-19",
        "/usr/bin/clang-18",
        "/usr/bin/clang-17",
        "/usr/bin/clang-16",
        "/usr/bin/clang-15",
        "/usr/bin/clang-14",
        "/usr/lib/llvm-21/bin/clang",
        "/usr/lib/llvm-20/bin/clang",
        "/usr/lib/llvm-19/bin/clang",
        "/usr/lib/llvm-18/bin/clang",
        "/usr/lib/llvm-17/bin/clang",
        "/usr/lib/llvm-16/bin/clang",
        "/usr/lib/llvm-15/bin/clang",
        "/usr/lib/llvm-14/bin/clang"
    };
    for (const auto& path : absolute_paths) {
        if (platform::is_executable_file(path)) return path;
    }

    return "clang";
}

} // namespace ari
