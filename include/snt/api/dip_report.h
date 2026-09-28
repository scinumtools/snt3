#ifndef SNT_API_DIP_REPORT_H
#define SNT_API_DIP_REPORT_H

#include <snt/dip/report/report.h>

namespace snt::api {

/** Generate a report from an evaluated DIP environment. */
void generate_dip_report(const dip::Environment& environment, dip::report::ReportFormat format,
                       const std::filesystem::path& output, const dip::report::ReportOptions& options = {});

} // namespace snt::api

#endif
