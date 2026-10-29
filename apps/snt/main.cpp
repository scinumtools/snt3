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
    std::ostringstream output;
    output << R"(
Scientific Numerical Tools v3 (SNT)

Usage:
  snt [<module> [<command>]] [options [arguments]]

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
    module("setup", true, "Create a validated DIPL override through guided prompts");
#ifdef ENABLE_SNT_MCP
    module("mcp", true, "Expose DIPL and PUQ tools through Model Context Protocol");
#else
    module("mcp", false, "Expose DIPL and PUQ tools through Model Context Protocol");
#endif
    module("format", true, "Format DIPL source files");
    module("validate", true, "Validate DIPL files and DIPfile projects");
    module("lint", true, "Check evaluated DIPL for advisory issues");
    module("completion", true, "Generate Bash and Zsh completion scripts");
    output << R"(
  + included in this build; - unavailable in this build.
Run 'snt <module> --help' for module-specific commands.
)";
    return output.str();
}

int main(int argc, char* argv[]) {

    if (argc > 1 && std::string(argv[1]) == "completion")
        return module_completion(argc - 1, argv + 1);

    if (argc > 1 && (std::string(argv[1]) == "fmt" || std::string(argv[1]) == "format")) {
        try {
            return module_fmt(argc, argv);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }

    if (argc > 1 && std::string(argv[1]) == "validate") {
        try {
            return module_validate(argc, argv);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 2;
        }
    }

    if (argc > 1 && std::string(argv[1]) == "lint") {
        try {
            return module_lint(argc, argv);
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
    if (argc > 1 && std::string(argv[1]) == "setup") {
        try {
            return module_setup(argc, argv);
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
    if (argc > 2 && std::string(argv[1]) == "dip" &&
        (std::string(argv[2]) == "describe" || std::string(argv[2]) == "override-contract" ||
         std::string(argv[2]) == "sources" || std::string(argv[2]) == "schemas" || std::string(argv[2]) == "list" ||
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
}
