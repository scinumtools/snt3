/** @file
 * @brief Shared, dependency-free HTTP access guard for the REST and MCP frontends.
 */
#ifndef SNT_HTTP_SECURITY_H
#define SNT_HTTP_SECURITY_H

#include <httplib.h>

#include <string>
#include <string_view>
#include <vector>

namespace snt::http {
    /** Protect one HTTP listener with loopback-only local access or one service bearer token. */
    class Security {
      public:
        /** Read an optional token file and reject non-loopback binds without a token. */
        Security(std::string_view address, int port, const std::string& token_file = {});
        /** Return true after writing a rejection response, before any route is dispatched. */
        bool reject(
            const httplib::Request& request,
            httplib::Response& response,
            const std::vector<std::string>& allowed_origins = {}
        ) const;

      private:
        std::string token_; ///< Fixed-length service token; empty means loopback-only local mode.
        int port_;          ///< Listener port used for local Host and Origin checks.
    };
} // namespace snt::http

#endif // SNT_HTTP_SECURITY_H
