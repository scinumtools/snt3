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
  puq     Physical Units & Quantities

Run 'snt <module> --help' for module-specific commands.
)";
}

int main(int argc, char* argv[]) {

    ArgParser argpar(argc, argv);

    if (!argpar.hasPositionals() && (argpar.hasKeyword("-h") || !argpar.hasKeywords())) {
        std::cout << help();
        exit(0);
    } else if (argpar.hasKeyword("-v")) {
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
            }
        }
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}
