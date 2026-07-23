#include "argparser.h"
#include "main.h"
#include "snt/api/dip_parse.h"

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
  -i,--input <type> [<name>] <value>
      Register a new input type (file/string/unit/source). Unit and source input require name and value.
  -r,--request <query>
      Request specific nodes (e.g. "family.father").
  --print
      Print nodes.

Examples:
  snt dip parse -f parameters.dip --print

  snt dip parse \
      -i file parameters.dip \
      -i string "age int = 23 yr" \
      -r "family.father" \
      --print
)";
}

void module_dip(ArgParser& argpar) {

    if (argpar.hasKeyword("-h") || argpar.numPositional() < 2) {
        std::cout << help_dip();
    }
    try {

        std::string command = argpar.getPositionalValue(1);
        if (command == "parse") {
            api::DIPParse cmd;
            std::vector<std::string> arguments;
            arguments = argpar.getKeywordValues("-i", "--input");
            if (!arguments.empty()) {
                size_t i = 0;
                size_t arguments_size = arguments.size();
                while (i < arguments_size) {
                    if (arguments[i] == "string" && (i + 1) < arguments_size) {
                        cmd.argument_input(arguments[i], {arguments[i + 1]});
                        i += 2;
                    } else if (arguments[i] == "file" && (i + 1) < arguments_size) {
                        cmd.argument_input(arguments[i], {arguments[i + 1]});
                        i += 2;
                    } else if (arguments[i] == "source" && (i + 2) < arguments_size) {
                        cmd.argument_input(arguments[i], {arguments[i + 1], arguments[i + 2]});
                        i += 3;
                    } else if (arguments[i] == "unit" && (i + 2) < arguments_size) {
                        cmd.argument_input(arguments[i], {arguments[i + 1], arguments[i + 2]});
                        i += 3;
                    } else {
                        throw std::runtime_error("Invalid input type: " + arguments[i]);
                    }
                }
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
            std::cout << cmd.execute();
        }

        // TODO: implement tag selectors
        // TODO: print can have an argument: format
        // TODO: implement exports to other languages: yaml, toml, c/c++, fortran,...
        // TODO: implement output into a file
    } catch (std::exception& e) {
        std::cout << e.what() << '\n';
    }
}
