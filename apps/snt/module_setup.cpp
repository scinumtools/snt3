#include "main.h"
#include "help.h"

#include <snt/api/dip_setup.h>

#include <cctype>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

    void print_help() {
        snt::cli::HelpPage page;
        page.command = "setup";
        page.usage = "snt setup <DIPfile> [--output FILE]";
        page.description = R"(Guide through values marked with `!setup`, validate the answers with DIP, and
write them as an unwrapped DIPL override file. The default output is
setup-overrides.dip in the current directory. Existing files are not replaced.)";
        page.arguments = "<DIPfile>  Project manifest with !setup parameters.";
        page.options = "-h, --help       Show this help.\n--output FILE    Write answers to FILE instead of setup-overrides.dip.";
        page.examples = "snt setup DIPfile\nsnt setup DIPfile --output run-overrides.dip";
        std::cout << snt::cli::render_help(page);
    }

    std::string trim_lower(std::string value) {
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
            value.pop_back();
        std::size_t start = 0;
        while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
            ++start;
        value.erase(0, start);
        for (char& character : value)
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        return value;
    }

    std::string type_name(const snt::dip::SetupQuestion& question) {
        if (question.array)
            return "DIPL array/value expression";
        switch (question.type) {
        case snt::dip::NodeDtype::Boolean:
            return "yes/no";
        case snt::dip::NodeDtype::Integer:
            return "integer";
        case snt::dip::NodeDtype::Float:
            return "number";
        case snt::dip::NodeDtype::String:
            return "text";
        default:
            return "DIPL value expression";
        }
    }

    bool read_line(const std::string& prompt, std::string& value) {
        std::cout << prompt;
        std::cout.flush();
        if (!std::getline(std::cin, value))
            return false;
        return true;
    }

    void ask_boolean(snt::api::DIPSetup& setup, const snt::dip::SetupQuestion& question) {
        while (true) {
            std::string input;
            const std::string default_hint = question.has_value
                                                  ? (question.current_value == "true" ? " [Y/n] " : " [y/N] ")
                                                  : " [y/n] ";
            if (!read_line("  Enable?" + default_hint, input))
                throw std::runtime_error("Input ended before setup was complete.");
            const std::string answer = trim_lower(input);
            if (answer.empty() && question.has_value)
                return;
            if (answer == "y" || answer == "yes" || answer == "true") {
                try {
                    setup.answer(question.path, "true");
                    return;
                } catch (const std::exception& error) {
                    std::cout << "  " << error.what() << '\n';
                }
            }
            if (answer == "n" || answer == "no" || answer == "false") {
                try {
                    setup.answer(question.path, "false");
                    return;
                } catch (const std::exception& error) {
                    std::cout << "  " << error.what() << '\n';
                }
            }
            if (answer != "y" && answer != "yes" && answer != "true" && answer != "n" && answer != "no" &&
                answer != "false")
                std::cout << "  Enter y or n.\n";
        }
    }

    void ask_choice(snt::api::DIPSetup& setup, const snt::dip::SetupQuestion& question) {
        for (std::size_t index = 0; index < question.options.size(); ++index) {
            std::cout << "  " << (index + 1) << ") " << question.options[index].label << '\n';
        }
        if (question.has_value)
            std::cout << "  Current: " << question.current_value << '\n';
        while (true) {
            std::string input;
            const std::string prompt = question.has_value ? "  Select option (Enter keeps current): "
                                                          : "  Select option number: ";
            if (!read_line(prompt, input))
                throw std::runtime_error("Input ended before setup was complete.");
            if (input.empty() && question.has_value)
                return;
            const std::string selected = trim_lower(input);
            std::size_t consumed = 0;
            unsigned long long index = 0;
            try {
                index = std::stoull(selected, &consumed);
            } catch (const std::exception&) {
            }
            if (consumed == selected.size() && index > 0 && index <= question.options.size()) {
                try {
                    setup.choose(question.path, index - 1);
                    return;
                } catch (const std::exception& error) {
                    std::cout << "  " << error.what() << '\n';
                }
            } else {
                std::cout << "  Enter a number from 1 to " << question.options.size() << ".\n";
            }
        }
    }

    void ask_value(snt::api::DIPSetup& setup, const snt::dip::SetupQuestion& question) {
        while (true) {
            std::string input;
            if (question.has_value)
                std::cout << "  Current: " << question.current_value << '\n';
            const std::string prompt = question.array
                                           ? "  Enter a DIPL value expression (Enter keeps current): "
                                       : question.type == snt::dip::NodeDtype::Float ||
                                                 question.type == snt::dip::NodeDtype::Integer
                                           ? "  Enter a value (with units if needed; Enter keeps current): "
                                           : "  Enter text (Enter keeps current): ";
            if (!read_line(prompt, input))
                throw std::runtime_error("Input ended before setup was complete.");
            if (input.empty() && question.has_value)
                return;
            try {
                setup.answer(question.path, input);
                return;
            } catch (const std::exception& error) {
                std::cout << "  " << error.what() << '\n';
            }
        }
    }

} // namespace

int module_setup(int argc, char* argv[]) {
    if (argc == 2 || (argc > 2 && (std::string(argv[2]) == "--help" || std::string(argv[2]) == "-h"))) {
        print_help();
        return 0;
    }

    std::string project;
    std::filesystem::path output = "setup-overrides.dip";
    for (int index = 2; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--output") {
            if (++index >= argc)
                throw std::invalid_argument("--output requires a file path.");
            output = argv[index];
        } else if (!argument.empty() && argument.front() == '-') {
            throw std::invalid_argument("Unknown setup option: " + argument + ". Use 'snt setup --help'.");
        } else if (project.empty()) {
            project = argument;
        } else {
            throw std::invalid_argument("Only one DIPfile project may be supplied.");
        }
    }
    if (project.empty()) {
        print_help();
        return 2;
    }
    if (std::filesystem::exists(output))
        throw std::runtime_error("Refusing to overwrite existing setup file: " + output.string());

    snt::api::DIPSetup setup(project);
    if (setup.questions().empty())
        throw std::runtime_error("No setup parameters were found. Mark project values with `!setup`.");

    std::cout << "Guided setup for " << std::filesystem::absolute(project).string() << "\n\n";
    for (std::size_t index = 0; index < setup.questions().size(); ++index) {
        const auto& question = setup.questions()[index];
        std::cout << '[' << index + 1 << '/' << setup.questions().size() << "] " << question.path;
        if (!question.description.empty())
            std::cout << " — " << question.description;
        else
            std::cout << " (" << type_name(question) << ')';
        std::cout << '\n';

        if (question.type == snt::dip::NodeDtype::Boolean && question.options.empty())
            ask_boolean(setup, question);
        else if (!question.options.empty())
            ask_choice(setup, question);
        else
            ask_value(setup, question);
        std::cout << '\n';
    }

    const auto override_text = setup.override_text();
    setup.preview();
    std::cout << "Validated override:\n";
    if (override_text.empty())
        std::cout << "  (all setup values kept their current values)\n";
    else
        std::cout << override_text;
    std::string confirmation;
    if (!read_line("Write to " + output.string() + "? [Y/n] ", confirmation))
        throw std::runtime_error("Input ended before setup was saved.");
    const auto response = trim_lower(confirmation);
    if (response == "n" || response == "no") {
        std::cout << "Setup discarded.\n";
        return 0;
    }
    setup.save(output);
    std::cout << "Saved " << output.string() << '\n';
    return 0;
}
