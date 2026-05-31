#pragma once

#include <optional>
#include <string>
#include <vector>

namespace ari::platform {

struct OperationResult {
    bool ok = false;
    std::string error;
};

struct CommandResult {
    bool launched = false;
    int status = -1;
    std::string output;
};

bool is_path_separator(char c);
bool is_absolute_path(const std::string& path);
std::string path_join(const std::string& left, const std::string& right);
std::string dirname(const std::string& path);

std::vector<std::string> split_path_environment(const std::string& value);
std::optional<std::string> current_executable_path();
std::optional<std::string> current_working_directory();

bool file_exists(const std::string& path);
bool regular_file_exists(const std::string& path);
bool is_executable_file(const std::string& path);
std::optional<std::string> find_executable_on_path(const std::string& name);

OperationResult set_executable_permission(const std::string& path);

std::string shell_quote(const std::string& text);
int run_shell_command(const std::string& command);
CommandResult run_shell_command_capture(const std::string& command);

} // namespace ari::platform
