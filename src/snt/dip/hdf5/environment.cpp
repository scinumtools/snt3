#include "io.h"

#include <snt/dip/environment.h>
#include <snt/dip/exceptions.h>

namespace snt::dip {

    void Environment::load(const std::filesystem::path& file) {
        try {
            Environment loaded;
            hdf5::load(loaded, file);
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
            hdf5::save(*this, file);
        } catch (const hdf5::Error& error) {
            throw dip::IOException(
                "Unable to save HDF5 environment",
                "The file `" + file.string() + "` could not be saved: " + error.what(),
                "Check that the destination is writable.",
                __FILE__,
                __LINE__
            );
        }
    }

} // namespace snt::dip
