#include "process.h"

#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace snt::hub {
namespace {
constexpr std::size_t output_limit = 256 * 1024;

void append_tail(std::string& output, const char* data, std::size_t size) {
    output.append(data, size);
    if (output.size() > output_limit) output.erase(0, output.size() - output_limit);
}
}

ProcessResult process(const std::vector<std::string>& args, bool capture) {
    if (args.empty() || args.front().empty()) throw std::invalid_argument("Empty process command");
    std::vector<char*> pointers;
    pointers.reserve(args.size() + 1);
    for (const auto& arg : args) pointers.push_back(const_cast<char*>(arg.c_str()));
    pointers.push_back(nullptr);
#ifdef _WIN32
    if (!capture) {
        const int result = _spawnvp(_P_WAIT, args.front().c_str(), pointers.data());
        return {result < 0 && errno == ENOENT ? 127 : result, {}};
    }
    const auto temporary = std::filesystem::temp_directory_path() /
        ("snt-hub-process-" + std::to_string(_getpid()) + "-" + std::to_string(std::rand()) + ".log");
    const int log = _open(temporary.string().c_str(), _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY,
                          _S_IREAD | _S_IWRITE);
    if (log < 0) throw std::runtime_error("Cannot create Hub process log");
    const int saved_out = _dup(1), saved_err = _dup(2);
    _dup2(log, 1);
    _dup2(log, 2);
    const int result = _spawnvp(_P_WAIT, args.front().c_str(), pointers.data());
    const int spawn_errno = errno;
    _dup2(saved_out, 1);
    _dup2(saved_err, 2);
    _close(saved_out);
    _close(saved_err);
    _close(log);
    std::ifstream stream(temporary, std::ios::binary);
    std::string output((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    std::error_code ignored;
    std::filesystem::remove(temporary, ignored);
    if (output.size() > output_limit) output.erase(0, output.size() - output_limit);
    return {result < 0 && spawn_errno == ENOENT ? 127 : result, std::move(output)};
#else
    int pipefd[2] = {-1, -1};
    if (capture && pipe(pipefd) != 0) throw std::runtime_error("Cannot create Hub process pipe");
    const pid_t child = fork();
    if (child < 0) {
        if (capture) { close(pipefd[0]); close(pipefd[1]); }
        throw std::runtime_error("Cannot start Hub process");
    }
    if (child == 0) {
        if (capture) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);
        }
        execvp(args.front().c_str(), pointers.data());
        _exit(errno == ENOENT ? 127 : 126);
    }
    ProcessResult result;
    if (capture) {
        close(pipefd[1]);
        char buffer[4096];
        for (;;) {
            const auto n = read(pipefd[0], buffer, sizeof(buffer));
            if (n > 0) append_tail(result.output, buffer, static_cast<std::size_t>(n));
            else if (n == 0) break;
            else if (errno != EINTR) break;
        }
        close(pipefd[0]);
    }
    int status = 0;
    while (waitpid(child, &status, 0) < 0) {
        if (errno != EINTR) throw std::runtime_error("Cannot wait for Hub process");
    }
    result.status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    return result;
#endif
}

} // namespace snt::hub
