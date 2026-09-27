#include <snt/docs/report.h>

#include "compiler.h"
#include "model.h"
#include "writer.h"

#include <fstream>
#include <iterator>
#include <stdexcept>

namespace snt::docs {

void generate(const dip::Environment& environment, ReportFormat format,
              const std::filesystem::path& output, const ReportOptions& options) {
    if (output.empty()) throw std::invalid_argument("A report output path is required.");
    std::string introduction;
    if (!options.introduction_file.empty()) {
        std::ifstream stream(options.introduction_file, std::ios::binary);
        if (!stream)
            throw std::runtime_error("Cannot read introduction file: " + options.introduction_file.string());
        introduction.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
        if (!stream.eof() && stream.fail())
            throw std::runtime_error("Cannot read introduction file: " + options.introduction_file.string());
    }
    const auto document = build_document(environment, options.input_label, introduction,
                                         environment.is_loaded_snapshot());
    const auto tex = render_tex(document);
    switch (format) {
    case ReportFormat::Tex: write_file(output, tex); break;
    case ReportFormat::Pdf: compile_pdf(tex, output, options.tex_compiler); break;
    default: throw std::invalid_argument("Unknown report format.");
    }
}

} // namespace snt::docs
