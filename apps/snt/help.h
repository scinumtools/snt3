#pragma once

#include <array>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace snt::cli {

    // Keep optional help sections in one order, regardless of which command uses them.
    struct HelpPage {
        std::string module;
        std::string command;
        std::string usage;
        std::string description;
        std::string commands;
        std::string modules;
        std::string arguments;
        std::string options;
        std::string examples;
        std::string notes;
    };

    inline void append_help_section(std::string& output, std::string_view heading, std::string_view body) {
        const auto first = body.find_first_not_of('\n');
        if (first == std::string_view::npos)
            return;
        const auto last = body.find_last_not_of('\n');
        output += '\n';
        output += heading;
        output += ":\n";
        std::istringstream lines(std::string(body.substr(first, last - first + 1)));
        std::string line;
        while (std::getline(lines, line)) {
            if (!line.empty() && line.front() != ' ' && line.front() != '\t')
                output += "  ";
            output += line;
            output += '\n';
        }
    }

    inline std::string render_help(const HelpPage& page) {
        std::string output = "Scientific Numerical Tools (SNT)\n";
        if (!page.module.empty() || !page.command.empty())
            output += '\n';
        if (!page.module.empty())
            output += "Module: " + page.module + '\n';
        if (!page.command.empty())
            output += "Command: " + page.command + '\n';
        append_help_section(output, "Usage", page.usage);
        append_help_section(output, "Description", page.description);
        append_help_section(output, "Commands", page.commands);
        append_help_section(output, "Modules", page.modules);
        append_help_section(output, "Arguments", page.arguments);
        append_help_section(output, "Options", page.options);
        append_help_section(output, "Examples", page.examples);
        append_help_section(output, "Notes", page.notes);
        return output;
    }

    // Allow established, longer help texts to use the same renderer while their
    // detailed body copy remains close to the command it documents.
    inline std::string format_help(std::string_view source) {
        HelpPage page;
        std::istringstream lines{std::string(source)};
        std::string line;
        std::string* current = nullptr;
        const std::array<std::pair<std::string_view, std::string HelpPage::*>, 8> sections{{
            {"Usage", &HelpPage::usage},
            {"Description", &HelpPage::description},
            {"Commands", &HelpPage::commands},
            {"Modules", &HelpPage::modules},
            {"Arguments", &HelpPage::arguments},
            {"Options", &HelpPage::options},
            {"Examples", &HelpPage::examples},
            {"Notes", &HelpPage::notes},
        }};
        while (std::getline(lines, line)) {
            if (line == "Scientific Numerical Tools v3 (SNT)" || line == "Scientific Numerical Tools (SNT)")
                continue;
            if (line.rfind("Module: ", 0) == 0) {
                page.module = line.substr(8);
                continue;
            }
            if (line.rfind("Command: ", 0) == 0) {
                page.command = line.substr(9);
                continue;
            }
            bool heading = false;
            for (const auto& [name, member] : sections) {
                if (line == std::string(name) + ":") {
                    current = &(page.*member);
                    heading = true;
                    break;
                }
            }
            if (heading)
                continue;
            if (!current)
                current = &page.description;
            *current += line + '\n';
        }
        return render_help(page);
    }

} // namespace snt::cli
