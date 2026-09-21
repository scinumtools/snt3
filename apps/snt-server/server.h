#ifndef SNT_SERVER_H
#define SNT_SERVER_H

#include "httplib.h"

#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace snt::server {

    enum class PublishedInputKind { Project, DIPH5 };

    struct PublishedInput {
        std::string name;
        std::filesystem::path file;
        PublishedInputKind kind;
    };

    std::string required_param(const httplib::Request& request, const std::string& name);
    bool optional_flag(const httplib::Request& request, const std::string& name);
    std::vector<std::string> split_tags(std::string_view value);
    void handle_response(httplib::Response& response, const std::function<void()>& handler);
    void handle_json(httplib::Response& response, const std::function<std::string()>& handler);

    void register_puq_routes(httplib::Server& server);
    void register_dip_routes(httplib::Server& server);
    void register_published_routes(httplib::Server& server, const std::vector<PublishedInput>& inputs);
    void register_openapi_route(httplib::Server& server);
    int run(std::string_view address, int port, const std::vector<PublishedInput>& inputs = {});

} // namespace snt::server

#endif // SNT_SERVER_H
