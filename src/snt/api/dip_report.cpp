#include <snt/api/dip_report.h>

namespace snt::api {

void generate_dip_report(const dip::Environment& environment, dip::report::ReportFormat format,
                       const std::filesystem::path& output, const dip::report::ReportOptions& options) {
    dip::report::generate(environment, format, output, options);
}

} // namespace snt::api
