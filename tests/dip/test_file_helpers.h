#ifndef SNT_DIP_TEST_FILE_HELPERS_H
#define SNT_DIP_TEST_FILE_HELPERS_H

#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace test_file {
    inline void write_text(const std::filesystem::path& path, const std::string& text) {
        std::ofstream output(path, std::ios::binary);
        if (!output || !output.write(text.data(), static_cast<std::streamsize>(text.size())))
            throw std::runtime_error("Cannot write test file: " + path.string());
    }

    inline std::string read_text(const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary);
        if (!input)
            throw std::runtime_error("Cannot read test file: " + path.string());
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }
}

#endif
