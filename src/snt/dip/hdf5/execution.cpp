#include "io_private.h"
#include "../input/utf8.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <snt/dip/environment.h>
#include <snt/sha256.h>
#include <sstream>
#include <thread>

namespace snt::dip::hdf5 {
    using namespace detail;
    namespace detail {
        ExperimentOutcome inspect_execution(hid_t input) {
            if (required_utf8(input, "_DIPH5_Profile") != "experiment")
                throw Error("This DIPH5 file is not an experiment");
            Id execution(H5Gopen2(input, "/execution", H5P_DEFAULT), H5Gclose, "Missing execution group");
            Id outcome(H5Gopen2(execution, "outcome", H5P_DEFAULT), H5Gclose, "Missing execution outcome");
            ExperimentOutcome result;
            result.status = parse_status(required_utf8(outcome, "status"));
            if (required_utf8(input, "_DIPH5_State") != status_name(result.status))
                throw Error("Root and execution outcome statuses disagree");
            result.record_id = required_utf8(input, "_DIPH5_Record_Id");
            result.created_at = required_utf8(outcome, "created_at");
            if (result.created_at.empty())
                throw Error("Execution outcome has no creation time");
            result.started_at = read_string(outcome, "started_at");
            result.finished_at = read_string(outcome, "finished_at");
            result.message = read_string(outcome, "message");
            result.capture_origin = read_string(outcome, "capture_origin");
            if (result.status != ExperimentStatus::Prepared &&
                result.capture_origin != "launcher_observed" && result.capture_origin != "host_reported" &&
                result.capture_origin != "imported")
                throw Error("Execution outcome has an invalid capture origin");
            if (has_attribute(outcome, "exit_code"))
                result.exit_code = required_scalar<int32_t>(outcome, "exit_code", H5T_NATIVE_INT32);
            if (has_attribute(outcome, "termination_signal"))
                result.termination_signal = required_scalar<int32_t>(outcome, "termination_signal", H5T_NATIVE_INT32);
            if (result.exit_code && result.termination_signal)
                throw Error("An execution outcome cannot have both exit code and termination signal");
            if (result.status == ExperimentStatus::Running && result.started_at.empty() &&
                result.capture_origin == "launcher_observed")
                throw Error("Running experiment has no start time");
            if ((result.status == ExperimentStatus::Completed || result.status == ExperimentStatus::Failed ||
                 result.status == ExperimentStatus::Aborted) && result.finished_at.empty() &&
                result.capture_origin == "launcher_observed")
                throw Error("Terminal experiment has no finish time");
            if (has_link(execution, "command")) {
                Id command(H5Gopen2(execution, "command", H5P_DEFAULT), H5Gclose, "Invalid execution command");
                ExperimentCommand value;
                value.argv = read_argv(command, "argv");
                value.working_directory = required_utf8(command, "working_directory");
                value.command_role = required_utf8(command, "command_role");
                value.capture_origin = required_utf8(command, "capture_origin");
                value.argv_redacted = required_scalar<uint8_t>(command, "argv_redacted", H5T_NATIVE_UINT8) != 0;
                if (value.command_role != "scientific_program" && value.command_role != "outer_launcher")
                    throw Error("Invalid execution command role");
                if (value.command_role == "outer_launcher" &&
                    required_utf8(command, "scientific_child_invocation") != "unknown")
                    throw Error("An outer launcher must mark the scientific child invocation unknown");
                if (value.capture_origin != "launcher_observed" && value.capture_origin != "host_reported" &&
                    value.capture_origin != "imported")
                    throw Error("Invalid execution command capture origin");
                if (value.argv.empty() || value.argv[0] != required_utf8(command, "executable_invoked"))
                    throw Error("Execution argv[0] differs from the invoked executable");
                result.resolved_executable_path = read_string(command, "resolved_executable_path");
                if (has_link(command, "launcher_argv"))
                    value.launcher_argv = read_argv(command, "launcher_argv");
                result.command = std::move(value);
            }
            if (has_link(execution, "software")) {
                Id software(H5Gopen2(execution, "software", H5P_DEFAULT), H5Gclose,
                            "Invalid execution software group");
                result.executable_sha256 = read_string(software, "executable_sha256");
                if (has_attribute(software, "executable_size_bytes"))
                    result.executable_size_bytes = required_scalar<uint64_t>(
                        software, "executable_size_bytes", H5T_NATIVE_UINT64);
                if (result.executable_sha256.empty() != !result.executable_size_bytes)
                    throw Error("Incomplete executable identity");
            }
            if (has_link(execution, "runtime")) {
                Id runtime(H5Gopen2(execution, "runtime", H5P_DEFAULT), H5Gclose,
                           "Invalid execution runtime group");
                if (has_link(runtime, "system")) {
                    Id system(H5Gopen2(runtime, "system", H5P_DEFAULT), H5Gclose,
                              "Invalid execution system facts");
                    result.system_os = read_string(system, "os");
                    result.system_architecture = read_string(system, "architecture");
                    if (has_attribute(system, "hardware_threads"))
                        result.hardware_threads = required_scalar<uint32_t>(system, "hardware_threads",
                                                                             H5T_NATIVE_UINT32);
                }
            }
            if (result.command && result.command->capture_origin == "host_reported") {
                result.command->reported_executable_path = result.resolved_executable_path;
                result.command->reported_executable_sha256 = result.executable_sha256;
                result.command->reported_executable_size_bytes = result.executable_size_bytes;
            }
            if (result.command && result.capture_origin != result.command->capture_origin)
                throw Error("Command and outcome capture origins disagree");
            if (result.status == ExperimentStatus::Running && !result.command)
                throw Error("Running experiment has no command");
            return result;
        }

    } // namespace detail
    namespace {
        std::optional<std::filesystem::path> resolved_executable(const ExperimentCommand& command) {
            const auto invoked = std::filesystem::path(command.argv.front());
            std::vector<std::filesystem::path> candidates;
            if (invoked.is_absolute() || invoked.has_parent_path()) {
                candidates.push_back(invoked.is_absolute() ? invoked : command.working_directory / invoked);
            } else if (const char* path = std::getenv("PATH")) {
#ifdef _WIN32
                constexpr char separator = ';';
#else
                constexpr char separator = ':';
#endif
                std::stringstream directories(path);
                std::string directory;
                while (std::getline(directories, directory, separator)) {
                    const auto base = directory.empty() ? command.working_directory : std::filesystem::path(directory);
                    candidates.push_back((base.is_absolute() ? base : command.working_directory / base) / invoked);
                }
            }
            for (const auto& candidate : candidates) {
                std::error_code error;
                if (std::filesystem::is_regular_file(candidate, error)) {
                    auto path = std::filesystem::canonical(candidate, error);
                    if (!error)
                        return path;
                }
            }
            return std::nullopt;
        }

    } // namespace
    void prepare_experiment(const std::filesystem::path& file) {
        Environment validated;
        load(validated, file);
        Id archive = open_experiment_file(file);
        if (required_utf8(archive, "_DIPH5_Profile") != "environment")
            throw Error("Only an environment-profile archive can prepare a new experiment");
        const auto now = utc_now();
        write_string(archive, "_DIPH5_Profile", "experiment");
        write_string(archive, "_DIPH5_Record_Id", uuid());
        write_string(archive, "_DIPH5_Created_At", now);
        Id execution(H5Gcreate2(archive, "/execution", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                     "Unable to create execution record");
        Id outcome(H5Gcreate2(execution, "outcome", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                   "Unable to create execution outcome");
        write_string(outcome, "status", "prepared");
        write_string(outcome, "created_at", now);
        check(H5Fflush(archive, H5F_SCOPE_GLOBAL), "Unable to flush prepared experiment");
    }

    ExperimentOutcome read_experiment(const std::filesystem::path& file) {
        Environment validated;
        load(validated, file);
        H5Eset_auto2(H5E_DEFAULT, nullptr, nullptr);
        Id archive(H5Fopen(file.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose,
                   "Unable to open DIPH5 experiment");
        return inspect_execution(archive);
    }
    void start_experiment(const std::filesystem::path& file, const ExperimentCommand& command) {
        if (read_experiment(file).status != ExperimentStatus::Prepared)
            throw Error("Only a prepared experiment can start");
        if (command.argv.empty() || command.argv.front().empty() || command.working_directory.empty() ||
            !valid_utf8(command.working_directory.generic_string()) ||
            (command.command_role != "scientific_program" && command.command_role != "outer_launcher") ||
            (command.capture_origin != "launcher_observed" && command.capture_origin != "host_reported"))
            throw Error("Invalid experiment command or capture origin");
        checked_argv_width(command.argv);
        if (!command.launcher_argv.empty())
            checked_argv_width(command.launcher_argv);
        const auto executable = command.capture_origin == "launcher_observed"
                                    ? resolved_executable(command) : std::optional<std::filesystem::path>{};
        std::optional<std::string> executable_hash;
        std::optional<uint64_t> executable_size;
        if (executable) {
            std::error_code error;
            const auto size = std::filesystem::file_size(*executable, error);
            if (!error) {
                try {
                    executable_hash = hash_file(*executable);
                    executable_size = size;
                } catch (const Error&) {
                    // The path is still useful even if another process changes the file before hashing.
                }
            }
        }
        if (command.capture_origin == "host_reported") {
            if (!valid_utf8(command.reported_executable_path) ||
                (!command.reported_executable_sha256.empty() &&
                 (command.reported_executable_sha256.size() != 64 ||
                  !std::all_of(command.reported_executable_sha256.begin(),
                               command.reported_executable_sha256.end(), [](char digit) {
                                   return (digit >= '0' && digit <= '9') ||
                                          (digit >= 'a' && digit <= 'f');
                               }))) ||
                (command.reported_executable_sha256.empty() != !command.reported_executable_size_bytes))
                throw Error("Incomplete or invalid host-reported executable identity");
            if (!command.reported_executable_sha256.empty()) {
                executable_hash = command.reported_executable_sha256;
                executable_size = command.reported_executable_size_bytes;
            }
        } else if (!command.reported_executable_path.empty() ||
                   !command.reported_executable_sha256.empty() || command.reported_executable_size_bytes) {
            throw Error("A launcher-observed command cannot use host-reported executable identity");
        }
        Id archive = open_experiment_file(file);
        Id execution(H5Gopen2(archive, "/execution", H5P_DEFAULT), H5Gclose, "Missing execution record");
        Id details(H5Gcreate2(execution, "command", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                   "Unable to create execution command");
        write_argv(details, "argv", command.argv);
        if (!command.launcher_argv.empty())
            write_argv(details, "launcher_argv", command.launcher_argv);
        write_string(details, "executable_invoked", command.argv.front());
        write_string(details, "working_directory", command.working_directory.generic_string());
        write_string(details, "command_role", command.command_role);
        if (command.command_role == "outer_launcher")
            write_string(details, "scientific_child_invocation", "unknown");
        write_string(details, "capture_origin", command.capture_origin);
        write_scalar<uint8_t>(details, "argv_redacted", H5T_NATIVE_UINT8, command.argv_redacted);

        Id software(H5Gcreate2(execution, "software", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                    "Unable to create execution software record");
        if (executable) {
            write_string(details, "resolved_executable_path", executable->generic_string());
        }
        if (!command.reported_executable_path.empty())
            write_string(details, "resolved_executable_path", command.reported_executable_path);
        if (executable_hash) {
            write_string(software, "executable_sha256", *executable_hash);
            write_scalar<uint64_t>(software, "executable_size_bytes", H5T_NATIVE_UINT64, *executable_size);
        }
        Id runtime(H5Gcreate2(execution, "runtime", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                   "Unable to create execution runtime record");
        // For a reported command, this process may run on a different machine from the experiment.
        if (command.capture_origin == "launcher_observed") {
            Id system(H5Gcreate2(runtime, "system", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT), H5Gclose,
                      "Unable to create system facts");
            write_scalar<uint32_t>(system, "hardware_threads", H5T_NATIVE_UINT32,
                                   std::thread::hardware_concurrency());
#ifdef _WIN32
        write_string(system, "os", "Windows");
#elif defined(__APPLE__)
        write_string(system, "os", "macOS");
#else
        write_string(system, "os", "Linux");
#endif
#if defined(__x86_64__) || defined(_M_X64)
        write_string(system, "architecture", "x86_64");
#elif defined(__aarch64__) || defined(_M_ARM64)
        write_string(system, "architecture", "aarch64");
#elif defined(__i386__) || defined(_M_IX86)
        write_string(system, "architecture", "x86");
#else
        write_string(system, "architecture", "unknown");
#endif
        write_string(system, "capture_origin", command.capture_origin);
        write_string(system, "captured_at", utc_now());
        }
        Id outcome(H5Gopen2(execution, "outcome", H5P_DEFAULT), H5Gclose, "Missing execution outcome");
        if (command.capture_origin == "launcher_observed")
            write_string(outcome, "started_at", utc_now());
        else
            write_string(outcome, "recorded_start_at", utc_now());
        write_string(outcome, "capture_origin", command.capture_origin);
        write_string(outcome, "status", "running");
        write_string(archive, "_DIPH5_State", "running");
        check(H5Fflush(archive, H5F_SCOPE_GLOBAL), "Unable to flush running experiment");
    }

    void finish_experiment(const std::filesystem::path& file, ExperimentStatus status,
                           std::optional<int> exit_code, const std::string& message, bool imported,
                           std::optional<int> termination_signal) {
        const auto before = read_experiment(file);
        if (status != ExperimentStatus::Completed && status != ExperimentStatus::Failed &&
            status != ExperimentStatus::Aborted)
            throw Error("Finish requires completed, failed, or aborted status");
        if ((imported && before.status != ExperimentStatus::Prepared) ||
            (!imported && before.status != ExperimentStatus::Running))
            throw Error("Invalid experiment state transition");
        if (!valid_utf8(message))
            throw Error("Invalid UTF-8 in execution message");
        if (exit_code && termination_signal)
            throw Error("Specify either an exit code or a termination signal");
        if (termination_signal && (*termination_signal <= 0 || status == ExperimentStatus::Completed))
            throw Error("A completed run cannot have a termination signal");
        if (status == ExperimentStatus::Completed) {
            Id check_file = open_experiment_file(file);
            bool finalized = false;
            if (has_link(check_file, "/results") && has_link(check_file, "/results/manifest")) {
                Id manifest(H5Gopen2(check_file, "/results/manifest", H5P_DEFAULT), H5Gclose,
                            "Invalid result manifest");
                finalized = required_scalar<uint8_t>(manifest, "finalized", H5T_NATIVE_UINT8) != 0;
            }
            check_file = Id();
            if (!finalized) finalize_results(file, imported);
            const auto verification = verify_experiment(file);
            if (verification.structural != "pass" || verification.integrity != "pass" ||
                verification.external_artifacts == "fail")
                throw Error("Cannot complete experiment: archive or artifact verification failed");
        }
        Id archive = open_experiment_file(file);
        Id outcome(H5Gopen2(archive, "/execution/outcome", H5P_DEFAULT), H5Gclose,
                   "Missing execution outcome");
        if (exit_code)
            write_scalar<int32_t>(outcome, "exit_code", H5T_NATIVE_INT32, *exit_code);
        if (termination_signal)
            write_scalar<int32_t>(outcome, "termination_signal", H5T_NATIVE_INT32, *termination_signal);
        if (!message.empty())
            write_string(outcome, "message", message);
        if (imported) {
            write_string(outcome, "imported_at", utc_now());
            write_string(outcome, "capture_origin", "imported");
        } else if (before.capture_origin == "launcher_observed") {
            write_string(outcome, "finished_at", utc_now());
        } else {
            write_string(outcome, "recorded_terminal_at", utc_now());
        }
        write_string(outcome, "status", status_name(status));
        write_string(archive, "_DIPH5_State", status_name(status));
        check(H5Fflush(archive, H5F_SCOPE_GLOBAL), "Unable to flush terminal experiment");
    }
} // namespace snt::dip::hdf5
