#include "mcp.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <httplib.h>
#include <iostream>
#include <limits>
#include <mutex>
#include <sstream>
#include <thread>

namespace snt::mcp {
    int run_stdio(Protocol& protocol) {
        std::string line;
        bool oversized = false;
        char character;
        std::thread pending;
        std::atomic<bool> running{false};
        std::mutex output;
        auto respond = [&](const Reply& reply) {
            if (reply.body) {
                std::lock_guard<std::mutex> lock(output);
                std::cout << reply.body->dump() << '\n' << std::flush;
            }
        };
        while (std::cin.get(character)) {
            if (character != '\n') {
                if (line.size() < message_limit)
                    line += character;
                else
                    oversized = true;
                continue;
            }
            bool tool_call = false;
            if (!oversized) {
                try {
                    const auto request = Json::parse(
                        line,
                        [](int depth, Json::parse_event_t, Json&) {
                            if (depth > 64)
                                throw std::invalid_argument("JSON nesting limit");
                            return true;
                        },
                        false
                    );
                    tool_call = request.is_object() && request.value("method", Json{}) == "tools/call";
                } catch (const std::exception&) {
                }
            }
            if (pending.joinable() && !running)
                pending.join();
            if (tool_call && !running) {
                if (pending.joinable())
                    pending.join();
                running = true;
                pending = std::thread([&, body = line] {
                    respond(protocol.handle(body));
                    running = false;
                });
                // Register the request before reading a possible cancellation notification.
                while (running && !protocol.busy())
                    std::this_thread::yield();
            } else {
                respond(
                    oversized ? Reply{Protocol::error(nullptr, -32600, "Message exceeds 4 MiB."), 413}
                              : protocol.handle(line)
                );
            }
            line.clear();
            oversized = false;
        }
        if (pending.joinable())
            pending.join();
        return std::cin.bad() || !std::cout ? 1 : 0;
    }

    namespace {
        bool media_type(const std::string& header, const std::string& expected) {
            std::istringstream stream(header);
            std::string part;
            while (std::getline(stream, part, ',')) {
                part = part.substr(0, part.find(';'));
                part.erase(
                    std::remove_if(part.begin(), part.end(), [](unsigned char c) { return std::isspace(c); }),
                    part.end()
                );
                std::transform(part.begin(), part.end(), part.begin(), [](unsigned char c) { return std::tolower(c); });
                if (part == expected)
                    return true;
            }
            return false;
        }
    } // namespace

    int run_http(
        Protocol& protocol, const std::string& address, int port, const std::vector<std::string>& allowed_origins
    ) {
        httplib::Server server;
        server.set_payload_max_length(message_limit);
        server.set_read_timeout(30);
        server.set_write_timeout(30);
        server.set_pre_routing_handler([&](const httplib::Request& request, httplib::Response& response) {
            if (request.path == "/mcp" && request.has_header("Origin")) {
                const auto origin = request.get_header_value("Origin");
                if (request.get_header_value_count("Origin") != 1 ||
                    std::find(allowed_origins.begin(), allowed_origins.end(), origin) == allowed_origins.end()) {
                    response.status = 403;
                    response.set_content("Origin is not allowed.", "text/plain");
                    return httplib::Server::HandlerResponse::Handled;
                }
            }
            return httplib::Server::HandlerResponse::Unhandled;
        });
        server.Post("/mcp", [&](const httplib::Request& request, httplib::Response& response) {
            if (!media_type(request.get_header_value("Content-Type"), "application/json")) {
                response.status = 415;
                return;
            }
            const auto accept = request.get_header_value("Accept");
            if (!media_type(accept, "application/json") || !media_type(accept, "text/event-stream")) {
                response.status = 406;
                return;
            }
            std::map<std::string, std::string> headers;
            for (const auto* name : {"mcp-protocol-version", "mcp-method", "mcp-name"}) {
                if (request.get_header_value_count(name) > 1) {
                    response.status = 400;
                    response.set_content(
                        Protocol::error(nullptr, -32020, "Duplicate MCP routing header.").dump(), "application/json"
                    );
                    return;
                }
                headers[name] = request.get_header_value(name);
            }
            const auto result = protocol.handle(request.body, true, headers, request.is_connection_closed);
            response.status = result.status;
            if (result.body)
                response.set_content(result.body->dump(), "application/json");
        });
        const auto unsupported = [](const httplib::Request&, httplib::Response& response) {
            response.status = 405;
            response.set_header("Allow", "POST");
        };
        server.Get("/mcp", unsupported);
        server.Delete("/mcp", unsupported);
        std::cerr << "SNT MCP listening on http://" << address << ':' << port << "/mcp\n";
        if (!server.listen(address, port)) {
            std::cerr << "Cannot listen on " << address << ':' << port << ".\n";
            return 1;
        }
        return 0;
    }
} // namespace snt::mcp
