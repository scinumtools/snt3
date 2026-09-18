#ifndef SNT_DIP_GENERATE_EXPORT_H
#define SNT_DIP_GENERATE_EXPORT_H

#include <filesystem>
#include <snt/dip/environment.h>

namespace snt::dip::generate {
    void write(const Environment& environment, ExportFormat format, const std::filesystem::path& file);
}

#endif // SNT_DIP_GENERATE_EXPORT_H
