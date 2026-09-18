#ifndef SNT_TEST_DIP_ENVIRONMENT_FIXTURE_H
#define SNT_TEST_DIP_ENVIRONMENT_FIXTURE_H

#include <filesystem>
#include <snt/dip/dip.h>

inline snt::dip::Environment parsed_environment() {
    snt::dip::DIP parser;
    parser.add_string(
        "title str = \"C++ environment\"\n"
        "  !options [\"C++ environment\", \"Other\"]\n"
        "  !tags [\"example\", \"hdf5\"]\n"
        "  ?descr \"Environment round-trip fixture\"\n"
        "simulation\n"
        "  steps int = 100\n"
        "  timestep float = 0.5 fs\n"
        "  enabled bool = true\n"
        "  restart_file str = none\n"
        "boundary[inlet]\n"
        "  velocity float[3] = [1.0, 0.0, 0.0] m/s\n"
        "samples[]\n"
        "  time float = 0 s\n"
        "samples[]\n"
        "  time float = 1 s"
    );
    return parser.parse();
}

inline std::filesystem::path environment_file(const std::string& name) {
    return std::filesystem::temp_directory_path() / ("scinumtools3-" + name + ".diph5");
}

#endif // SNT_TEST_DIP_ENVIRONMENT_FIXTURE_H
