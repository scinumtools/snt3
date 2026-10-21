/** @file
 * @brief Hub command dispatch and CLI failures.
 */
#ifndef SNT_APP_HUB_H
#define SNT_APP_HUB_H

#include <stdexcept>
#include <string>

namespace snt::hub {

    /** Hub command failure carrying the intended CLI exit status. */
    class Error : public std::runtime_error {
        int code_; ///< Process exit status reported by the Hub dispatcher.

      public:
        /** Construct an actionable command failure; usage and input errors default to exit status two. */
        explicit Error(std::string message, int code = 2) : std::runtime_error(std::move(message)), code_(code) {}
        /** Return the CLI exit status associated with this failure. */
        int code() const noexcept { return code_; }
    };

    /** Dispatch Hub arguments and return the command exit status. */
    int command(int argc, char* argv[]);

} // namespace snt::hub

#endif
