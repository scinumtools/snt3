#include "main.h"
#include "help.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <snt/api/dip_lint.h>
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
        std::cerr << (diagnostic.severity == snt::core::DiagnosticSeverity::Error ? ": error [" : ": warning [")
                  << diagnostic.code << "]: " << diagnostic.message;
        if (diagnostic.node_path)
            std::cerr << " (`" << *diagnostic.node_path << "`)";
        std::cerr << '\n';
        if (!diagnostic.details.empty())
            std::cerr << "  " << diagnostic.details << '\n';
        if (!diagnostic.suggestion.empty())
            std::cerr << "  suggestion: " << diagnostic.suggestion << '\n';
    }
} // namespace

int module_lint(int argc, char* argv[]) {
    const auto print_help = [] {
        snt::cli::HelpPage page;
        page.command = "lint";
        page.usage = "snt lint [--project] [--json] <file> [<file> ...]";
        page.description = "Validate, then check evaluated DIPL for advisory issues.";
        page.arguments = "<file>  One or more DIPL files, or DIPfile projects with --project.";
        page.options = R"(-h, --help  Show this help.
--project    Treat each input as a DIPfile project.
--json       Write machine-readable findings.
--           Treat later arguments as file names.)";
        page.examples = "snt lint parameters.dip\nsnt lint --project DIPfile\nsnt lint --json parameters.dip";
        page.notes = "Exit status: 0 clean, 1 findings or invalid DIPL, 2 input or command error.";
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
            throw std::invalid_argument("Unknown lint option: " + arg);
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
        const auto result = snt::api::lint_dip_file(input, project);
        if (!result.clean()) {
            for (const auto& diagnostic : result.diagnostics) {
                status = std::max(status, diagnostic.code == "dip.io" ? 2 : 1);
                if (!json)
                    print_diagnostic(diagnostic, input);
            }
        } else if (!json) {
            std::cout << input.string() << ": clean\n";
        }
        if (json) {
            if (i)
                std::cout << ',';
            std::cout << snt::api::dip_lint_json(input, result);
        }
    }
    if (json)
        std::cout << (inputs.size() > 1 ? "]\n" : "\n");
    return status;
}
