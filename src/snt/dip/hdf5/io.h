#ifndef SNT_DIP_HDF5_IO_H
#define SNT_DIP_HDF5_IO_H

#include <filesystem>
#include <snt/dip/experiment.h>
#include <stdexcept>

namespace snt::dip {
    class Environment;
    class OutputPlan;
} // namespace snt::dip

namespace snt::dip::hdf5 {
    class Error : public std::runtime_error {
      public:
        using std::runtime_error::runtime_error;
    };

    void load(Environment& env, const std::filesystem::path& file);
    void save(const Environment& env, const std::filesystem::path& file, const OutputPlan* output_plan = nullptr);
    void prepare_experiment(const std::filesystem::path& file);
    void start_experiment(const std::filesystem::path& file, const ExperimentCommand& command);
    void finish_experiment(const std::filesystem::path& file, ExperimentStatus status,
                           std::optional<int> exit_code, const std::string& message, bool imported,
                           std::optional<int> termination_signal = {});
    ExperimentOutcome read_experiment(const std::filesystem::path& file);
    std::string add_result_dataset(const std::filesystem::path& file, const std::string& name,
                                   const ExperimentValues& values, const std::string& dtype,
                                   const std::vector<uint64_t>& shape,
                                   const std::string& units, const std::vector<std::string>& dimensions,
                                   const std::string& description, const std::string& category);
    std::string attach_result_artifact(const std::filesystem::path& file, const std::filesystem::path& artifact,
                                       const std::string& name, const std::string& format);
    std::vector<ExperimentResult> read_results(const std::filesystem::path& file);
    void finalize_results(const std::filesystem::path& file, bool imported = false);
    ExperimentVerification verify_experiment(const std::filesystem::path& file);

} // namespace snt::dip::hdf5

#endif
