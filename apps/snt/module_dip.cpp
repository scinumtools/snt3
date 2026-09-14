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
      Add a new source type (file/string/unit/source). Unit and source input require name and value.
  -r,--request <query>
      Request specific nodes (e.g. "family.father").
  --print
      Print nodes with their names and units.
  --value
      Print exactly one defined, unitless scalar without a name or quotes.
      Requires --request. Errors are written to stderr with a nonzero exit status.
  --type <bool|integer|float|string>
      Require this DIPL type with --value (no implicit conversion).

Examples:
  snt dip parse -i file parameters.dip --print

  snt dip parse \
      -i file parameters.dip \
      -i string "age int = 23 yr" \
      -r "family.father" \
      --print
)";
}

void module_dip(ArgParser& argpar) {
    if (argpar.hasKeyword("-h") || argpar.hasKeyword("--help") || argpar.numPositional() < 2) {
        std::cout << help_dip();
        return;
    }
    if (argpar.getPositionalValue(1) != "parse" || argpar.numPositional() != 2) {
        throw std::runtime_error("Unknown DIP command. Use 'snt dip parse --help'.");
    }

    api::DIPParse cmd;
    bool has_input = false;
    bool has_request = false;
    bool print = false;
    bool value = false;
    std::string type;
    for (const auto& argument : argpar.getAllKeywords()) {
        const auto& key = argument.key;
        const auto& values = argument.values;
        if (key == "-i" || key == "--input") {
            if (values.empty())
                throw std::runtime_error(key + " requires an input type and value.");
            for (size_t i = 0; i < values.size();) {
                const auto& kind = values[i];
                size_t count = (kind == "file" || kind == "string") ? 1 : 2;
                if (i + count >= values.size())
                    throw std::runtime_error("Incomplete DIP input: " + kind);
                cmd.argument_add(
                    kind, std::vector<std::string>(values.begin() + i + 1, values.begin() + i + count + 1)
                );
                i += count + 1;
            }
            has_input = true;
        } else if (key == "-r" || key == "--request") {
            if (values.size() != 1 || has_request)
                throw std::runtime_error("Specify exactly one request.");
            cmd.argument_request(values.front());
            has_request = true;
        } else if (key == "-t" || key == "--tags") {
            if (values.empty())
                throw std::runtime_error(key + " requires tags.");
            cmd.argument_tags(values);
        } else if (key == "--print" || key == "--value") {
            if (!values.empty())
                throw std::runtime_error(key + " does not take arguments.");
            if (key == "--print")
                print = true;
            else
                value = true;
        } else if (key == "--type") {
            if (values.size() != 1)
                throw std::runtime_error("--type requires one scalar type.");
            type = values.front();
        } else {
            throw std::runtime_error("Unknown DIP option: " + key);
        }
    }
    if (!has_input)
        throw std::runtime_error("Specify a DIP input with --input.");
    if (print && value)
        throw std::runtime_error("Use either --print or --value.");
    if (!type.empty() && !value)
        throw std::runtime_error("--type requires --value.");
    if (value && !has_request)
        throw std::runtime_error("--value requires --request.");
    if (print)
        cmd.argument_print();
    if (value)
        cmd.argument_value(type);
    std::cout << cmd.execute();
}
