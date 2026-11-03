#include "io.h"
#include "io_private.h"

#include <snt/dip/environment.h>
#include <snt/dip/experiment.h>
#include <stdexcept>

namespace snt::dip {
    namespace {
        template <typename Prepare> void create_experiment(const std::filesystem::path& file, Prepare prepare) {
            const auto staged = file.parent_path() / (".diph5-experiment-" + hdf5::detail::uuid() + ".h5");
            try {
                prepare(staged);
                hdf5::prepare_experiment(staged);
                // A hard link publishes the fully prepared file atomically without
                // replacing a destination created by another process in the meantime.
                std::error_code error;
                std::filesystem::create_hard_link(staged, file, error);
                if (error) throw std::runtime_error("Unable to publish experiment: " + error.message());
                std::filesystem::remove(staged, error);
            } catch (...) {
                std::error_code ignored;
                std::filesystem::remove(staged, ignored);
                throw;
            }
        }
    }

    Experiment Experiment::prepare(const Environment& environment, const std::filesystem::path& file) {
        if (std::filesystem::exists(file))
            throw std::invalid_argument("Experiment output already exists");
        create_experiment(file, [&](const auto& staged) { environment.save(staged); });
        return Experiment(file);
    }

    Experiment Experiment::prepare_from_archive(const std::filesystem::path& source,
                                                const std::filesystem::path& file) {
        if (std::filesystem::absolute(source).lexically_normal() ==
            std::filesystem::absolute(file).lexically_normal())
            throw std::invalid_argument("The experiment must have a separate output file");
        Environment checked;
        checked.load(source);
        if (checked.archived_sources().empty())
            throw std::invalid_argument("An experiment requires a source-complete DIPH5 v3 archive");
        create_experiment(file, [&](const auto& staged) {
            std::filesystem::copy_file(source, staged, std::filesystem::copy_options::none);
        });
        return Experiment(file);
    }

    Experiment Experiment::open(const std::filesystem::path& file) {
        hdf5::read_experiment(file);
        return Experiment(file);
    }

    void Experiment::start(const ExperimentCommand& command) const { hdf5::start_experiment(file_, command); }

    void Experiment::finish(ExperimentStatus status, std::optional<int> exit_code,
                            const std::string& message, std::optional<int> termination_signal) const {
        hdf5::finish_experiment(file_, status, exit_code, message, false, termination_signal);
    }

    void Experiment::import(ExperimentStatus status, std::optional<int> exit_code,
                            const std::string& message, std::optional<int> termination_signal) const {
        hdf5::finish_experiment(file_, status, exit_code, message, true, termination_signal);
    }

    ExperimentOutcome Experiment::outcome() const { return hdf5::read_experiment(file_); }

    std::string Experiment::add_dataset(const std::string& name, const std::vector<double>& values,
                                        const std::vector<uint64_t>& shape, const std::string& units,
                                        const std::vector<std::string>& dimensions, const std::string& description) const {
        return hdf5::add_result_dataset(file_, name, values, "float64", shape, units, dimensions, description, "datasets");
    }

    std::string Experiment::add_typed_dataset(const std::string& name, const ExperimentValues& values,
                                               const std::string& dtype, const std::vector<uint64_t>& shape,
                                               const std::string& units, const std::vector<std::string>& dimensions,
                                               const std::string& description) const {
        return hdf5::add_result_dataset(file_, name, values, dtype, shape, units, dimensions, description, "datasets");
    }

    std::string Experiment::attach_artifact(const std::filesystem::path& artifact, const std::string& name,
                                             const std::string& format) const {
        return hdf5::attach_result_artifact(file_, artifact, name, format);
    }

    std::string Experiment::add_diagnostic(const std::string& name, double value,
                                            const std::string& category, const std::string& description) const {
        if (category != "observed" && category != "judgment")
            throw std::invalid_argument("Diagnostic category must be observed or judgment");
        return hdf5::add_result_dataset(file_, name, std::vector<double>{value}, "float64", {}, "", {}, description, "diagnostics:" + category);
    }

    std::vector<ExperimentResult> Experiment::results() const { return hdf5::read_results(file_); }
    void Experiment::finalize_results() const { hdf5::finalize_results(file_); }
    ExperimentVerification Experiment::verify() const { return hdf5::verify_experiment(file_); }
} // namespace snt::dip
