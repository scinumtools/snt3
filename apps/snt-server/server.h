/** @file
 * @brief REST route registration, input publication, and response helpers.
 */
#ifndef SNT_SERVER_H
#define SNT_SERVER_H

#include "httplib.h"

#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace snt::server {

    /** Distinguish source projects from evaluated DIPH5 snapshots. */
    enum class PublishedInputKind {
        Project, ///< DIPL source project evaluated when requested.
        DIPH5    ///< Saved evaluated environment.
    };

    /** Named input made available through the REST publication routes. */
    struct PublishedInput {
        std::string name;           ///< Public identifier used in requests.
        std::filesystem::path file; ///< Configured project or snapshot path.
        PublishedInputKind kind;    ///< Input representation and supported operations.
    };

    /** Read a required query parameter; throw if it is absent or empty. */
    std::string required_param(const httplib::Request& request, const std::string& name);
    /** Read an optional boolean query flag and reject unsupported spellings. */
    bool optional_flag(const httplib::Request& request, const std::string& name);
    /** Split a comma-separated tag filter into individual tags. */
    std::vector<std::string> split_tags(std::string_view value);
    /** Escape text for inclusion inside a JSON string. */
    std::string json_escape(std::string_view value);
    /** Run a text response handler with serialized API access and translate exceptions into HTTP errors. */
    void handle_response(httplib::Response& response, const std::function<void()>& handler);
    /** Run a JSON producer with serialized API access and translate exceptions into HTTP errors. */
    void handle_json(httplib::Response& response, const std::function<std::string()>& handler);

    /** Register quantity evaluation, conversion, inspection, and unit-listing endpoints. */
    void register_puq_routes(httplib::Server& server);
    /** Register DIPL source, semantic inspection, export, and comparison endpoints. */
    void register_dip_routes(httplib::Server& server);
    /** Register discovery and operations for the supplied named inputs. */
    void register_published_routes(httplib::Server& server, const std::vector<PublishedInput>& inputs);
    /** Expose the REST OpenAPI description. */
    void register_openapi_route(httplib::Server& server);
    /** Serve the REST API at the requested address and port, returning a process exit status. */
    int run(std::string_view address, int port, const std::vector<PublishedInput>& inputs = {});

} // namespace snt::server

#endif // SNT_SERVER_H
