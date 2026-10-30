#include "../snt/main.h"
#include "../snt/help.h"

#include "mcp.h"

#include <charconv>
#include <iostream>
#include <regex>
#include <stdexcept>

int module_mcp(int argc, char* argv[]) {
    using namespace snt::mcp;
    try {
        if (argc == 4 && std::string(argv[2]) == "--worker")
            return worker_main(argv[3]);
        Projects projects;
        std::string transport = "stdio", address = "127.0.0.1", auth_token_file;
        int port = 8081;
        unsigned timeout_ms = 30000;
        std::vector<std::string> origins;
        for (int i = 2; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--help" || option == "-h") {
                snt::cli::HelpPage page;
                page.module = "Model Context Protocol (MCP) server";
                page.usage = "snt mcp [options]";
                page.description = "Native C++ Model Context Protocol server using the SNT APIs directly.";
                page.options = R"(-h, --help                 Show this help.
--input PATH               Publish a DIPfile, .dip/.dipl or .diph5 as "default"
--project NAME=PATH        Publish a named input (repeatable)
--transport stdio|streamable-http
                           Transport (default: stdio)
--address ADDRESS          HTTP bind address (default: 127.0.0.1)
--port PORT                HTTP port (default: 8081); endpoint /mcp
--allow-origin ORIGIN      Allow an additional exact HTTP Origin (repeatable)
--auth-token-file PATH     Require a 64-character hex bearer token from an owner-only file
--timeout-ms MILLISECONDS  Tool execution deadline (default: 30000; maximum: 3600000))";
                page.examples = R"(snt mcp --input model/DIPfile
snt mcp --transport streamable-http --project model=model/DIPfile)";
                page.notes = R"(Evaluation uses supervised native C++ workers. No Python or MCP SDK is required.
Generated artifacts expire after one hour or shutdown; storage is limited to 256 MiB.
Non-loopback HTTP requires a token file and an HTTPS gateway.)";
                std::cout << snt::cli::render_help(page);
                return 0;
            }
            if (++i == argc)
                throw std::invalid_argument("Missing value for " + option);
            const std::string value = argv[i];
            if (option == "--input" || option == "--project") {
                const auto separator = value.find('=');
                const std::string name = option == "--input" ? "default" : value.substr(0, separator);
                const std::string file = option == "--input"              ? value
                                         : separator == std::string::npos ? ""
                                                                          : value.substr(separator + 1);
                if (!std::regex_match(name, std::regex("[A-Za-z0-9_-]+")) || file.empty())
                    throw std::invalid_argument("Expected --project NAME=PATH or --input PATH.");
                if (!std::filesystem::is_regular_file(file))
                    throw std::invalid_argument("Input does not exist: " + file);
                if (!projects.emplace(name, std::filesystem::canonical(file)).second)
                    throw std::invalid_argument("Duplicate project name: " + name);
            } else if (option == "--transport") {
                if (value != "stdio" && value != "streamable-http")
                    throw std::invalid_argument("Transport must be stdio or streamable-http.");
                transport = value;
            } else if (option == "--address") {
                if (value.empty())
                    throw std::invalid_argument("Address must not be empty.");
                address = value;
            } else if (option == "--port") {
                const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), port);
                if (error != std::errc{} || end != value.data() + value.size() || port < 1 || port > 65535)
                    throw std::invalid_argument("Port must be between 1 and 65535.");
            } else if (option == "--allow-origin") {
                if (!std::regex_match(value, std::regex("https?://[^/?#[:space:]]+")))
                    throw std::invalid_argument("Origin must be an exact http(s)://host[:port] origin.");
                origins.push_back(value);
            } else if (option == "--auth-token-file") {
                if (value.empty())
                    throw std::invalid_argument("HTTP token file path must not be empty.");
                auth_token_file = value;
            } else if (option == "--timeout-ms") {
                const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), timeout_ms);
                if (error != std::errc{} || end != value.data() + value.size() || timeout_ms < 1 ||
                    timeout_ms > 3600000)
                    throw std::invalid_argument("Timeout must be between 1 and 3600000 milliseconds.");
            } else {
                throw std::invalid_argument("Unknown option: " + option);
            }
        }
        Protocol protocol(std::move(projects), timeout_ms);
        if (transport == "stdio") {
            if (!auth_token_file.empty())
                throw std::invalid_argument("--auth-token-file requires streamable-http transport.");
            return run_stdio(protocol);
        }
        for (const auto* host : {"127.0.0.1", "localhost", "[::1]"})
            origins.push_back("http://" + std::string(host) + ":" + std::to_string(port));
        return run_http(protocol, address, port, origins, auth_token_file);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }
}
