#include <snt/dip/report/docs.h>
#include <snt/dip/report/report.h>
#include <stdexcept>

namespace snt::dip::report {

    void generate(const dip::Environment&, ReportFormat, const std::filesystem::path&, const ReportOptions&) {
        throw std::runtime_error("Report support is not included in this build. Configure with ENABLE_SNT_REPORT=ON.");
    }

    DocsResult generate_docs(const DocsOptions&) {
        throw std::runtime_error("Report support is not included in this build. Configure with ENABLE_SNT_REPORT=ON.");
    }

} // namespace snt::dip::report
