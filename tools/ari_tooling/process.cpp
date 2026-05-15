#include "process.hpp"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

namespace ari::tooling {
namespace {

void best_effort_write(int fd, const char* data, std::size_t size) {
    ssize_t ignored = write(fd, data, size);
    (void)ignored;
}

} // namespace

ProcessResult run_process(const std::vector<std::string>& args) {
    if (args.empty()) throw std::runtime_error("run_process requires at least one argument");

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        throw std::runtime_error(std::string("pipe failed: ") + std::strerror(errno));
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        throw std::runtime_error(std::string("fork failed: ") + std::strerror(errno));
    }

    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        std::vector<char*> argv;
        argv.reserve(args.size() + 1);
        for (const std::string& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        const char* prefix = "ari-tooling: exec failed: ";
        best_effort_write(STDERR_FILENO, prefix, std::strlen(prefix));
        best_effort_write(STDERR_FILENO, args[0].c_str(), args[0].size());
        const char* suffix = "\n";
        best_effort_write(STDERR_FILENO, suffix, std::strlen(suffix));
        _exit(127);
    }

    close(pipefd[1]);
    std::string output;
    char buffer[4096];
    while (true) {
        ssize_t count = read(pipefd[0], buffer, sizeof(buffer));
        if (count > 0) {
            output.append(buffer, static_cast<std::size_t>(count));
            continue;
        }
        if (count == 0) break;
        if (errno == EINTR) continue;
        close(pipefd[0]);
        throw std::runtime_error(std::string("read failed: ") + std::strerror(errno));
    }
    close(pipefd[0]);

    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) continue;
        throw std::runtime_error(std::string("waitpid failed: ") + std::strerror(errno));
    }

    int exit_code = 1;
    if (WIFEXITED(status)) exit_code = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) exit_code = 128 + WTERMSIG(status);
    return ProcessResult{exit_code, output};
}

} // namespace ari::tooling
