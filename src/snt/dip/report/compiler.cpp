#include "compiler.h"

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <process.h>
#include <cerrno>
#else
#include <cerrno>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace snt::dip::report {
namespace {
struct TemporaryDirectory {
    std::filesystem::path path;

    TemporaryDirectory() {
        const auto base = std::filesystem::temp_directory_path();
        const auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
        for (int attempt = 0; attempt < 100; ++attempt) {
            auto candidate = base / ("snt-dip-report-" + std::to_string(seed) + "-" + std::to_string(attempt));
            if (std::filesystem::create_directory(candidate)) {
                path = std::move(candidate);
                return;
            }
        }
        throw std::runtime_error("Cannot create a temporary directory for PDF compilation.");
    }

    ~TemporaryDirectory() {
        if (!path.empty()) {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    }
};

std::string compiler_log(const std::filesystem::path& path) {
    std::ifstream stream(path);
    if (!stream) return "";
    std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    if (content.size() > 1200) content = content.substr(content.size() - 1200);
    return content;
}

int run_compiler(const std::string& compiler, const std::filesystem::path& tex,
                 const std::filesystem::path& directory, const std::filesystem::path& log) {
    const std::string output_arg = "-output-directory=" + directory.string();
#ifdef _WIN32
    const std::string tex_arg = tex.string();
    const char* args[] = {compiler.c_str(), "-interaction=nonstopmode", "-halt-on-error",
                          output_arg.c_str(), tex_arg.c_str(), nullptr};
    // The Windows C runtime resolves the executable and quotes each argument.
    const int result = _spawnvp(_P_WAIT, compiler.c_str(), args);
    return result == -1 && errno == ENOENT ? 127 : result;
#else
    const std::string tex_arg = tex.string();
    const pid_t child = fork();
    if (child < 0) throw std::runtime_error("Could not start the TeX compiler.");
    if (child == 0) {
        int fd = open(log.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        char* const args[] = {const_cast<char*>(compiler.c_str()), const_cast<char*>("-interaction=nonstopmode"),
                              const_cast<char*>("-halt-on-error"), const_cast<char*>(output_arg.c_str()),
                              const_cast<char*>(tex_arg.c_str()), nullptr};
        execvp(compiler.c_str(), args);
        _exit(errno == ENOENT ? 127 : 126);
    }
    int status = 0;
    while (waitpid(child, &status, 0) < 0) {
        if (errno != EINTR) throw std::runtime_error("Could not wait for the TeX compiler.");
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
#endif
}
} // namespace

void write_file(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) throw std::runtime_error("Cannot open report output: " + path.string());
    stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (!stream) throw std::runtime_error("Cannot write report output: " + path.string());
}

void compile_pdf(const std::string& contents, const std::filesystem::path& output, const std::string& compiler) {
    TemporaryDirectory temporary;
    const auto tex = temporary.path / "report.tex";
    const auto pdf = temporary.path / "report.pdf";
    const auto log = temporary.path / "compiler.log";
    write_file(tex, contents);
    for (int pass = 0; pass < 2; ++pass) {
        const int status = run_compiler(compiler, tex, temporary.path, log);
        if (status == 127 || status == -1)
            throw std::runtime_error("TeX compiler '" + compiler + "' is unavailable. Install it or set --tex-compiler.");
        if (status != 0 || !std::filesystem::exists(pdf))
            throw std::runtime_error("TeX compiler failed (exit " + std::to_string(status) + ").\n" + compiler_log(log));
    }
    std::filesystem::copy_file(pdf, output, std::filesystem::copy_options::overwrite_existing);
}

} // namespace snt::dip::report
