#ifndef SNT_APP_EXPERIMENT_COMMANDS_H
#define SNT_APP_EXPERIMENT_COMMANDS_H

#include <array>

namespace snt::cli {

    struct ExperimentCommandHelp {
        const char* name;
        const char* options;
    };

    // One command list feeds shell completion and command recognition.
    inline constexpr std::array<ExperimentCommandHelp, 12> experiment_commands{{
        {"prepare", "--input --output --help"},
        {"run", "--input --output --role --help"},
        {"start", "--file --origin --role --cwd --executable-path --executable-sha256 --executable-size --help"},
        {"finish", "--file --status --exit-code --termination-signal --help"},
        {"import", "--input --output --status --exit-code --termination-signal --help"},
        {"show", "--file --help"},
        {"add", "--file --name --values --dtype --shape --unit --dimensions --description --help"},
        {"attach", "--file --name --artifact --format --help"},
        {"diagnostic", "--file --name --values --category --description --help"},
        {"results", "--file --help"},
        {"finalize", "--file --help"},
        {"verify", "--file --help"},
    }};

} // namespace snt::cli

#endif
