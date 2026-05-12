#include "main.h"
#include "argparser.h"
#include "snt/cli/dip_parse.h"

#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>

using namespace snt;

std::string help_dip() {
  return R"(
Scientific Numerical Tools v3 (SNT)
Module: Dimensional Input Parameters (DIP)

Usage:
  snt dip parse [options] [arguments]

Description:
  Parse and query dimensional input parameter definitions.

Options:
  -h, --help
      Show help.
  -v, --version
      Show version information.
  -f,--file <filename>
      Load a .dip file (repeatable).
  -s,--string <code>
      Parse inline DIP code (repeatable).
  -r,--request <query>
      Request specific nodes (e.g. "?family.father").
  --print
      Print nodes.

Examples:
  snt dip parse -f parameters.dip --print

  snt dip parse \
      -f parameters.dip \
      -s "age int = 23 yr" \
      -r "?family.father" \
      --print
)";
}
  
void main_dip(ArgParser& argpar) {
  
  if (argpar.hasKeyword("-h") || argpar.numPositional()<2) {
    std::cout << help_dip();
  }
  try {

    std::string command = argpar.getPositionalValue(1);
    if (command=="parse") {
      cli::DIPParse cmd;
      std::vector<std::string> arguments;
      arguments = argpar.getKeywordValues("-f", "--file");
      if (!arguments.empty()) {
	cmd.argument_file(arguments[0]);
      }
      arguments = argpar.getKeywordValues("-s", "--string");
      if (!arguments.empty()) {
        cmd.argument_string(arguments);
      }
      arguments = argpar.getKeywordValues("-r", "--request");
      if (!arguments.empty()) {
        cmd.argument_request(arguments[0]);
      }
      arguments = argpar.getKeywordValues("-t", "--tags");
      if (!arguments.empty()) {
        cmd.argument_tags(arguments);
      }
      if (argpar.hasKeyword("--print")) {
	cmd.argument_print();
      }
      cmd.execute();
    }
    
    // TODO: implement tag selectors
    // TODO: print can have an argument: format
    // TODO: implement exports to other languages: yaml, toml, c/c++, fortran,...
    // TODO: implement output into a file
  } catch (std::exception& e) {
    std::cout << e.what() << '\n';
  }
}
