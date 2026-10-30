#include "main.h"
#include "help.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <snt/api/dip_validate.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    void print_diagnostic(const snt::core::Diagnostic& diagnostic, const std::filesystem::path& input) {
        const auto& place = diagnostic.location;
        std::cerr << (place && !place->source.empty() ? place->source : input.string());
        if (place && place->line) {
            std::cerr << ':' << place->line;
            if (place->column)
                std::cerr << ':' << *place->column;
        }
        std::cerr << ": error [" << diagnostic.code << "]: " << diagnostic.message << '\n';
        if (!diagnostic.details.empty())
            std::cerr << "  " << diagnostic.details << '\n';
        if (!diagnostic.suggestion.empty())
            std::cerr << "  suggestion: " << diagnostic.suggestion << '\n';
    }
} // namespace

int module_validate(int argc, char* argv[]) {
    const auto print_help = [] {
        snt::cli::HelpPage page;
        page.command = "validate";
        page.usage = "snt validate [--project] [--json] <file> [<file> ...]";
        page.description = "Validate DIPL files or DIPfile projects with the native parser.";
        page.arguments = "<file>  One or more DIPL files, or DIPfile projects with --project.";
        page.options = R"(-h, --help  Show this help.
--project    Treat each input as a DIPfile project.
--json       Write machine-readable diagnostics.
--           Treat later arguments as file names.)";
        page.examples = "snt validate parameters.dip\nsnt validate --project DIPfile\nsnt validate --json parameters.dip";
        page.notes = "Exit status: 0 valid, 1 invalid DIPL, 2 input or command error.";
        std::cout << snt::cli::render_help(page);
    };
    if (argc == 2) {
        print_help();
        return 0;
    }
    bool project = false;
    bool json = false;
    bool positional_only = false;
    std::vector<std::filesystem::path> inputs;
    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (!positional_only && (arg == "--help" || arg == "-h")) {
            print_help();
            return 0;
        }
        if (!positional_only && arg == "--project")
            project = true;
        else if (!positional_only && arg == "--json")
            json = true;
        else if (!positional_only && arg == "--")
            positional_only = true;
        else if (!positional_only && !arg.empty() && arg.front() == '-')
            throw std::invalid_argument("Unknown validate option: " + arg);
        else
            inputs.emplace_back(arg);
    }
    if (inputs.empty())
        throw std::invalid_argument("Specify at least one DIPL file or DIPfile");

    int status = 0;
    if (json && inputs.size() > 1)
        std::cout << '[';
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        const auto& input = inputs[i];
        const auto result = snt::api::validate_dip_file(input, project);
        if (!result.valid()) {
            for (const auto& diagnostic : result.diagnostics) {
                status = std::max(status, diagnostic.code == "dip.io" ? 2 : 1);
                if (!json)
                    print_diagnostic(diagnostic, input);
            }
        } else if (!json) {
            std::cout << input.string() << ": valid\n";
        }
        if (json) {
            if (i)
                std::cout << ',';
            std::cout << snt::api::dip_validation_json(input, result);
        }
    }
    if (json)
        std::cout << (inputs.size() > 1 ? "]\n" : "\n");
    return status;
}
