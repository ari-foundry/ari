#include "toolchain.hpp"

#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace ari {

namespace {

bool is_executable_file(const std::string& path) {
#ifdef _WIN32
    (void)path;
    return false;
#else
    return access(path.c_str(), X_OK) == 0;
#endif
}

std::string find_on_path(const std::string& name) {
    const char* path_env = std::getenv("PATH");
    if (!path_env) return "";
    std::stringstream stream(path_env);
    std::string dir;
    while (std::getline(stream, dir, ':')) {
        if (dir.empty()) dir = ".";
        std::string candidate = dir + "/" + name;
        if (is_executable_file(candidate)) return candidate;
    }
    return "";
}

} // namespace

std::string default_llvm_compiler() {
    if (const char* env = std::getenv("ARI_LLVM_CC")) {
        if (*env) return env;
    }

    const std::vector<std::string> names = {
        "clang", "clang-21", "clang-20", "clang-19", "clang-18", "clang-17", "clang-16", "clang-15", "clang-14"
    };
    for (const auto& name : names) {
        std::string found = find_on_path(name);
        if (!found.empty()) return found;
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
        if (is_executable_file(path)) return path;
    }

    return "clang";
}

} // namespace ari
