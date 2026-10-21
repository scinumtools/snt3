#include "main.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <snt/api/dip_format.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    std::string read_file(const std::filesystem::path& path) {
        std::ifstream stream(path, std::ios::binary);
        if (!stream)
            throw std::runtime_error("Cannot read DIPL file: " + path.string());
        return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
    }
} // namespace

int module_fmt(int argc, char* argv[]) {
    bool check = false;
    bool write = false;
    std::vector<std::filesystem::path> files;
    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: snt format [--check | --write] <file> [<file> ...]\n"
                         "Format DIPL source. Output goes to stdout unless --check or --write is used.\n";
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
    if (!check && !write)
        std::cout << snt::api::format_dip_file(files.front());
    return check && changed ? 1 : 0;
}
