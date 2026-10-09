#include "main.h"

#include "argparser.h"

#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>

std::string help() {
    return R"(
Scientific Numerical Tools v3 (SNT)

Usage:
  snt [<module> [<command>]] [options [arguments]]

Options:
  -h, --help
      Show help.
  -v, --version
      Show version information.

Modules:
  dip     Dimensional Input Parameters
  report  Generate a DIP report (optional build feature)
  puq     Physical Units & Quantities
  dmap    Regenerate PUQ dimension-map headers (developer tool; optional)
  server  REST API server (optional build feature)
  view    Read-only parameter browser (optional build feature)
  hub     Install and prepare pinned code examples

Run 'snt <module> --help' for module-specific commands.
)";
}

int main(int argc, char* argv[]) {

    // Server arguments have their own parser; pass through without reinterpreting them.
    if (argc > 1 && std::string(argv[1]) == "server") {
#ifdef ENABLE_SNT_SERVER
        return module_server(argc - 1, argv + 1);
#else
        std::cerr << "Server support is not included in this build. Configure with ENABLE_SNT_SERVER=ON.\n";
        return 1;
#endif
    }
    if (argc > 1 && std::string(argv[1]) == "dmap") {
#ifdef ENABLE_SNT_DMAP
        return module_dmap(argc - 1, argv + 1);
#else
        std::cerr << "Dimension map support is not included in this build. Configure with ENABLE_SNT_DMAP=ON.\n";
        return 1;
#endif
    }
    if (argc > 1 && std::string(argv[1]) == "view") {
#ifdef ENABLE_SNT_VIEW
        return module_view(argc - 1, argv + 1);
#else
        std::cerr << "Viewer support is not included in this build. Configure with ENABLE_SNT_VIEW=ON.\n";
        return 1;
#endif
    }
    if (argc > 1 && std::string(argv[1]) == "hub") {
        return module_hub(argc, argv);
    }
    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "compare") {
        try {
            return module_dip_compare(argc - 2, argv + 2);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }
    if (argc > 2 && std::string(argv[1]) == "dip" &&
        (std::string(argv[2]) == "describe" || std::string(argv[2]) == "override-contract" || std::string(argv[2]) == "list" ||
         std::string(argv[2]) == "preview")) {
        try {
            return module_dip_semantic(argc - 2, argv + 2);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }

    ArgParser argpar(argc, argv);

    if (!argpar.hasPositionals() && (argpar.hasKeyword("-h") || argpar.hasKeyword("--help") || !argpar.hasKeywords())) {
        std::cout << help();
        exit(0);
    } else if (argpar.hasKeyword("-v") || argpar.hasKeyword("--version")) {
        std::cout << CODE_VERSION << '\n';
        exit(0);
    }

    try {
        if (argpar.hasPositionals()) {
            std::string mod = argpar.getPositionalValue(0);
            if (mod == "puq") {
                module_puq(argpar);
            } else if (mod == "dip") {
                module_dip(argpar);
            } else if (mod == "report") {
#ifdef SNT_ENABLE_REPORT
                module_report(argpar);
#else
                throw std::runtime_error("Report support is not included in this build. Configure with ENABLE_SNT_REPORT=ON.");
#endif
            } else {
                throw std::runtime_error("Unknown module: " + mod + ". Use snt --help.");
            }
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
