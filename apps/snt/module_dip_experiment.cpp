#include "help.h"
#include "experiment_commands.h"

#include <snt/dip/experiment.h>

#include <filesystem>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    using snt::dip::ExperimentStatus;

    std::string help() {
        return snt::cli::format_help(R"(
Scientific Numerical Tools (SNT)

Usage:
  snt dip experiment prepare --input ENV.diph5 --output RUN.diph5
  snt dip experiment run --input ENV.diph5 --output RUN.diph5 -- COMMAND [ARG ...]
  snt dip experiment start --file RUN.diph5 [--origin host_reported] [--role outer_launcher] [--cwd DIR] -- COMMAND [ARG ...]
  snt dip experiment finish --file RUN.diph5 --status completed|failed|aborted [--exit-code N]
  snt dip experiment import --input ENV.diph5 --output RUN.diph5 --status completed|failed|aborted
  snt dip experiment show --file RUN.diph5
  snt dip experiment add --file RUN.diph5 --name NAME --values 1,2,3 [--shape 3] [--unit UNIT]
  snt dip experiment attach --file RUN.diph5 --name NAME --artifact FILE [--format FORMAT]
  snt dip experiment diagnostic --file RUN.diph5 --name NAME --values NUMBER --category observed|judgment
  snt dip experiment results|finalize|verify --file RUN.diph5

Description:
  Record one execution alongside a source-complete DIPH5 v3 configuration.

Commands:
  prepare  Create a prepared experiment from a v3 environment archive.
  run      Launch a command without a shell and record its observed argv and exit code.
  start    Record a command launched by an external host or scheduler.
  finish   Record a terminal outcome for a running experiment.
  import   Record a past outcome; invocation and timestamps not observed remain unknown.
  show     Print the recorded status and command.
  add      Store a typed scalar or array in the running experiment.
  attach   Register and hash an external artifact.
  diagnostic  Store an observed statistic or validation judgment.
  results  List the result inventory.
  finalize  Seal the result inventory before completing the run.
  verify   Check the archive and reachable external artifacts.

Options:
  --input FILE       Source-complete v3 environment archive.
  --output FILE      New experiment file; existing files are not overwritten.
  --file FILE        Existing experiment file.
  --origin ORIGIN    launcher_observed or host_reported (default: host_reported for start).
  --role ROLE        scientific_program or outer_launcher.
  --cwd DIR          Working directory reported for the command.
  --executable-path FILE    Executable path reported by an external host.
  --executable-sha256 HEX   SHA-256 reported by that host, with --executable-size.
  --executable-size N       Executable byte size reported by that host.
  --status STATUS    completed, failed, or aborted.
  --exit-code N      Observed process exit code, when known.
  --termination-signal N  POSIX signal number, when the process did not exit normally.
  --name NAME        Human-readable result name.
  --values CSV      One or more comma-separated values.
  --dtype TYPE      DIPL type: bool, char, int8/16/32/64, uint8/16/32/64, float32/64 (default: float64).
  --shape CSV       Array extents; omit for a scalar.
  --unit UNIT       Unit for an embedded dataset.
  --dimensions CSV  Optional names for each array dimension.
  --description TEXT  Result description.
  --artifact FILE   External file to register.
  --format FORMAT   External artifact format.
  --category KIND   observed or judgment for a diagnostic.

Examples:
  snt dip parse -i file parameters.dip --save prepared.diph5
  snt dip experiment run --input prepared.diph5 --output run.diph5 -- python simulate.py
  snt dip experiment show --file run.diph5

Notes:
  The run command stores exact arguments but never passes them through a shell.
  Arguments may contain secrets; inspect them before archiving a command.
)");
    }

    ExperimentStatus terminal_status(const std::string& value) {
        if (value == "completed") return ExperimentStatus::Completed;
        if (value == "failed") return ExperimentStatus::Failed;
        if (value == "aborted") return ExperimentStatus::Aborted;
        throw std::invalid_argument("--status must be completed, failed, or aborted");
    }

    const char* name(ExperimentStatus status) {
        switch (status) {
        case ExperimentStatus::Prepared: return "prepared";
        case ExperimentStatus::Running: return "running";
        case ExperimentStatus::Completed: return "completed";
        case ExperimentStatus::Failed: return "failed";
        case ExperimentStatus::Aborted: return "aborted";
        }
        return "unknown";
    }

    int integer(const std::string& value) {
        size_t consumed = 0;
        const int result = std::stoi(value, &consumed);
        if (consumed != value.size())
            throw std::invalid_argument("Expected an integer, got: " + value);
        return result;
    }

    std::vector<std::string> csv(const std::string& value) {
        std::vector<std::string> items;
        std::stringstream stream(value);
        std::string item;
        while (std::getline(stream, item, ',')) {
            if (item.empty()) throw std::invalid_argument("Empty comma-separated value");
            items.push_back(item);
        }
        if (items.empty()) throw std::invalid_argument("Expected a comma-separated value");
        return items;
    }
}

int module_dip_experiment(int argc, char* argv[]) {
    if (argc < 4 || std::string(argv[3]) == "--help" || std::string(argv[3]) == "-h") {
        std::cout << help();
        return 0;
    }
    const std::string action = argv[3];
    if (std::none_of(snt::cli::experiment_commands.begin(), snt::cli::experiment_commands.end(),
                     [&](const auto& command) { return action == command.name; }))
        throw std::invalid_argument("Unknown experiment command: " + action);
    std::string input, output, file, origin, role, cwd, status, executable_path, executable_sha256;
    std::string result_name, values_text, shape_text, unit, dimensions_text, description, artifact, format, category;
    std::string dtype = "float64";
    std::optional<uint64_t> executable_size;
    std::optional<int> exit_code;
    std::optional<int> termination_signal;
    std::vector<std::string> command;
    for (int i = 4; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--") {
            for (++i; i < argc; ++i)
                command.emplace_back(argv[i]);
            break;
        }
        if (arg == "--help" || arg == "-h") {
            std::cout << help();
            return 0;
        }
        if (i + 1 >= argc)
            throw std::invalid_argument("Missing value after " + arg);
        const std::string value = argv[++i];
        if (arg == "--input") input = value;
        else if (arg == "--output") output = value;
        else if (arg == "--file") file = value;
        else if (arg == "--origin") origin = value;
        else if (arg == "--role") role = value;
        else if (arg == "--cwd") cwd = value;
        else if (arg == "--executable-path") executable_path = value;
        else if (arg == "--executable-sha256") executable_sha256 = value;
        else if (arg == "--executable-size") {
            if (value.empty() || value.front() == '-')
                throw std::invalid_argument("Invalid --executable-size");
            size_t consumed = 0;
            executable_size = std::stoull(value, &consumed);
            if (consumed != value.size())
                throw std::invalid_argument("Invalid --executable-size");
        }
        else if (arg == "--status") status = value;
        else if (arg == "--exit-code") exit_code = integer(value);
        else if (arg == "--termination-signal") termination_signal = integer(value);
        else if (arg == "--name") result_name = value;
        else if (arg == "--values") values_text = value;
        else if (arg == "--dtype") dtype = value;
        else if (arg == "--shape") shape_text = value;
        else if (arg == "--unit") unit = value;
        else if (arg == "--dimensions") dimensions_text = value;
        else if (arg == "--description") description = value;
        else if (arg == "--artifact") artifact = value;
        else if (arg == "--format") format = value;
        else if (arg == "--category") category = value;
        else throw std::invalid_argument("Unknown option: " + arg);
    }
    if (action == "prepare") {
        if (input.empty() || output.empty() || !file.empty() || !command.empty())
            throw std::invalid_argument("prepare requires --input and --output");
        snt::dip::Experiment::prepare_from_archive(input, output);
        std::cout << output << '\n';
        return 0;
    }
    if (action == "run" || action == "start") {
        if (command.empty())
            throw std::invalid_argument("A command is required after --");
        if (action == "run" && (input.empty() || output.empty() || !file.empty()))
            throw std::invalid_argument("run requires --input and --output");
        if (action == "start" && (file.empty() || !input.empty() || !output.empty()))
            throw std::invalid_argument("start requires --file");
        if (action == "run" && (!cwd.empty() || (!origin.empty() && origin != "launcher_observed")))
            throw std::invalid_argument("run captures its actual working directory and launcher origin");
        snt::dip::ExperimentCommand details;
        details.argv = command;
        if (!role.empty())
            details.command_role = role;
        details.working_directory = cwd.empty() ? std::filesystem::current_path() : std::filesystem::path(cwd);
        details.capture_origin = origin.empty() ? (action == "run" ? "launcher_observed" : "host_reported") : origin;
        details.reported_executable_path = executable_path;
        details.reported_executable_sha256 = executable_sha256;
        details.reported_executable_size_bytes = executable_size;
        if (action == "run")
            return snt::dip::Experiment::run_from_archive(input, output, std::move(details)).status;
        snt::dip::Experiment::open(file).start(details);
        return 0;
    }
    if (action == "finish") {
        if (file.empty() || status.empty())
            throw std::invalid_argument("finish requires --file and --status");
        snt::dip::Experiment::open(file).finish(terminal_status(status), exit_code, {}, termination_signal);
        return 0;
    }
    if (action == "import") {
        if (input.empty() || output.empty() || status.empty())
            throw std::invalid_argument("import requires --input, --output, and --status");
        auto experiment = snt::dip::Experiment::prepare_from_archive(input, output);
        experiment.import(terminal_status(status), exit_code, {}, termination_signal);
        return 0;
    }
    if (action == "show") {
        if (file.empty())
            throw std::invalid_argument("show requires --file");
        const auto outcome = snt::dip::Experiment::open(file).outcome();
        std::cout << "Status: " << name(outcome.status) << '\n'
                  << "Record ID: " << outcome.record_id << '\n';
        if (outcome.command) {
            std::cout << "Command role: " << outcome.command->command_role << '\n';
            for (size_t i = 0; i < outcome.command->argv.size(); ++i)
                std::cout << "argv[" << i << "]: " << outcome.command->argv[i] << '\n';
        }
        if (!outcome.resolved_executable_path.empty())
            std::cout << "Executable path: " << outcome.resolved_executable_path << '\n';
        if (!outcome.executable_sha256.empty())
            std::cout << "Executable SHA-256: " << outcome.executable_sha256 << '\n';
        if (!outcome.system_os.empty())
            std::cout << "System: " << outcome.system_os << " (" << outcome.system_architecture << ")\n";
        if (outcome.exit_code)
            std::cout << "Exit code: " << *outcome.exit_code << '\n';
        if (outcome.termination_signal)
            std::cout << "Termination signal: " << *outcome.termination_signal << '\n';
        return 0;
    }
    if (action == "add" || action == "diagnostic") {
        if (file.empty() || result_name.empty() || values_text.empty())
            throw std::invalid_argument(action + " requires --file, --name, and --values");
        const auto items = csv(values_text);
        auto experiment = snt::dip::Experiment::open(file);
        if (action == "diagnostic") {
            if (items.size() != 1 || category.empty() || dtype != "float64")
                throw std::invalid_argument("diagnostic requires one value and --category");
            size_t used = 0;
            const auto value = std::stod(items.front(), &used);
            if (used != items.front().size()) throw std::invalid_argument("Invalid diagnostic value");
            std::cout << experiment.add_diagnostic(result_name, value, category, description) << '\n';
        } else {
            snt::dip::ExperimentValues values;
            if (dtype == "char" || dtype.rfind("int", 0) == 0) {
                std::vector<int64_t> parsed;
                for (const auto& item : items) {
                    size_t used = 0;
                    parsed.push_back(std::stoll(item, &used));
                    if (used != item.size()) throw std::invalid_argument("Invalid signed value: " + item);
                }
                values = std::move(parsed);
            } else if (dtype == "bool" || dtype.rfind("uint", 0) == 0) {
                std::vector<uint64_t> parsed;
                for (const auto& item : items) {
                    if (item.empty() || item.front() == '-') throw std::invalid_argument("Invalid unsigned value: " + item);
                    size_t used = 0;
                    parsed.push_back(std::stoull(item, &used));
                    if (used != item.size()) throw std::invalid_argument("Invalid unsigned value: " + item);
                }
                values = std::move(parsed);
            } else {
                std::vector<double> parsed;
                for (const auto& item : items) {
                    size_t used = 0;
                    parsed.push_back(std::stod(item, &used));
                    if (used != item.size()) throw std::invalid_argument("Invalid floating-point value: " + item);
                }
                values = std::move(parsed);
            }
            std::vector<uint64_t> shape;
            if (!shape_text.empty())
                for (const auto& item : csv(shape_text)) {
                    size_t used = 0;
                    shape.push_back(std::stoull(item, &used));
                    if (used != item.size()) throw std::invalid_argument("Invalid array extent: " + item);
                }
            std::cout << experiment.add_typed_dataset(result_name, values, dtype, shape, unit,
                                                dimensions_text.empty() ? std::vector<std::string>{}
                                                                        : csv(dimensions_text), description) << '\n';
        }
        return 0;
    }
    if (action == "attach") {
        if (file.empty() || result_name.empty() || artifact.empty())
            throw std::invalid_argument("attach requires --file, --name, and --artifact");
        std::cout << snt::dip::Experiment::open(file).attach_artifact(artifact, result_name, format) << '\n';
        return 0;
    }
    if (action == "results" || action == "finalize" || action == "verify") {
        if (file.empty()) throw std::invalid_argument(action + " requires --file");
        auto experiment = snt::dip::Experiment::open(file);
        if (action == "finalize") experiment.finalize_results();
        else if (action == "results") {
            for (const auto& result : experiment.results())
                std::cout << result.result_id << '\t' << result.kind << '\t' << result.status << '\t'
                          << result.name << '\n';
        } else {
            const auto check = experiment.verify();
            std::cout << "Structural: " << check.structural << '\n'
                      << "Integrity: " << check.integrity << '\n'
                      << "Configuration fingerprint: " << check.configuration_fingerprint << '\n'
                      << "External artifacts: " << check.external_artifacts << '\n'
                      << "Re-executability: " << check.reexecutability << '\n';
            for (const auto& detail : check.details) std::cout << detail << '\n';
            return check.structural == "fail" || check.integrity == "fail" || check.external_artifacts == "fail";
        }
        return 0;
    }
    throw std::invalid_argument("Unknown experiment command");
}
