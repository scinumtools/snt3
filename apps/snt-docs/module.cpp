#include "compiler.h"
#include "model.h"
#include "writer.h"
#include "../snt/argparser.h"
#include "../snt/main.h"

#include <snt/api/dip_parse.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
std::string help_docs() {
    return R"(
Scientific Numerical Tools v3 (SNT)
Module: DIP Documentation

Usage:
  snt docs [options] [arguments]

Description:
  Generate a TeX or PDF report from an evaluated DIP environment.
  The report includes values, hierarchy, sources, custom units, schemas,
  overrides, and available publication metadata.

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
  --format <tex|pdf>
      Select the report format (default: tex).
  --output <file>
      Write the report to this file.
  --intro <file.tex>
      Insert trusted LaTeX after the title, in both TeX and PDF output.
      Supply a fragment without a document preamble.
  --tex-compiler <executable>
      Use this TeX compiler for PDF output (default: pdflatex).
      A compiler must be installed locally; snt docs installs nothing.
      Use lualatex for Unicode text unsupported by pdflatex.

Examples:
  snt docs --project DIPfile --output report.tex
  snt docs --load run.diph5 --output report.tex
  snt docs --project DIPfile --intro introduction.tex --format pdf --output report.pdf
  snt docs --input file parameters.dip -i override_string "steps = 200" \
      --output report.tex
)";
}

std::string read_file(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw std::runtime_error("Cannot read introduction file: " + path);
    return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}
} // namespace

void module_docs(ArgParser& argpar) {
    if (argpar.hasKeyword("-h") || argpar.hasKeyword("--help")) {
        std::cout << help_docs();
        return;
    }
    if (argpar.numPositional() != 1)
        throw std::runtime_error("Unknown docs command. Use 'snt docs --help'.");

    snt::api::DIPParse command;
    bool has_input = false, has_project = false, has_load = false;
    std::string format = "tex", output, intro, compiler = "pdflatex", input_label;
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
        } else if (key == "--load") {
            if (values.size() != 1 || has_load)
                throw std::runtime_error("Specify exactly one --load file.");
            command.argument_load(values.front());
            has_load = true;
            input_label = values.front();
        } else if (key == "--format" || key == "--output" || key == "--intro" || key == "--tex-compiler") {
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
            } else {
                if (has_compiler) throw std::runtime_error("Specify --tex-compiler only once.");
                compiler = values.front(); has_compiler = true;
            }
        } else {
            throw std::runtime_error("Unknown docs option: " + key);
        }
    }

    if (!has_input && !has_project && !has_load)
        throw std::runtime_error("Specify --input, --project, or --load.");
    if (output.empty()) throw std::runtime_error("Specify --output for the report.");
    if (format != "tex" && format != "pdf")
        throw std::runtime_error("Unknown docs format: " + format + ". Use tex or pdf.");
    if (has_compiler && format != "pdf")
        throw std::runtime_error("--tex-compiler requires --format pdf.");

    const auto document = snt::docs::build_document(command.evaluate(), input_label,
                                                    has_intro ? read_file(intro) : "", has_load);
    const auto tex = snt::docs::render_tex(document);
    if (format == "tex") snt::docs::write_file(output, tex);
    else snt::docs::compile_pdf(tex, output, compiler);
}
