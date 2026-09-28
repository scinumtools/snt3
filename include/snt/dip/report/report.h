#ifndef SNT_DIP_REPORT_H
#define SNT_DIP_REPORT_H

#include <filesystem>
#include <string>

namespace snt::dip { class Environment; }

namespace snt::dip::report {

enum class ReportFormat { Tex, Pdf, Markdown, Rst, Html, Typst, Text, Json };

struct ReportOptions {
    std::string input_label;                    ///< Label identifying the report input.
    std::filesystem::path introduction_file;  ///< Optional trusted LaTeX fragment for TeX/PDF only.
    std::string tex_compiler = "pdflatex";     ///< Executable used only for PDF output.
    std::string title = "DIP parameter report"; ///< Report title.
    std::string author;                         ///< Report author; empty means unspecified.
    std::string date;                           ///< Report date; empty uses the generation date.
    std::string version;                        ///< Report version; empty uses the SNT version.
    std::filesystem::path source_root;           ///< Show source paths relative to this directory when possible.
};

/** Generate a report from an already evaluated environment.
 * Text formats need no external tool. PDF output invokes the configured TeX compiler.
 */
void generate(const dip::Environment& environment, ReportFormat format,
              const std::filesystem::path& output, const ReportOptions& options = {});

} // namespace snt::dip::report

#endif
