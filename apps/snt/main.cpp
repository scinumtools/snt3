#include "main.h"
#include "argparser.h"
#include "puq/main.h"
#include "dip/main.h"

#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>

using namespace snt;

std::string help() {
  return R"(
Scientific Numerical Tools v3 (SNT)

Usage:
  snt <module> <command> [options] [arguments]

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

  cli::ArgParser argpar(argc, argv);
  
  if (!argpar.hasPositionals() && (argpar.has("-h") || !argpar.hasArguments())) {
    std::cout << help();
    exit(0);
  } else if (argpar.has("-v")) {
    std::cout << CODE_VERSION << '\n';
    exit(0);
  }

  try {
    const std::vector<std::string>& pos = argpar.positionals();
    if (!pos.empty()) {
      if (pos[0]=="puq") {
	cli::puq_main(argpar);
      } else if (pos[0]=="dip") {
	cli::dip_main(argpar);
      }
    }
  } catch (std::exception& e) {
    std::cout << e.what() << '\n';
  }

}
