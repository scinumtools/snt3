#ifndef SNT_DOCS_REPORT_H
#define SNT_DOCS_REPORT_H

#include <filesystem>
#include <string>

namespace snt::dip { class Environment; }

namespace snt::docs {

enum class ReportFormat { Tex, Pdf };

struct ReportOptions {
    std::string input_label;                    ///< Label printed at the top of the report.
    std::filesystem::path introduction_file;  ///< Optional trusted LaTeX fragment.
    std::string tex_compiler = "pdflatex";     ///< Executable used only for PDF output.
};

/** Generate a report from an already evaluated environment.
 * TeX output needs no external tool. PDF output invokes the configured TeX compiler.
 */
void generate(const dip::Environment& environment, ReportFormat format,
              const std::filesystem::path& output, const ReportOptions& options = {});

} // namespace snt::docs

#endif
