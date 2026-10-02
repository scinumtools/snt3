#include "argparser.h"
#include "main.h"
#include "snt/api/dip_compare.h"
#include "snt/api/dip_parse.h"

#include <cstddef>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace snt;

std::string help_dip() {
    return R"(
Scientific Numerical Tools v3 (SNT)
Module: Dimensional Input Parameters (DIP)

Usage:
  snt dip parse [options] [arguments]
  snt dip compare <before.diph5> <after.diph5> [options]

Description:
  Parse and query dimensional input parameter definitions.

Options:
  -h, --help
      Show help.
  -v, --version
      Show version information.
  -i,--input <type> [<name>] <value>
      Add input (file/string/override_string/override_file/unit/source/schema_string/schema_file).
      override_string takes an unwrapped body of path = value modifications.
      override_file reads such a body from a file.
      Unit, source, and schema inputs require name and value.
  --project <file>
      Load a DIPfile project; override_string/override_file inputs may tune its values.
  --load <file>
      Load an evaluated DIPH5 environment instead of --input.
  --save <file>
      Save the full environment as DIPH5, overwriting the file.
      Request and tag filters affect printed output only.
  --record-graph
      Record evaluation dependencies for inspection and include them in --save output.
  --relative-source-paths
      Store source paths relative to the --save DIPH5 file.
  --generate <cpp|c|fortran|rust|julia|json|yaml> <file>
      Generate static parameters in the selected format, overwriting the file.
      Request and tag filters affect printed output only.
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

  snt dip parse --project DIPfile --print

  snt dip parse -i file parameters.dip --save parameters.diph5
  snt dip parse --load parameters.diph5 --print
  snt dip parse -i file parameters.dip --generate cpp parameters.hpp
  snt dip compare before.diph5 after.diph5 --scope full

  snt dip parse \
      -i file parameters.dip \
      -i string "age int = 23 yr" \
      -r "family.father" \
      --print
)";
}

int module_dip_compare(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        std::cout << "Usage: snt dip compare <before.diph5> <after.diph5> "
                     "[--scope effective|full] [--max-details N] [--max-array-examples N]\n";
        return 0;
    }
    std::vector<std::string> paths;
    dip::ComparisonOptions options;
    std::size_t max_details = 50;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--scope" || argument == "--max-details" || argument == "--max-array-examples") {
            if (++index >= argc)
                throw std::invalid_argument(argument + " requires a value.");
            const std::string value = argv[index];
            if (argument == "--scope") {
                if (value == "full") options.scope = dip::ComparisonScope::Full;
                else if (value != "effective") throw std::invalid_argument("Scope must be effective or full.");
            } else {
                if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
                    throw std::invalid_argument(argument + " requires a nonnegative integer.");
                const auto count = static_cast<std::size_t>(std::stoull(value));
                if (argument == "--max-details") max_details = count;
                else options.max_array_examples = count;
            }
        } else if (!argument.empty() && argument[0] == '-') {
            throw std::invalid_argument("Unknown comparison option: " + argument);
        } else {
            paths.push_back(argument);
        }
    }
    if (paths.size() != 2)
        throw std::invalid_argument("Compare requires exactly two DIPH5 file paths.");
    api::DIPCompare command(paths[0], paths[1]);
    command.set_options(options);
    command.set_max_details(max_details);
    const auto result = command.compare();
    std::cout << api::render_dip_comparison(result, max_details);
    return result.equal() ? 0 : 1;
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
    bool has_project = false;
    bool has_load = false;
    bool has_save = false;
    bool has_generate = false;
    bool record_graph = false;
    bool relative_source_paths = false;
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
                size_t count = (kind == "file" || kind == "string" || kind == "override_string" || kind == "override_file") ? 1 : 2;
                if (i + count >= values.size())
                    throw std::runtime_error("Incomplete DIP input: " + kind);
                cmd.argument_add(
                    kind,
                    std::vector<std::string>(
                        values.begin() + static_cast<std::ptrdiff_t>(i + 1),
                        values.begin() + static_cast<std::ptrdiff_t>(i + count + 1)
                    )
                );
                i += count + 1;
            }
            has_input = true;
        } else if (key == "--project") {
            if (values.size() != 1 || has_project)
                throw std::runtime_error("Specify exactly one --project file.");
            cmd.argument_add("project", {values.front()});
            has_project = true;
        } else if (key == "--load") {
            if (values.size() != 1 || has_load)
                throw std::runtime_error("Specify exactly one --load file.");
            cmd.argument_load(values.front());
            has_load = true;
        } else if (key == "--save") {
            if (values.size() != 1 || has_save)
                throw std::runtime_error("Specify exactly one --save file.");
            cmd.argument_save(values.front());
            has_save = true;
        } else if (key == "--record-graph" || key == "--relative-source-paths") {
            if (!values.empty())
                throw std::runtime_error(key + " does not take arguments.");
            if (key == "--record-graph") record_graph = true;
            else relative_source_paths = true;
        } else if (key == "--generate") {
            if (values.size() != 2 || has_generate)
                throw std::runtime_error("Specify one export format and one output file with --generate.");
            cmd.argument_generate(values[0], values[1]);
            has_generate = true;
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
    if (!has_input && !has_project && !has_load)
        throw std::runtime_error("Specify a DIP input with --input, --project, or --load.");
    if (record_graph && has_load)
        throw std::runtime_error("--record-graph requires DIPL input, not --load.");
    if (relative_source_paths && !has_save)
        throw std::runtime_error("--relative-source-paths requires --save.");
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
    cmd.argument_record_dependency_graph(record_graph);
    cmd.argument_relative_source_paths(relative_source_paths);
    std::cout << cmd.execute();
}
