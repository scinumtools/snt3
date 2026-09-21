#include "server.h"
#include "settings.h"

#include <charconv>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

    int parse_port(const std::string_view value) {
        int port = 0;
        const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), port);
        if (error != std::errc{} || end != value.data() + value.size() || port < 1 || port > 65535)
            throw std::invalid_argument("Port must be an integer from 1 through 65535.");
        return port;
    }

    void print_usage(const char* executable) {
        std::cout << "Usage: " << executable
                  << " [--address ADDRESS] [--port PORT] [--project NAME=PATH] [--diph5 NAME=PATH]\n";
    }

    snt::server::PublishedInput parse_published_input(
        const std::string_view value, const snt::server::PublishedInputKind kind
    ) {
        const size_t separator = value.find('=');
        if (separator == std::string_view::npos || separator == 0 || separator + 1 == value.size())
            throw std::invalid_argument("Published inputs must use NAME=PATH.");
        const std::string name(value.substr(0, separator));
        for (const unsigned char character : name) {
            if (!std::isalnum(character) && character != '_' && character != '-')
                throw std::invalid_argument("Published environment names use letters, digits, hyphens, and underscores only.");
        }
        return {name, std::string(value.substr(separator + 1)), kind};
    }

} // namespace

int main(const int argc, char* argv[]) {
    int port = SERVER_PORT;
    std::string address(SERVER_ADDRESS);
    std::vector<snt::server::PublishedInput> published_inputs;
    std::set<std::string> published_names;
    for (int argument = 1; argument < argc; ++argument) {
        const std::string_view option(argv[argument]);
        if (option == "--help" || option == "-h") {
            print_usage(argv[0]);
            return 0;
        }
        if (option == "--port") {
            if (++argument == argc) {
                std::cerr << "Missing value for --port.\n";
                print_usage(argv[0]);
                return 2;
            }
            try {
                port = parse_port(argv[argument]);
            } catch (const std::invalid_argument& exception) {
                std::cerr << exception.what() << '\n';
                return 2;
            }
            continue;
        }
        if (option == "--address") {
            if (++argument == argc || std::string_view(argv[argument]).empty()) {
                std::cerr << "Missing value for --address.\n";
                print_usage(argv[0]);
                return 2;
            }
            address = argv[argument];
            continue;
        }
        if (option == "--project" || option == "--diph5") {
            if (++argument == argc) {
                std::cerr << "Missing value for " << option << ".\n";
                print_usage(argv[0]);
                return 2;
            }
            try {
                const auto kind = option == "--project" ? snt::server::PublishedInputKind::Project
                                                          : snt::server::PublishedInputKind::DIPH5;
                auto input = parse_published_input(argv[argument], kind);
                if (!published_names.insert(input.name).second)
                    throw std::invalid_argument("Each published environment name may be configured only once: " + input.name);
                published_inputs.emplace_back(std::move(input));
            } catch (const std::invalid_argument& exception) {
                std::cerr << exception.what() << '\n';
                return 2;
            }
            continue;
        }
        std::cerr << "Unknown option: " << option << '\n';
        print_usage(argv[0]);
        return 2;
    }

    return snt::server::run(address, port, published_inputs);
}
