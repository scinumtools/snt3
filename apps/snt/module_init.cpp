#include "main.h"
#include "help.h"

#include <filesystem>
#include <iostream>
#include <snt/dip/project_init.h>
#include <string>

namespace {
    void print_help() {
        std::cout << snt::cli::format_help(R"(Scientific Numerical Tools v3 (SNT)
Command: init

Usage:
  snt init

Description:
  Initialize a DIP project in the current directory. Create a DIPfile that
  loads main.dip, and a main.dip file with an editable example parameter.
  Existing files are never replaced.

Options:
  -h, --help    Show this help.

Examples:
  mkdir my-project
  cd my-project
  snt init
  snt dip parse --project DIPfile --print
)");
    }

} // namespace

int module_init(int argc, char* argv[]) {
    if (argc == 3 && (std::string(argv[2]) == "--help" || std::string(argv[2]) == "-h")) {
        print_help();
        return 0;
    }
    if (argc != 2) {
        std::cerr << "Usage: snt init\n";
        return 2;
    }

    const auto directory = std::filesystem::current_path();
    snt::dip::initialize_project(directory);
    std::cout << "Initialized DIP project in " << directory.string() << '\n'
              << "  DIPfile\n"
              << "  main.dip\n";
    return 0;
}
