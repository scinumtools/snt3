#include "argparser.h"
#include "main.h"

#include <snt/api/dip_parse.h>
#include <snt/api/dip_report.h>
#include <cstddef>
#include <iostream>
#include <stdexcept>

namespace {
snt::dip::report::ReportFormat report_format(const std::string& name) {
    using Format = snt::dip::report::ReportFormat;
    if (name == "tex") return Format::Tex;
    if (name == "pdf") return Format::Pdf;
    if (name == "md") return Format::Markdown;
    if (name == "rst") return Format::Rst;
    if (name == "html") return Format::Html;
    if (name == "typ") return Format::Typst;
    if (name == "txt") return Format::Text;
    if (name == "json") return Format::Json;
    throw std::runtime_error("Unknown report format: " + name + ". Use tex, pdf, md, rst, html, typ, txt, or json.");
}

std::string help_report() {
    return R"(
Scientific Numerical Tools v3 (SNT)
Module: DIP Report

Usage:
  snt report [options] [arguments]

Description:
  Generate a report from an evaluated DIP environment using Brief++.
  The report includes values, types, table summaries, applied changes, sources,
  custom units, schemas, overrides where present, and publication metadata.

Options:
  -h, --help
      Show help.
  -v, --version
      Show version information.
  -i, --input <type> [<name>] <value>
      Add input (file/string/override_string/override_file/unit/source/
      schema_string/schema_file), as in 'snt dip parse'.
      Unit, source, and schema inputs require a name and value.
  --project <file>
      Load a DIPfile project. Override inputs may tune its values.
  --load <file>
      Load an evaluated DIPH5 environment instead of DIPL inputs.
  --format <tex|pdf|md|rst|html|typ|txt|json>
      Select the report format (default: tex).
  --output <file>
      Write the report to this file.
  --intro <file.tex>
      Insert trusted LaTeX after the contents page, in TeX or PDF output.
      Supply a fragment without a document preamble.
  --title <text>
      Set the cover title (default: DIP parameter report).
  --author <text>
      Set the cover author (default: Not specified).
  --date <text>
      Set the cover date (default: generation date in YYYY-MM-DD format).
  --report-version <text>
      Set the cover version (default: SNT version).
  --tex-compiler <executable>
      Use this TeX compiler for PDF output (default: pdflatex).
      A compiler must be installed locally; snt report installs nothing.
      Use lualatex for Unicode text unsupported by pdflatex.

Examples:
  snt report --project DIPfile --output report.tex
  snt report --project DIPfile --format html --output report.html
  snt report --project DIPfile --format md --output report.md
  snt report --load run.diph5 --output report.tex
  snt report --project DIPfile --intro introduction.tex --format pdf --output report.pdf
  snt report --input file parameters.dip -i override_string "steps = 200" \
      --output report.tex
)";
}

} // namespace

void module_report(ArgParser& argpar) {
    if (argpar.hasKeyword("-h") || argpar.hasKeyword("--help")) {
        std::cout << help_report();
        return;
    }
    if (argpar.numPositional() != 1)
        throw std::runtime_error("Unknown report command. Use 'snt report --help'.");

    snt::api::DIPParse command;
    bool has_input = false, has_project = false, has_load = false;
    std::string format = "tex", output, intro, compiler = "pdflatex", input_label;
    std::string title, author, date, report_version;
    std::filesystem::path source_root;
    bool has_format = false, has_output = false, has_intro = false, has_compiler = false;

    for (const auto& argument : argpar.getAllKeywords()) {
        const auto& key = argument.key;
        const auto& values = argument.values;
        if (key == "-i" || key == "--input") {
            if (values.empty()) throw std::runtime_error(key + " requires an input type and value.");
            for (size_t i = 0; i < values.size();) {
                const auto& kind = values[i];
                const size_t count = (kind == "file" || kind == "string" || kind == "override_string" ||
                                      kind == "override_file") ? 1 : 2;
                if (i + count >= values.size()) throw std::runtime_error("Incomplete DIP input: " + kind);
                command.argument_add(kind, std::vector<std::string>(
                    values.begin() + static_cast<std::ptrdiff_t>(i + 1),
                    values.begin() + static_cast<std::ptrdiff_t>(i + count + 1)));
                i += count + 1;
            }
            has_input = true;
            if (input_label.empty()) input_label = "DIPL inputs";
        } else if (key == "--project") {
            if (values.size() != 1 || has_project)
                throw std::runtime_error("Specify exactly one --project file.");
            command.argument_add("project", {values.front()});
            has_project = true;
            input_label = values.front();
            source_root = std::filesystem::absolute(values.front()).parent_path();
        } else if (key == "--load") {
            if (values.size() != 1 || has_load)
                throw std::runtime_error("Specify exactly one --load file.");
            command.argument_load(values.front());
            has_load = true;
            input_label = values.front();
        } else if (key == "--format" || key == "--output" || key == "--intro" || key == "--tex-compiler" ||
                   key == "--title" || key == "--author" || key == "--date" || key == "--report-version") {
            if (values.size() != 1) throw std::runtime_error(key + " requires exactly one value.");
            if (key == "--format") {
                if (has_format) throw std::runtime_error("Specify --format only once.");
                format = values.front(); has_format = true;
            } else if (key == "--output") {
                if (has_output) throw std::runtime_error("Specify --output only once.");
                output = values.front(); has_output = true;
            } else if (key == "--intro") {
                if (has_intro) throw std::runtime_error("Specify --intro only once.");
                intro = values.front(); has_intro = true;
            } else if (key == "--tex-compiler") {
                if (has_compiler) throw std::runtime_error("Specify --tex-compiler only once.");
                compiler = values.front(); has_compiler = true;
            } else if (key == "--title") {
                title = values.front();
            } else if (key == "--author") {
                author = values.front();
            } else if (key == "--date") {
                date = values.front();
            } else {
                report_version = values.front();
            }
        } else {
            throw std::runtime_error("Unknown report option: " + key);
        }
    }

    if (!has_input && !has_project && !has_load)
        throw std::runtime_error("Specify --input, --project, or --load.");
    if (output.empty()) throw std::runtime_error("Specify --output for the report.");
    const auto selected_format = report_format(format);
    if (has_compiler && format != "pdf")
        throw std::runtime_error("--tex-compiler requires --format pdf.");
    if (has_intro && format != "tex" && format != "pdf")
        throw std::runtime_error("--intro requires --format tex or pdf.");

    snt::dip::report::ReportOptions options;
    options.input_label = input_label;
    if (has_intro) options.introduction_file = intro;
    options.tex_compiler = compiler;
    if (!title.empty()) options.title = title;
    options.author = author;
    options.date = date;
    options.version = report_version;
    options.source_root = source_root;
    snt::api::generate_dip_report(command.evaluate(), selected_format, output, options);
}
