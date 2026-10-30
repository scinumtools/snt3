#include "main.h"
#include "help.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <snt/api/dip_format.h>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace {
    std::string read_file(const std::filesystem::path& path) {
        std::ifstream stream(path, std::ios::binary);
        if (!stream)
            throw std::runtime_error("Cannot read DIPL file: " + path.string());
        return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
    }
} // namespace

int module_fmt(int argc, char* argv[]) {
    const auto print_help = [] {
        snt::cli::HelpPage page;
        page.command = "format";
        page.usage = "snt format [--check | --write] <file> [<file> ...]";
        page.description = "Format DIPL source. Output goes to stdout unless --check or --write is used.";
        page.arguments = "<file>  DIPL source file. Multiple files require --check or --write.";
        page.options = R"(-h, --help  Show this help.
--check     Report files that need formatting; exit 1 if any do.
--write     Format files in place.)";
        page.examples = "snt format parameters.dip\nsnt format --check parameters.dip\nsnt format --write a.dip b.dip";
        std::cout << snt::cli::render_help(page);
    };
    if (argc == 2) {
        print_help();
        return 0;
    }
    bool check = false;
    bool write = false;
    std::vector<std::filesystem::path> files;
    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        }
        if (arg == "--check")
            check = true;
        else if (arg == "--write")
            write = true;
        else if (!arg.empty() && arg[0] == '-')
            throw std::invalid_argument("Unknown format option: " + arg);
        else
            files.emplace_back(arg);
    }
    if (check && write)
        throw std::invalid_argument("Use either --check or --write");
    if (files.empty() || (files.size() > 1 && !check && !write))
        throw std::invalid_argument("Specify one file, or use --check/--write with several files");

    bool changed = false;
    for (const auto& path : files) {
        const auto before = read_file(path);
        const auto after = snt::api::format_dip_file(path);
        if (before == after)
            continue;
        changed = true;
        if (check)
            std::cerr << path.string() << " needs formatting\n";
        else if (write) {
            std::ofstream stream(path, std::ios::binary | std::ios::trunc);
            if (!stream || !(stream << after))
                throw std::runtime_error("Cannot write DIPL file: " + path.string());
        }
    }
    if (!check && !write) {
#ifdef _WIN32
        // The formatter preserves the input's line endings. Windows text-mode
        // stdout would silently translate each LF and change those bytes.
        if (_setmode(_fileno(stdout), _O_BINARY) == -1)
            throw std::runtime_error("Cannot set binary stdout for formatted DIPL");
#endif
        std::cout << snt::api::format_dip_file(files.front());
    }
    return check && changed ? 1 : 0;
}
