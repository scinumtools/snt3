#include "io.h"

#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace snt::dip {

    void Environment::load(const std::filesystem::path& file) {
        try {
            Environment loaded;
            hdf5::load(loaded, file);
            loaded.snapshot_loaded_ = true;
            loaded.source_path_base_ = std::filesystem::absolute(file).parent_path();
            *this = std::move(loaded);
        } catch (const hdf5::Error& error) {
            throw dip::IOException(
                "Unable to load HDF5 environment",
                "The file `" + file.string() + "` could not be loaded: " + error.what(),
                "Check that the file is a valid SciNumTools3 environment.",
                __FILE__,
                __LINE__
            );
        }
    }

    void Environment::save(const std::filesystem::path& file) const {
        try {
            hdf5::save(*this, file, output_plan_ ? &*output_plan_ : nullptr);
        } catch (const hdf5::Error& error) {
            throw dip::IOException(
                "Unable to save DIPH5 archive", error.what(),
                "Save an environment with recoverable source bytes.", __FILE__, __LINE__
            );
        }
    }

    void Environment::save(const std::filesystem::path& file, const OutputPlan& output_plan) const {
        try {
            hdf5::save(*this, file, &output_plan);
        } catch (const hdf5::Error& error) {
            throw dip::IOException("Unable to save DIPH5 v3 archive", error.what(),
                                   "Save an environment with recoverable source bytes.", __FILE__, __LINE__);
        }
    }

    void Environment::extract_sources(const std::filesystem::path& destination) const {
        if (archived_sources_.empty())
            throw hdf5::Error("No archived sources are available for extraction");
        std::filesystem::create_directories(destination);
        for (size_t index = 0; index < archived_sources_.size(); ++index) {
            // Generated names deliberately ignore untrusted original source paths.
            std::ostringstream name;
            name << std::setw(6) << std::setfill('0') << index << ".dip";
            const auto output = destination / name.str();
            // symlink_status also catches a dangling symlink, which exists() would miss.
            if (std::filesystem::symlink_status(output).type() != std::filesystem::file_type::not_found)
                throw hdf5::Error("Refusing to overwrite an extracted source: " + output.string());
            std::ofstream stream(output, std::ios::binary);
            stream.write(archived_sources_[index].content.data(), archived_sources_[index].content.size());
            if (!stream)
                throw hdf5::Error("Unable to write an extracted source: " + output.string());
        }
    }

} // namespace snt::dip
