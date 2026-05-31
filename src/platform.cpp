#include "platform.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace ari::platform {

namespace {

char host_path_separator() {
#if defined(_WIN32)
    return '\\';
#else
    return '/';
#endif
}

char path_list_separator() {
#if defined(_WIN32)
    return ';';
#else
    return ':';
#endif
}

#if defined(_WIN32)
bool has_windows_extension(const std::string& path) {
    std::size_t last_separator = path.find_last_of("/\\");
    std::size_t dot = path.find_last_of('.');
    return dot != std::string::npos &&
           (last_separator == std::string::npos || dot > last_separator);
}

std::vector<std::string> windows_executable_extensions() {
    const char* env = std::getenv("PATHEXT");
    std::vector<std::string> extensions = split_path_environment(env && *env ? env : ".COM;.EXE;.BAT;.CMD");
    for (std::string& extension : extensions) {
        if (!extension.empty() && extension.front() != '.') extension.insert(extension.begin(), '.');
    }
    return extensions;
}
#endif

} // namespace

bool is_path_separator(char c) {
    return c == '/' || c == '\\';
}

bool is_absolute_path(const std::string& path) {
    if (path.empty()) return false;
#if defined(_WIN32)
    if (path.size() >= 2 && path[1] == ':') return true;
    return path.size() >= 2 && is_path_separator(path[0]) && is_path_separator(path[1]);
#else
    return path.front() == '/';
#endif
}

std::string path_join(const std::string& left, const std::string& right) {
    if (left.empty() || left == ".") return right;
    if (!right.empty() && is_absolute_path(right)) return right;
    if (is_path_separator(left.back())) return left + right;
    return left + host_path_separator() + right;
}

std::string dirname(const std::string& path) {
    std::size_t split = path.find_last_of("/\\");
    if (split == std::string::npos) return ".";
    if (split == 0) return path.substr(0, 1);
#if defined(_WIN32)
    if (split == 2 && path.size() >= 3 && path[1] == ':') return path.substr(0, 3);
#endif
    return path.substr(0, split);
}

std::vector<std::string> split_path_environment(const std::string& value) {
    std::vector<std::string> parts;
    std::stringstream stream(value);
    std::string part;
    while (std::getline(stream, part, path_list_separator())) {
        if (part.empty()) part = ".";
        parts.push_back(std::move(part));
    }
    return parts;
}

std::optional<std::string> current_executable_path() {
#if defined(_WIN32)
    std::vector<char> buffer(MAX_PATH);
    for (;;) {
        DWORD length = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) return std::nullopt;
        if (length < buffer.size()) return std::string(buffer.data(), length);
        buffer.resize(buffer.size() * 2);
    }
#elif defined(__linux__)
    std::vector<char> buffer(4096);
    for (;;) {
        ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
        if (length <= 0) return std::nullopt;
        if (static_cast<std::size_t>(length) < buffer.size() - 1) {
            return std::string(buffer.data(), static_cast<std::size_t>(length));
        }
        buffer.resize(buffer.size() * 2);
    }
#else
    return std::nullopt;
#endif
}

std::optional<std::string> current_working_directory() {
#if defined(_WIN32)
    DWORD needed = GetCurrentDirectoryA(0, nullptr);
    if (needed == 0) return std::nullopt;
    std::vector<char> buffer(needed);
    DWORD length = GetCurrentDirectoryA(static_cast<DWORD>(buffer.size()), buffer.data());
    if (length == 0 || length >= buffer.size()) return std::nullopt;
    return std::string(buffer.data(), length);
#else
    std::vector<char> buffer(4096);
    for (;;) {
        if (getcwd(buffer.data(), buffer.size()) != nullptr) return std::string(buffer.data());
        if (errno != ERANGE) return std::nullopt;
        buffer.resize(buffer.size() * 2);
    }
#endif
}

bool file_exists(const std::string& path) {
#if defined(_WIN32)
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat st;
    return stat(path.c_str(), &st) == 0;
#endif
}

bool regular_file_exists(const std::string& path) {
#if defined(_WIN32)
    DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
           (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
#endif
}

bool is_executable_file(const std::string& path) {
#if defined(_WIN32)
    return regular_file_exists(path);
#else
    return access(path.c_str(), X_OK) == 0;
#endif
}

std::optional<std::string> find_executable_on_path(const std::string& name) {
    const char* path_env = std::getenv("PATH");
    if (!path_env) return std::nullopt;

    std::vector<std::string> probes{name};
#if defined(_WIN32)
    if (!has_windows_extension(name)) {
        for (const std::string& extension : windows_executable_extensions()) {
            if (!extension.empty()) probes.push_back(name + extension);
        }
    }
#endif

    for (const auto& dir : split_path_environment(path_env)) {
        for (const auto& probe : probes) {
            std::string candidate = path_join(dir, probe);
            if (is_executable_file(candidate)) return candidate;
        }
    }
    return std::nullopt;
}

OperationResult set_executable_permission(const std::string& path) {
#if defined(_WIN32)
    (void)path;
    return OperationResult{true, ""};
#else
    if (chmod(path.c_str(), 0755) == 0) return OperationResult{true, ""};
    return OperationResult{false, std::strerror(errno)};
#endif
}

std::string shell_quote(const std::string& text) {
#if defined(_WIN32)
    std::string quoted = "\"";
    for (char c : text) {
        if (c == '"' || c == '\\') quoted.push_back('\\');
        quoted.push_back(c);
    }
    quoted += "\"";
    return quoted;
#else
    std::string quoted = "'";
    for (char c : text) {
        if (c == '\'') quoted += "'\\''";
        else quoted.push_back(c);
    }
    quoted += "'";
    return quoted;
#endif
}

int run_shell_command(const std::string& command) {
    return std::system(command.c_str());
}

CommandResult run_shell_command_capture(const std::string& command) {
    CommandResult result;
    std::vector<char> buffer(4096);
#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (pipe == nullptr) return result;
    result.launched = true;
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result.output += buffer.data();
    }
#if defined(_WIN32)
    result.status = _pclose(pipe);
#else
    result.status = pclose(pipe);
#endif
    return result;
}

} // namespace ari::platform
