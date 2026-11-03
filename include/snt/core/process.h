#ifndef SNT_CORE_PROCESS_H
#define SNT_CORE_PROCESS_H

#include <string>
#include <vector>

namespace snt::core {

    /** Result of launching one native child process. */
    struct ProcessResult {
        int status = 0;             ///< Exit code, or 128 plus the terminating signal on POSIX.
        std::string output;          ///< Tail of combined output when capture is requested.
        int termination_signal = 0; ///< POSIX signal number, if applicable.
    };

    /** Run an argument vector directly, without shell expansion. */
    ProcessResult process(const std::vector<std::string>& args, bool capture = true);

} // namespace snt::core

#endif
