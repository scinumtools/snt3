#include "execution.h"

#include "mcp.h"

#include <chrono>
#include <fstream>
#include <thread>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
extern char** environ;
#endif

namespace snt::mcp {
    namespace fs = std::filesystem;
    namespace {
        fs::path executable() {
#ifdef _WIN32
            std::wstring path(32768, L'\0');
            auto length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
            if (!length || length == path.size())
                throw std::runtime_error("Cannot locate native executable.");
            path.resize(length);
            return fs::path(path);
#elif defined(__APPLE__)
            std::uint32_t size = 0;
            _NSGetExecutablePath(nullptr, &size);
            std::string path(size, '\0');
            if (_NSGetExecutablePath(path.data(), &size))
                throw std::runtime_error("Cannot locate native executable.");
            return fs::canonical(path.c_str());
#else
            return fs::read_symlink("/proc/self/exe");
#endif
        }
        class Worker {
#ifdef _WIN32
            HANDLE process_ = nullptr, job_ = nullptr;
            static std::wstring quote(const std::wstring& text) {
                std::wstring result = L"\"";
                unsigned slashes = 0;
                for (auto c : text) {
                    if (c == L'\\') {
                        ++slashes;
                        continue;
                    }
                    result.append(c == L'"' ? slashes * 2 + 1 : slashes, L'\\');
                    slashes = 0;
                    result += c;
                }
                result.append(slashes * 2, L'\\');
                return result + L'"';
            }
#else
            pid_t pid_ = -1;
#endif
            bool finished_ = false;

          public:
            Worker(const fs::path& request) {
#ifdef _WIN32
                const auto exe = executable();
                auto command = quote(exe.wstring()) + L" mcp --worker " + quote(request.wstring());
                STARTUPINFOW startup{};
                startup.cb = sizeof(startup);
                PROCESS_INFORMATION info{};
                job_ = CreateJobObjectW(nullptr, nullptr);
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
                limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                if (!job_ || !SetInformationJobObject(job_, JobObjectExtendedLimitInformation, &limits, sizeof(limits)))
                    throw std::runtime_error("Cannot create native worker job.");
                if (!CreateProcessW(
                        exe.c_str(),
                        command.data(),
                        nullptr,
                        nullptr,
                        FALSE,
                        CREATE_NO_WINDOW | CREATE_SUSPENDED,
                        nullptr,
                        nullptr,
                        &startup,
                        &info
                    )) {
                    CloseHandle(job_);
                    throw std::runtime_error("Cannot start native worker.");
                }
                process_ = info.hProcess;
                if (!AssignProcessToJobObject(job_, process_)) {
                    TerminateProcess(process_, 1);
                    CloseHandle(process_);
                    CloseHandle(info.hThread);
                    CloseHandle(job_);
                    throw std::runtime_error("Cannot supervise native worker.");
                }
                ResumeThread(info.hThread);
                CloseHandle(info.hThread);
#else
                const auto exe = executable().string(), file = request.string();
                char* args[] = {
                    const_cast<char*>(exe.c_str()),
                    const_cast<char*>("mcp"),
                    const_cast<char*>("--worker"),
                    const_cast<char*>(file.c_str()),
                    nullptr
                };
                posix_spawn_file_actions_t actions;
                posix_spawn_file_actions_init(&actions);
                // No stdout/stderr pipe can block an evaluating worker.
                posix_spawn_file_actions_addopen(&actions, STDIN_FILENO, "/dev/null", O_RDONLY, 0);
                posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
                posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);
                const auto error = posix_spawn(&pid_, exe.c_str(), &actions, nullptr, args, environ);
                posix_spawn_file_actions_destroy(&actions);
                if (error) {
                    pid_ = -1;
                    throw std::runtime_error("Cannot start native C++ worker.");
                }
#endif
            }
            bool done() {
                if (finished_)
                    return true;
#ifdef _WIN32
                finished_ = WaitForSingleObject(process_, 0) == WAIT_OBJECT_0;
#else
                int status = 0;
                const auto result = waitpid(pid_, &status, WNOHANG);
                if (result == -1 && errno != EINTR)
                    throw std::runtime_error("Cannot wait for native worker.");
                finished_ = result == pid_;
#endif
                return finished_;
            }
            ~Worker() {
#ifdef _WIN32
                if (!finished_) {
                    TerminateJobObject(job_, 1);
                    WaitForSingleObject(process_, INFINITE);
                }
                CloseHandle(process_);
                CloseHandle(job_);
#else
                if (pid_ > 0 && !finished_) {
                    kill(pid_, SIGKILL);
                    int status = 0;
                    while (waitpid(pid_, &status, 0) < 0 && errno == EINTR) {
                    }
                }
#endif
            }
        };
    } // namespace
    void Execution::cancel(const Json& id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (active_ && active_->id == id)
            active_->cancelled = true;
    }
    bool Execution::busy() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return bool(active_);
    }
    Json Execution::run(
        const Json& id, const std::string& tool, const Json& arguments, const std::function<bool()>& disconnected
    ) {
        auto active = std::make_shared<Active>();
        active->id = id;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (active_)
                return failure_result(
                    ServiceError("server.busy", "Another tool is executing; retry after it completes.")
                );
            active_ = active;
        }
        struct Reset {
            Execution* self;
            ~Reset() {
                std::lock_guard<std::mutex> lock(self->mutex_);
                self->active_.reset();
            }
        } reset{this};
        try {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms_);
            storage().prune();
            app::TemporaryDirectory workspace(directory_.path);
            const auto request = workspace.path / "request.json", response = workspace.path / "response.json";
            {
                std::ofstream output(request);
                output << Json{
                    {"projects", projects_},
                    {"root", directory_.path.string()},
                    {"tool", tool},
                    {"arguments", arguments}
                }.dump();
                if (!output)
                    throw std::runtime_error("Cannot write native worker request.");
            }
            Worker worker(request);
            for (;;) {
                if (active->cancelled || (disconnected && disconnected()))
                    return Json();
                if (std::chrono::steady_clock::now() >= deadline)
                    return failure_result(ServiceError(
                        "execution.timeout",
                        "Evaluation exceeded the configured time limit; narrow the request or raise --timeout-ms."
                    ));
                if (worker.done())
                    break;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            if (!fs::exists(response) || fs::file_size(response) > message_limit)
                return failure_result(
                    ServiceError("execution.failed", "Native evaluation worker exited without a valid response.")
                );
            std::ifstream input(response);
            auto result = Json::parse(input);
            if (!result.value("isError", false))
                Storage(directory_.path, workspace.path).publish();
            return result;
        } catch (const std::exception& error) {
            return failure_result(error);
        }
    }
    int worker_main(const fs::path& request) {
        const auto workspace = request.parent_path();
        Json result;
        try {
            std::ifstream input(request);
            auto config = Json::parse(input);
            Projects projects;
            for (auto it = config["projects"].begin(); it != config["projects"].end(); ++it)
                projects.emplace(it.key(), it.value().get<std::string>());
            Storage storage(config["root"].get<std::string>(), workspace);
            Tools tools(std::move(projects), &storage);
            auto value = tools.call(config["tool"], config["arguments"]);
            if (value.dump().size() > message_limit / 3)
                value = {
                    {"result_ref", storage.put_json(value, "result")},
                    {"summary", "Result is large; retrieve the complete JSON using artifact_read."}
                };
            Json content = {{{"type", "text"}, {"text", value.dump()}}};
            for (const auto* key : {"artifact", "result_ref"})
                if (value.contains(key)) {
                    const auto& artifact = value[key];
                    content.push_back(
                        {{"type", "resource_link"},
                         {"uri", artifact["uri"]},
                         {"name", artifact["filename"]},
                         {"mimeType", artifact["mimeType"]},
                         {"size", artifact["size"]}}
                    );
                }
            result = {{"isError", false}, {"structuredContent", value}, {"content", content}};
        } catch (const std::exception& error) {
            result = failure_result(error);
        }
        std::ofstream output(workspace / "response.json");
        output << result.dump();
        return output ? 0 : 1;
    }
} // namespace snt::mcp
