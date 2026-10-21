/** @file
 * @brief Native child-process invocation for Hub operations.
 */
#ifndef SNT_APP_HUB_PROCESS_H
#define SNT_APP_HUB_PROCESS_H

#include <string>
#include <vector>

namespace snt::hub {

    /** Exit status and optional captured output from a child process. */
    struct ProcessResult {
        int status = 0;     ///< Child exit status.
        std::string output; ///< Last 256 KiB of combined stdout/stderr, empty when capture is disabled.
    };

    /** Run an argument vector without shell expansion, optionally capturing process output. */
    ProcessResult process(const std::vector<std::string>& args, bool capture = true);

} // namespace snt::hub

#endif
