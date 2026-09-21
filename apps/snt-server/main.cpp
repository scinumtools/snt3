#include "server.h"
#include "settings.h"

#include <charconv>
#include <iostream>
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
        std::cout << "Usage: " << executable << " [--address ADDRESS] [--port PORT]\n";
    }

} // namespace

int main(const int argc, char* argv[]) {
    int port = SERVER_PORT;
    std::string address(SERVER_ADDRESS);
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
        std::cerr << "Unknown option: " << option << '\n';
        print_usage(argv[0]);
        return 2;
    }

    return snt::server::run(address, port);
}
