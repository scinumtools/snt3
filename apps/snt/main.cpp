#include "main.h"

#include "argparser.h"
#include "help.h"

#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

std::string help() {
    std::ostringstream output;
    output << R"(
Scientific Numerical Tools v3 (SNT)

Usage:
  snt [<module> [<command>]] [options [arguments]]

Description:
  Work with physical units and dimensional input parameters.

Options:
  -h, --help
      Show help.
  -v, --version
      Show version information.

Modules:
  Command     Built  Description
)";
    const auto module = [&output](const char* name, bool built, const char* description) {
        output << "  " << std::left << std::setw(12) << name << (built ? "+      " : "-      ")
               << description << '\n';
    };
    module("puq", true, "Physical Units & Quantities");
    module("dip", true, "Dimensional Input Parameters");
#ifdef SNT_ENABLE_REPORT
    module("report", true, "Generate a DIP report");
#else
    module("report", false, "Generate a DIP report");
#endif
#ifdef ENABLE_SNT_DMAP
    module("dmap", true, "Regenerate PUQ dimension-map headers (developer tool)");
#else
    module("dmap", false, "Regenerate PUQ dimension-map headers (developer tool)");
#endif
#ifdef ENABLE_SNT_SERVER
    module("server", true, "REST API server");
#else
    module("server", false, "REST API server");
#endif
#ifdef ENABLE_SNT_VIEW
    module("view", true, "Graphical parameter browser and editor");
#else
    module("view", false, "Graphical parameter browser and editor");
#endif
    module("hub", true, "Install and prepare pinned code examples");
#ifdef ENABLE_SNT_MCP
    module("mcp", true, "Expose DIPL and PUQ tools through Model Context Protocol");
#else
    module("mcp", false, "Expose DIPL and PUQ tools through Model Context Protocol");
#endif
    module("completion", true, "Generate Bash and Zsh completion scripts");
    output << R"(
Notes:
  + included in this build; - unavailable in this build.
Run 'snt <module> --help' for module-specific commands.
)";
    return snt::cli::format_help(output.str());
}

static int run_main(int argc, char* argv[]) {

    if (argc > 1 && std::string(argv[1]) == "completion")
        return module_completion(argc - 1, argv + 1);

    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "format") {
        try {
            return module_fmt(argc - 1, argv + 1);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }

    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "validate") {
        try {
            return module_validate(argc - 1, argv + 1);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }

    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "lint") {
        try {
            return module_lint(argc - 1, argv + 1);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }

    if (argc > 1 && std::string(argv[1]) == "mcp") {
#ifdef ENABLE_SNT_MCP
        return module_mcp(argc, argv);
#else
        std::cerr << "MCP support is not included in this build. Configure with ENABLE_SNT_MCP=ON.\n";
        return 1;
#endif
    }

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
    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "init") {
        try {
            return module_init(argc - 1, argv + 1);
        } catch (const std::exception& error) {
            std::cerr << "snt dip init: " << error.what() << '\n';
            return 2;
        }
    }
    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "setup") {
        try {
            return module_setup(argc - 1, argv + 1);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }
    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "compare") {
        try {
            return module_dip_compare(argc - 2, argv + 2);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }
    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "package") {
        try {
            return module_dip_package(argc - 2, argv + 2);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }
    if (argc > 2 && std::string(argv[1]) == "dip" && std::string(argv[2]) == "experiment") {
        try {
            return module_dip_experiment(argc, argv);
        } catch (const std::exception& error) {
            std::cerr << "snt dip experiment: " << error.what() << '\n';
            return 2;
        }
    }
    if (argc > 2 && std::string(argv[1]) == "dip" &&
        (std::string(argv[2]) == "describe" || std::string(argv[2]) == "override-contract" ||
         std::string(argv[2]) == "sources" || std::string(argv[2]) == "schemas" || std::string(argv[2]) == "list" ||
         std::string(argv[2]) == "preview" || std::string(argv[2]) == "records" ||
         std::string(argv[2]) == "entries")) {
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
                throw std::runtime_error(
                    "Report support is not included in this build. Configure with ENABLE_SNT_REPORT=ON."
                );
#endif
            } else {
                throw std::runtime_error("Unknown module: " + mod + ". Use snt --help.");
            }
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}

#ifdef _WIN32
int wmain(int argc, wchar_t* wide_argv[]) {
    std::vector<std::string> arguments;
    arguments.reserve(static_cast<std::size_t>(argc));
    for (int index = 0; index < argc; ++index) {
        // Keep DIPL text as UTF-8 before the Windows code page can touch it.
        const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide_argv[index], -1,
                                            nullptr, 0, nullptr, nullptr);
        if (size == 0) {
            std::cerr << "Unable to decode a command-line argument as UTF-8.\n";
            return 2;
        }
        std::string value(static_cast<std::size_t>(size), '\0');
        if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, wide_argv[index], -1,
                                value.data(), size, nullptr, nullptr) == 0) {
            std::cerr << "Unable to decode a command-line argument as UTF-8.\n";
            return 2;
        }
        value.resize(static_cast<std::size_t>(size - 1));
        arguments.push_back(std::move(value));
    }
    std::vector<char*> argv;
    argv.reserve(arguments.size());
    for (auto& argument : arguments)
        argv.push_back(argument.data());
    return run_main(argc, argv.data());
}
#else
int main(int argc, char* argv[]) {
    return run_main(argc, argv);
}
#endif
