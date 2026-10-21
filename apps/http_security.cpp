#include "http_security.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace snt::http {
    namespace {
        bool loopback_address(const std::string_view address) {
            return address == "127.0.0.1" || address == "localhost" || address == "::1" || address == "[::1]";
        }

        bool local_host(const std::string& host, const int port) {
            for (const auto* name : {"127.0.0.1", "localhost", "[::1]"}) {
                if (host == std::string(name) + ':' + std::to_string(port))
                    return true;
            }
            return false;
        }

        bool local_origin(const std::string& origin, const int port) {
            return origin.size() > 7 && origin.compare(0, 7, "http://") == 0 &&
                   local_host(origin.substr(7), port);
        }

        std::string read_token(const std::string& path) {
            namespace fs = std::filesystem;
            const fs::path file(path);
            if (!fs::is_regular_file(file))
                throw std::invalid_argument("HTTP token file must be a regular file.");
#ifndef _WIN32
            const auto permissions = fs::status(file).permissions();
            if ((permissions & (fs::perms::group_all | fs::perms::others_all)) != fs::perms::none)
                throw std::invalid_argument("HTTP token file must be accessible only to its owner (chmod 600).");
#endif
            if (fs::file_size(file) > 66)
                throw std::invalid_argument("HTTP token file must contain one 64-character hex token.");
            std::ifstream stream(file, std::ios::binary);
            if (!stream)
                throw std::invalid_argument("Cannot read HTTP token file.");
            std::array<char, 67> contents{};
            stream.read(contents.data(), contents.size());
            if (stream.gcount() > 66)
                throw std::invalid_argument("HTTP token file must contain one 64-character hex token.");
            std::string token(contents.data(), static_cast<std::size_t>(stream.gcount()));
            if (!token.empty() && token.back() == '\n')
                token.pop_back();
            if (!token.empty() && token.back() == '\r')
                token.pop_back();
            if (token.size() != 64 || !std::all_of(token.begin(), token.end(), [](unsigned char ch) {
                    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
                }))
                throw std::invalid_argument("HTTP token file must contain one 64-character lowercase hex token.");
            return token;
        }

        bool same_token(const std::string& expected, const std::string& provided) {
            if (provided.size() != expected.size())
                return false;
            volatile unsigned char difference = 0;
            for (std::size_t i = 0; i < expected.size(); ++i)
                difference = static_cast<unsigned char>(difference | (expected[i] ^ provided[i]));
            return difference == 0;
        }

        void deny(httplib::Response& response, const int status, const char* message) {
            response.status = status;
            response.set_content(message, "text/plain");
        }
    } // namespace

    Security::Security(const std::string_view address, const int port, const std::string& token_file)
        : token_(token_file.empty() ? "" : read_token(token_file)), port_(port) {
        if (!loopback_address(address) && token_.empty())
            throw std::invalid_argument("Non-loopback HTTP binding requires --auth-token-file and HTTPS at the gateway.");
    }

    bool Security::reject(
        const httplib::Request& request,
        httplib::Response& response,
        const std::vector<std::string>& allowed_origins
    ) const {
        if (request.get_header_value_count("Host") != 1 || request.get_header_value("Host").empty() ||
            (token_.empty() && !local_host(request.get_header_value("Host"), port_))) {
            deny(response, 403, "Host is not allowed.");
            return true;
        }
        if (request.has_header("Origin")) {
            const auto origin = request.get_header_value("Origin");
            if (request.get_header_value_count("Origin") != 1 ||
                (!local_origin(origin, port_) &&
                 std::find(allowed_origins.begin(), allowed_origins.end(), origin) == allowed_origins.end())) {
                deny(response, 403, "Origin is not allowed.");
                return true;
            }
        }
        if (!token_.empty()) {
            const auto authorization = request.get_header_value("Authorization");
            if (request.get_header_value_count("Authorization") != 1 || authorization.size() != 71 ||
                authorization.compare(0, 7, "Bearer ") != 0 || !same_token(token_, authorization.substr(7))) {
                response.set_header("WWW-Authenticate", "Bearer realm=\"snt\"");
                deny(response, 401, "Bearer token required.");
                return true;
            }
        }
        return false;
    }
} // namespace snt::http
