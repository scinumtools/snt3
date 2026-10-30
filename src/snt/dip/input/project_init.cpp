#include <snt/dip/project_init.h>
#include <snt/dip/artifact.h>
#include <snt/dip/dip.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace snt::dip {
    namespace {
        bool path_is_occupied(const std::filesystem::path& path) {
            return std::filesystem::symlink_status(path).type() != std::filesystem::file_type::not_found;
        }

        void write_new_file(const std::filesystem::path& path, const std::string& contents) {
            if (path_is_occupied(path))
                throw std::runtime_error("refusing to replace existing " + path.filename().string());
            std::ofstream output(path, std::ios::binary);
            if (!output)
                throw std::runtime_error("could not create " + path.string());
            output << contents;
            output.close();
            if (!output) {
                std::error_code ignored;
                std::filesystem::remove(path, ignored);
                throw std::runtime_error("could not write " + path.string());
            }
        }
    } // namespace

    void initialize_project(const std::filesystem::path& directory) {
        const auto manifest = directory / "DIPfile";
        const auto source = directory / "main.dip";
        if (path_is_occupied(manifest) || path_is_occupied(source))
            throw std::runtime_error("DIPfile or main.dip already exists in " + directory.string());

        write_new_file(source, "# Replace this example with your project parameters.\nexample int = 1\n");
        try {
            write_new_file(manifest, "code[]\n  file = \"main.dip\"\n");
        } catch (...) {
            std::error_code ignored;
            std::filesystem::remove(source, ignored);
            throw;
        }
    }

    std::filesystem::path create_project_from_source(const std::filesystem::path& source_file) {
        if (detect_artifact(source_file) != ArtifactKind::DIPL || !std::filesystem::is_regular_file(source_file))
            throw std::invalid_argument("Choose an existing .dip or .dipl source file.");
        if (std::filesystem::is_symlink(source_file))
            throw std::invalid_argument("Convert the source file itself, not a symbolic link to it.");
        const auto name = source_file.filename().string();
        if (name.find_first_of("\"\\\r\n") != std::string::npos)
            throw std::invalid_argument("The source filename contains characters that cannot be used in a DIPfile.");
        const auto manifest = source_file.parent_path() / "DIPfile";
        if (path_is_occupied(manifest))
            throw std::runtime_error("DIPfile already exists in " + source_file.parent_path().string());

        write_new_file(manifest, "code[]\n  file = \"" + name + "\"\n");
        try {
            DIP parser;
            parser.add_project(manifest);
            parser.parse(false, false, true);
        } catch (...) {
            std::error_code ignored;
            std::filesystem::remove(manifest, ignored);
            throw;
        }
        return manifest;
    }
} // namespace snt::dip
