#include "main.h"

#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <snt/dip/dip.h>
#include <snt/dip/environment.h>
#include <snt/dip/nodes/node_value.h>
#include <sstream>
#include <stdexcept>

namespace snt::cli {

  std::string dip_help() {
    return R"(
Scientific Numerical Tools v3 (SNT)
Module: Dimensional Input Parameters (DIP)

Usage:
  snt dip parse [options]

Description:
  Parse and query dimensional input parameter definitions.

Options:
  -h, --help
      Show help.
  -v, --version
      Show version information.

  --file <filename>
      Load a .dip file (repeatable).

  --string <code>
      Parse inline DIP code (repeatable).

  --request <query>
      Request specific nodes (e.g. "?family.father").

  --print
      Print nodes.

  --print-values
      Print node values.

Examples:
  snt dip parse --file parameters.dip --print

  snt dip parse \
      --file parameters.dip \
      --string "age int = 23 y" \
      --request "?family.father" \
      --print
)";
  }
  
  void dip_main(ArgParser& argpar) {
  
    if (argpar.has("-h") || !argpar.hasArguments()) {
      std::cout << dip_help();
    }
    try {     
      std::vector<std::string> arguments;
      snt::dip::DIP d;
      // TODO: add multiple files, strings, sources and custom units
      
      arguments = argpar.getAll("--file");
      if (!arguments.empty()) {
        d.add_file(arguments[0]);
      }
      arguments = argpar.getAll("--string");
      if (!arguments.empty()) {
        d.add_string(arguments[0]);
      }
      // TODO: don't parse if no files, or strings added
      dip::Environment env = d.parse();
      dip::ValueNode::ListType vnodes;
      arguments = argpar.getAll("--request");
      if (!arguments.empty()) {
        vnodes = env.request_nodes(arguments[0]);
      } else {
        vnodes = env.request_nodes("?");
      }
      // TODO: implement tag selectors
      // TODO: print can have an argument: format
      if (argpar.has("--print")) {
        for (const auto& node : vnodes) {
          std::cout << node->name << " = " << node->to_string() << '\n';
        }
      }
      if (argpar.has("--print-values")) {
        for (const auto& node : vnodes) {
          std::cout << node->to_string() << '\n';
        }
      }
      // TODO: implement exports to other languages: yaml, toml, c/c++, fortran,...
      // TODO: implement output into a file
    } catch (std::exception& e) {
      std::cout << e.what() << '\n';
    }
  }

}
