#ifndef SNT_DIP_EXPERIMENT_H
#define SNT_DIP_EXPERIMENT_H

#include <filesystem>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace snt::dip {
    class Environment;

    using ExperimentValues = std::variant<std::vector<int64_t>, std::vector<uint64_t>, std::vector<double>>;

    enum class ExperimentStatus { Prepared, Running, Completed, Failed, Aborted };

    struct ExperimentCommand {
        std::vector<std::string> argv;                 ///< Observed argument vector, including the executable.
        std::filesystem::path working_directory;      ///< Directory used to launch the command.
        std::string command_role = "scientific_program"; ///< Whether argv identifies the program or a launcher.
        std::string capture_origin = "launcher_observed"; ///< How the command details were obtained.
        bool argv_redacted = false;                    ///< Whether sensitive arguments were removed.
        std::vector<std::string> launcher_argv;        ///< Outer launcher arguments when separately known.
        std::string reported_executable_path;         ///< Executable path supplied by an external host.
        std::string reported_executable_sha256;       ///< Executable digest supplied by an external host.
        std::optional<uint64_t> reported_executable_size_bytes; ///< Reported executable size, when known.
    };

    struct ExperimentOutcome {
        ExperimentStatus status = ExperimentStatus::Prepared; ///< Current lifecycle state.
        std::string record_id;                        ///< Stable identity of this execution attempt.
        std::string created_at;                       ///< Archive creation timestamp.
        std::string started_at;                       ///< Start timestamp, if observed.
        std::string finished_at;                      ///< Terminal timestamp, if observed.
        std::optional<int> exit_code;                 ///< Process exit status, when known.
        std::optional<int> termination_signal;        ///< Signal that ended the process, when known.
        std::string message;                          ///< Human-readable outcome detail.
        std::string capture_origin;                   ///< Provenance of the outcome report.
        std::optional<ExperimentCommand> command;     ///< Recorded invocation, if available.
        std::string resolved_executable_path;         ///< Resolved executable path, if available.
        std::string executable_sha256;                ///< Digest of the executable bytes, if available.
        std::optional<uint64_t> executable_size_bytes; ///< Size of the executable bytes, if available.
        std::string system_os;                        ///< Operating system recorded at start.
        std::string system_architecture;              ///< Machine architecture recorded at start.
        std::optional<uint32_t> hardware_threads;     ///< Hardware thread count, if observed.
    };

    struct ExperimentResult {
        std::string result_id;             ///< Stable identifier within the archive.
        std::string name;                  ///< User-facing result name.
        std::string kind;                  ///< Dataset, artifact, or diagnostic kind.
        std::string storage;               ///< Embedded or external storage mode.
        std::string status;                ///< Result lifecycle state.
        std::string path_or_uri;           ///< External artifact location, when applicable.
        std::string verification;          ///< Artifact verification state.
        std::string sha256;                ///< Hash of the result payload, when available.
        std::optional<uint64_t> size_bytes; ///< External artifact size, when available.
        std::string units;                 ///< Unit spelling for a numeric result.
        std::string description;           ///< User-provided result description.
        std::vector<std::string> dimensions; ///< Ordered names of array axes.
        std::vector<uint64_t> shape;       ///< Numeric dataset extents.
        std::vector<double> values;        ///< Numeric values converted to double for display.
        std::string logical_dtype;         ///< Declared DIPL numeric type.
        ExperimentValues typed_values;     ///< Values retained at their declared numeric width.
    };

    struct ExperimentVerification {
        std::string structural;            ///< Archive structure check status.
        std::string integrity;             ///< Internal checksum check status.
        std::string external_artifacts;    ///< External file availability and checksum status.
        std::string reexecutability;       ///< Whether recorded inputs could support rerunning.
        std::vector<std::string> details;  ///< Human-readable verification findings.
        std::string configuration_fingerprint = "unavailable"; ///< Resolved configuration digest status.
    };

    /**
     * One execution record per DIPH5 file. Methods reopen the file at each lifecycle boundary.
     */
    class Experiment {
      public:
        /** Create a prepared archive from a parsed model.
         *  @param environment Parsed configuration with source bytes. @param file New archive path. */
        static Experiment prepare(const Environment& environment, const std::filesystem::path& file);
        /** Copy a prepared environment archive into a new experiment record.
         *  @param environment_file Source archive. @param experiment_file New archive path. */
        static Experiment prepare_from_archive(const std::filesystem::path& environment_file,
                                               const std::filesystem::path& experiment_file);
        /**
         * Open an existing experiment.
         * @param file Archive path.
         */
        static Experiment open(const std::filesystem::path& file);

        /**
         * Record the command before execution.
         * @param command Observed invocation and runtime context.
         */
        void start(const ExperimentCommand& command) const;
        /** Close a running record. @param status Terminal status. @param exit_code Process exit code if known.
         *  @param message Human-readable outcome. @param termination_signal Signal if known. */
        void finish(ExperimentStatus status, std::optional<int> exit_code = {}, const std::string& message = {},
                    std::optional<int> termination_signal = {}) const;
        /** Record a past run when no live start event was observed.
         *  @param status Terminal status. @param exit_code Reported exit code, if known.
         *  @param message Human-readable outcome. @param termination_signal Reported signal, if known. */
        void import(ExperimentStatus status, std::optional<int> exit_code = {},
                    const std::string& message = {}, std::optional<int> termination_signal = {}) const;
        /** Add an embedded numeric result. @param name Result name. @param values Values in row-major order.
         *  @param shape Array dimensions, empty for a scalar. @param units PUEL unit spelling.
         *  @param dimensions Optional axis names. @param description Human-readable description.
         *  @return Stable result identifier. */
        std::string add_dataset(const std::string& name, const std::vector<double>& values,
                                const std::vector<uint64_t>& shape, const std::string& units = {},
                                const std::vector<std::string>& dimensions = {}, const std::string& description = {}) const;
        /**
         * Store values using their declared DIPL numeric type. Values must fit the requested type.
         */
        std::string add_typed_dataset(const std::string& name, const ExperimentValues& values,
                                      const std::string& dtype, const std::vector<uint64_t>& shape,
                                      const std::string& units = {},
                                      const std::vector<std::string>& dimensions = {},
                                      const std::string& description = {}) const;
        /** Register a result file without embedding it. @param artifact File to inspect.
         *  @param name Result name. @param format Format label. @return Stable result identifier. */
        std::string attach_artifact(const std::filesystem::path& artifact, const std::string& name,
                                    const std::string& format = {}) const;
        /** Add one observed diagnostic. @param name Diagnostic name. @param value Numeric value.
         *  @param category Diagnostic category. @param description Human-readable description.
         *  @return Stable result identifier. */
        std::string add_diagnostic(const std::string& name, double value, const std::string& category,
                                   const std::string& description = {}) const;
        /**
         * List registered results in archive order.
         */
        std::vector<ExperimentResult> results() const;
        /**
         * Seal the result inventory and its checksums.
         */
        void finalize_results() const;
        /**
         * Check archive structure, hashes, and external artifact availability.
         */
        ExperimentVerification verify() const;
        /**
         * Read the current execution status and command record.
         */
        ExperimentOutcome outcome() const;
        /**
         * Return the archive path used by this handle.
         */
        const std::filesystem::path& path() const { return file_; }

      private:
        explicit Experiment(std::filesystem::path file) : file_(std::move(file)) {}
        std::filesystem::path file_; ///< Archive operated on by this handle.
    };
} // namespace snt::dip

#endif
