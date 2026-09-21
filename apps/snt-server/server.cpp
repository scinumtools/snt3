#include "server.h"

#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>

namespace snt::server {

    namespace {

        std::mutex api_mutex;

        std::string json_escape(const std::string_view value) {
            std::string escaped;
            escaped.reserve(value.size());
            for (const unsigned char ch : value) {
                switch (ch) {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\b': escaped += "\\b"; break;
                case '\f': escaped += "\\f"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default:
                    if (ch < 0x20) {
                        constexpr char hex[] = "0123456789abcdef";
                        escaped += "\\u00";
                        escaped += hex[(ch >> 4) & 0x0f];
                        escaped += hex[ch & 0x0f];
                    } else {
                        escaped += static_cast<char>(ch);
                    }
                }
            }
            return escaped;
        }

        std::string json_result(const std::string_view value) {
            return "{\"result\":\"" + json_escape(value) + "\"}\n";
        }

        std::string json_error(const std::string_view value) {
            return "{\"error\":\"" + json_escape(value) + "\"}\n";
        }

    } // namespace

    std::string required_param(const httplib::Request& request, const std::string& name) {
        if (!request.has_param(name) || request.get_param_value(name).empty())
            throw std::invalid_argument("Missing required query parameter: " + name);
        return request.get_param_value(name);
    }

    bool optional_flag(const httplib::Request& request, const std::string& name) {
        if (!request.has_param(name))
            return false;
        const auto value = request.get_param_value(name);
        if (value == "true" || value == "1")
            return true;
        if (value == "false" || value == "0")
            return false;
        throw std::invalid_argument("The " + name + " query parameter must be true or false.");
    }

    std::vector<std::string> split_tags(const std::string_view value) {
        std::vector<std::string> tags;
        size_t begin = 0;
        while (begin <= value.size()) {
            const size_t end = value.find(',', begin);
            const auto tag = value.substr(begin, end == std::string_view::npos ? value.size() - begin : end - begin);
            if (tag.empty())
                throw std::invalid_argument("The tags query parameter must not contain empty tags.");
            tags.emplace_back(tag);
            if (end == std::string_view::npos)
                break;
            begin = end + 1;
        }
        return tags;
    }

    void handle_response(httplib::Response& response, const std::function<void()>& handler) {
        try {
            std::lock_guard<std::mutex> lock(api_mutex);
            handler();
        } catch (const std::exception& exception) {
            response.status = 400;
            response.set_content(json_error(exception.what()), "application/json");
        }
    }

    void handle_json(httplib::Response& response, const std::function<std::string()>& handler) {
        handle_response(response, [&] { response.set_content(json_result(handler()), "application/json"); });
    }

    int run(const std::string_view address, const int port, const std::vector<PublishedInput>& inputs) {
        try {
            httplib::Server server;
            server.Get("/", [](const httplib::Request&, httplib::Response& response) {
                response.set_content(
                    R"({"service":"SNT REST API","endpoints":["/snt/puq/eval","/snt/puq/convert","/snt/puq/info","/snt/puq/list","/snt/dip/parse","/snt/dip/environments","/snt/dip/environment"]}
)",
                    "application/json"
                );
            });
            register_puq_routes(server);
            register_dip_routes(server);
            register_published_routes(server, inputs);
            register_openapi_route(server);

            std::cout << "Starting SNT REST API server on http://" << address << ':' << port << '\n';
            if (!server.listen(std::string(address), port)) {
                std::cerr << "Unable to start SNT REST API server on port " << port << ".\n";
                return 1;
            }
        } catch (const std::exception& exception) {
            std::cerr << "Unable to load published environment: " << exception.what() << '\n';
            return 1;
        }
        return 0;
    }

} // namespace snt::server
