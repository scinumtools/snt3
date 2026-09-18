#ifndef SNT_DIP_HDF5_IO_H
#define SNT_DIP_HDF5_IO_H

#include <filesystem>
#include <stdexcept>

namespace snt::dip {
    class Environment;
}

namespace snt::dip::hdf5 {
    class Error : public std::runtime_error {
      public:
        using std::runtime_error::runtime_error;
    };

    void load(Environment& env, const std::filesystem::path& file);
    void save(const Environment& env, const std::filesystem::path& file);

} // namespace snt::dip::hdf5

#endif
