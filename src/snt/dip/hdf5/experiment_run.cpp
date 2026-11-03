#include <snt/dip/experiment.h>

#include <exception>

namespace snt::dip {

    core::ProcessResult Experiment::run_from_archive(const std::filesystem::path& environment_file,
                                                       const std::filesystem::path& experiment_file,
                                                       ExperimentCommand command) {
        auto experiment = prepare_from_archive(environment_file, experiment_file);
        command.working_directory = std::filesystem::current_path();
        command.capture_origin = "launcher_observed";
        experiment.start(command);
        try {
            const auto result = core::process(command.argv, false);
            experiment.finish(result.status == 0 ? ExperimentStatus::Completed : ExperimentStatus::Failed,
                              result.termination_signal ? std::optional<int>{} : std::optional<int>{result.status},
                              {}, result.termination_signal ? std::optional<int>{result.termination_signal}
                                                           : std::optional<int>{});
            return result;
        } catch (const std::exception& error) {
            experiment.finish(ExperimentStatus::Failed, {}, error.what());
            throw;
        }
    }

} // namespace snt::dip
