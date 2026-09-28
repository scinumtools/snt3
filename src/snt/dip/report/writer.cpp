#include "writer.h"
#include "template.h"

#include <sstream>
#include <stdexcept>

namespace snt::dip::report {
namespace {
std::string escape_tex(const std::string& value) {
    std::string out;
    for (char ch : value) {
        switch (ch) {
        case '\\': out += "\\textbackslash{}"; break;
        case '{': out += "\\{"; break;
        case '}': out += "\\}"; break;
        case '#': out += "\\#"; break;
        case '$': out += "\\$"; break;
        case '%': out += "\\%"; break;
        case '&': out += "\\&"; break;
        case '_': out += "\\_"; break;
        case '^': out += "\\textasciicircum{}"; break;
        case '~': out += "\\textasciitilde{}"; break;
        case '\n': out += ' '; break;
        case '\r': break;
        default: out += ch;
        }
    }
    return out;
}

void row(std::ostringstream& out, const std::string& label, const std::string& value,
         const char* color = "sntLabel") {
    if (!value.empty())
        out << "\\textcolor{" << color << "}{\\textbf{" << escape_tex(label) << "}} & "
            << escape_tex(value) << " \\\\\n";
}

std::string location(const Origin& origin) {
    std::string text = !origin.path.empty() ? origin.path : origin.name;
    if (origin.line)
        text += ":" + std::to_string(origin.line);
    return text;
}

void publication_rows(std::ostringstream& out, const Publication& publication) {
    row(out, "Authors", publication.authors, "sntMeta");
    row(out, "Publication", publication.title, "sntMeta");
    row(out, "Journal", publication.journal, "sntMeta");
    row(out, "Year", publication.year, "sntMeta");
    row(out, "Volume", publication.volume, "sntMeta");
    row(out, "Issue", publication.issue, "sntMeta");
    row(out, "Pages", publication.pages, "sntMeta");
    row(out, "DOI", publication.doi, "sntMeta");
    row(out, "URL", publication.url, "sntMeta");
}

std::string joined(const std::vector<std::string>& items) {
    std::string result;
    for (const auto& item : items) {
        if (!result.empty()) result += ", ";
        result += item;
    }
    return result;
}
} // namespace

std::string render_tex(const Document& document) {
    std::ostringstream body;
    body << "\\begin{titlepage}\n\\centering\n"
         << "\\vspace*{0.18\\textheight}\n"
         << "{\\Huge\\bfseries\\textcolor{sntBlue}{" << escape_tex(document.title) << "}\\par}\n"
         << "\\vspace{0.8em}\\textcolor{sntBlue}{\\rule{0.72\\linewidth}{1pt}}\\par\n"
         << "\\vspace{2em}\n"
         << "\\begin{tabular}{@{}rl@{}}\n"
         << "\\textcolor{sntLabel}{\\textbf{Author}} & "
         << escape_tex(document.author.empty() ? "Not specified" : document.author) << " \\\\\n"
         << "\\textcolor{sntLabel}{\\textbf{Date}} & " << escape_tex(document.date) << " \\\\\n"
         << "\\textcolor{sntLabel}{\\textbf{Version}} & " << escape_tex(document.version) << " \\\\\n";
    if (!document.input_label.empty())
        body << "\\textcolor{sntLabel}{\\textbf{Input}} & " << escape_tex(document.input_label) << " \\\\\n";
    body << "\\end{tabular}\n\\end{titlepage}\n"
         << "\\tableofcontents\n\\clearpage\n";
    if (document.loaded_snapshot)
        body << "\\emph{This report shows only the nodes and provenance retained in the DIPH5 snapshot.}\n";

    if (!document.introduction_tex.empty())
        body << "\\section{Introduction}\n" << document.introduction_tex << "\n";

    body << "\\section{Parameters}\n";
    if (document.parameters.empty())
        body << "No evaluated parameters.\n";
    for (const auto& item : document.parameters) {
        body << "\\sntnode{" << escape_tex(item.path) << "}\n"
             << "\\begin{longtable}{@{}p{0.25\\linewidth}p{0.69\\linewidth}@{}}\n";
        row(body, "Value", item.value, "sntBlue");
        row(body, "Units", item.units, "sntBlue");
        row(body, "Description", item.description, "sntMeta");
        row(body, "Applied schemas", joined(item.applied_schemas), "sntMeta");
        row(body, "Contributing schema", item.contributing_schema, "sntMeta");
        row(body, "Overridden", item.overridden ? "yes" : "no", "sntSource");
        row(body, "Declared at", location(item.declaration), "sntSource");
        row(body, "Declaration", item.declaration.code, "sntSource");
        if (item.overridden) {
            row(body, "Override at", location(item.replacement), "sntSource");
            row(body, "Override", item.replacement.code, "sntSource");
        }
        publication_rows(body, item.publication);
        body << "\\end{longtable}\n";
    }

    if (!document.schemas.empty()) {
        body << "\\section{Schemas}\n";
        for (const auto& schema : document.schemas) {
            body << "\\sntnode{" << escape_tex(schema.name) << "}\n"
                 << "\\begin{longtable}{@{}p{0.25\\linewidth}p{0.69\\linewidth}@{}}\n";
            row(body, "Description", schema.description);
            row(body, "Declared at", location(schema.origin));
            publication_rows(body, schema.publication);
            body << "\\end{longtable}\n";
        }
    }

    body << "\\section{Sources}\n";
    if (document.sources.empty())
        body << "No source manifest is available.\n";
    for (const auto& source : document.sources) {
        body << "\\sntnode{" << escape_tex(source.name) << "}\n"
             << "\\begin{longtable}{@{}p{0.25\\linewidth}p{0.69\\linewidth}@{}}\n";
        row(body, "Path", source.path, "sntSource");
        row(body, "Parent", source.parent, "sntSource");
        if (source.parent_line)
            row(body, "Parent line", std::to_string(source.parent_line), "sntSource");
        if (!source.hash.empty())
            row(body, "Content hash", source.hash_algorithm + ": " + source.hash, "sntSource");
        body << "\\end{longtable}\n";
    }

    body << "\\section{Custom units}\n";
    if (document.units.empty())
        body << "No custom units are registered.\n";
    for (const auto& unit : document.units) {
        body << "\\sntnode{" << escape_tex(unit.name) << "}\n"
             << "\\begin{longtable}{@{}p{0.25\\linewidth}p{0.69\\linewidth}@{}}\n";
        row(body, "Definition", unit.definition);
        body << "\\end{longtable}\n";
    }

    if (!document.functions.empty()) {
        body << "\\section{Registered functions}\n";
        for (const auto& function : document.functions) {
            body << "\\sntnode{" << escape_tex(function.name) << "}\n"
                 << "\\begin{longtable}{@{}p{0.25\\linewidth}p{0.69\\linewidth}@{}}\n";
            row(body, "Kind", function.kind);
            body << "\\end{longtable}\n";
        }
    }

    std::string output = latex_template;
    const std::string marker = "%%SNT_REPORT_BODY%%";
    const auto position = output.find(marker);
    if (position == std::string::npos)
        throw std::runtime_error("The snt report LaTeX template is missing its body marker.");
    output.replace(position, marker.size(), body.str());
    return output;
}

} // namespace snt::dip::report
