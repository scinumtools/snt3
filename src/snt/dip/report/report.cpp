#include <snt/dip/report/report.h>

#include "compiler.h"
#include "model.h"
#include "writer.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace snt::dip::report {
namespace {
std::string current_date() {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm local{};
#ifdef _WIN32
    if (localtime_s(&local, &now) != 0)
#else
    if (!localtime_r(&now, &local))
#endif
        throw std::runtime_error("Cannot determine the report date.");
    std::ostringstream date;
    date << std::put_time(&local, "%Y-%m-%d");
    return date.str();
}
} // namespace

void generate(const dip::Environment& environment, ReportFormat format,
              const std::filesystem::path& output, const ReportOptions& options) {
    if (output.empty()) throw std::invalid_argument("A report output path is required.");
    std::string introduction;
    if (!options.introduction_file.empty()) {
        if (!std::filesystem::is_regular_file(options.introduction_file))
            throw std::runtime_error("Cannot read introduction file: " + options.introduction_file.string());
        std::ifstream stream(options.introduction_file, std::ios::binary);
        if (!stream)
            throw std::runtime_error("Cannot read introduction file: " + options.introduction_file.string());
        introduction.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
        if (!stream.eof() && stream.fail())
            throw std::runtime_error("Cannot read introduction file: " + options.introduction_file.string());
    }
    const auto document = build_document(environment, options.input_label, introduction,
                                         options.title, options.author,
                                         options.date.empty() ? current_date() : options.date,
                                         options.version.empty() ? CODE_VERSION : options.version,
                                         options.source_root,
                                         environment.is_loaded_snapshot());
    const auto tex = render_tex(document);
    switch (format) {
    case ReportFormat::Tex: write_file(output, tex); break;
    case ReportFormat::Pdf: compile_pdf(tex, output, options.tex_compiler); break;
    default: throw std::invalid_argument("Unknown report format.");
    }
}

} // namespace snt::dip::report
