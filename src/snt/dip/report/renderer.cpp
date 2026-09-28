#include "renderer.h"

#include <briefpp/report.hpp>

#include <stdexcept>
#include <utility>
#include <vector>

namespace snt::dip::report {
namespace {
using Rows = std::vector<std::pair<std::string, std::string>>;

void add_row(Rows& rows, const char* label, const std::string& value) {
    if (!value.empty()) rows.emplace_back(label, value);
}

void add_table(briefpp::Node& section, const char* role, const Rows& rows) {
    if (rows.empty()) return;
    auto& table = section.table().role(role).column_count(2);
    for (const auto& [label, value] : rows) table.row(label, value);
}

briefpp::Node& add_heading(briefpp::Node& section, const std::string& name, bool latex) {
    if (latex) {
        section.raw(briefpp::Backend::Latex, "\\sntnode{" + briefpp::detail::escape_latex(name) + "}");
        return section;
    }
    return section.section(name);
}

std::string location(const Origin& origin) {
    std::string text = !origin.path.empty() ? origin.path : origin.name;
    if (origin.line) text += ":" + std::to_string(origin.line);
    return text;
}

std::string joined(const std::vector<std::string>& items) {
    std::string result;
    for (const auto& item : items) {
        if (!result.empty()) result += ", ";
        result += item;
    }
    return result;
}

Rows publication_rows(const Publication& publication) {
    Rows rows;
    add_row(rows, "Authors", publication.authors);
    add_row(rows, "Publication", publication.title);
    add_row(rows, "Journal", publication.journal);
    add_row(rows, "Year", publication.year);
    add_row(rows, "Volume", publication.volume);
    add_row(rows, "Issue", publication.issue);
    add_row(rows, "Pages", publication.pages);
    add_row(rows, "DOI", publication.doi);
    add_row(rows, "URL", publication.url);
    return rows;
}

std::string preamble(const Document& document) {
    const auto version = briefpp::detail::escape_latex(document.version);
    const auto input = briefpp::detail::escape_latex(document.input_label);
    const std::string input_row = document.input_label.empty() ? "" :
        "\\textcolor{sntLabel}{\\textbf{Input}} & " + input + " \\\\\n";
    return R"(\definecolor{sntBlue}{HTML}{204B70}
\definecolor{sntNodeFill}{HTML}{EDF3F7}
\definecolor{sntLabel}{HTML}{34495A}
\definecolor{sntMeta}{HTML}{246B62}
\definecolor{sntSource}{HTML}{646B73}
\hypersetup{colorlinks=true,linkcolor=sntBlue}
\setlength{\parskip}{0pt}
\setlength{\parindent}{0pt}
\setlength{\LTpre}{0pt}
\setlength{\LTpost}{0pt}
\renewcommand{\arraystretch}{0.9}
\newcommand{\sntnode}[1]{%
  \par\vspace{0.45em}\noindent
  \colorbox{sntNodeFill}{\parbox{\dimexpr\linewidth-2\fboxsep\relax}{%
    \textcolor{sntBlue}{\textbf{\texttt{#1}}}}}%
  \par\nobreak\vspace{0.08em}%
}
\makeatletter
\renewcommand{\maketitle}{%
  \begin{titlepage}\centering
  \vspace*{0.18\textheight}
  {\Huge\bfseries\textcolor{sntBlue}{\@title}\par}
  \vspace{0.8em}\textcolor{sntBlue}{\rule{0.72\linewidth}{1pt}}\par
  \vspace{2em}
  \begin{tabular}{@{}rl@{}}
  \textcolor{sntLabel}{\textbf{Author}} & \@author \\
  \textcolor{sntLabel}{\textbf{Date}} & \@date \\
  \textcolor{sntLabel}{\textbf{Version}} & \sntVersion \\
  \sntInputRow
  \end{tabular}\end{titlepage}%
}
\makeatother
\newcommand{\sntVersion}{)" + version + R"(}
\newcommand{\sntInputRow}{)" + input_row + R"(}
)";
}
} // namespace

std::string render_document(const Document& document, ReportFormat format) {
    const bool latex = format == ReportFormat::Tex || format == ReportFormat::Pdf;
    briefpp::Document report;
    report.title(document.title)
        .author(document.author.empty() ? "Not specified" : document.author)
        .date(document.date);
    if (!latex) {
        report.subtitle("Version: " + document.version);
        if (!document.input_label.empty()) report.institution("Input: " + document.input_label);
    }

    if (latex) report.raw(briefpp::Backend::Latex, "\\tableofcontents\n\\clearpage");
    if (document.loaded_snapshot)
        report.paragraph().emphasis("This report shows only the nodes and provenance retained in the DIPH5 snapshot.");
    if (latex && !document.introduction_tex.empty())
        report.section("Introduction").raw(briefpp::Backend::Latex, document.introduction_tex);

    auto& parameters = report.section("Parameters");
    if (document.parameters.empty()) parameters.paragraph("No evaluated parameters.");
    for (const auto& item : document.parameters) {
        auto& node = add_heading(parameters, item.path, latex);
        Rows primary, metadata, source;
        add_row(primary, "Value", item.value);
        add_row(primary, "Units", item.units);
        add_row(metadata, "Description", item.description);
        add_row(metadata, "Applied schemas", joined(item.applied_schemas));
        add_row(metadata, "Contributing schema", item.contributing_schema);
        add_row(source, "Overridden", item.overridden ? "yes" : "no");
        add_row(source, "Declared at", location(item.declaration));
        add_row(source, "Declaration", item.declaration.code);
        if (item.overridden) {
            add_row(source, "Override at", location(item.replacement));
            add_row(source, "Override", item.replacement.code);
        }
        add_table(node, "snt-primary", primary);
        add_table(node, "snt-metadata", metadata);
        add_table(node, "snt-source", source);
        add_table(node, "snt-metadata", publication_rows(item.publication));
    }

    if (!document.schemas.empty()) {
        auto& schemas = report.section("Schemas");
        for (const auto& schema : document.schemas) {
            auto& node = add_heading(schemas, schema.name, latex);
            Rows details;
            add_row(details, "Description", schema.description);
            add_row(details, "Declared at", location(schema.origin));
            add_table(node, "snt-metadata", details);
            add_table(node, "snt-metadata", publication_rows(schema.publication));
        }
    }

    auto& sources = report.section("Sources");
    if (document.sources.empty()) sources.paragraph("No source manifest is available.");
    for (const auto& source : document.sources) {
        auto& node = add_heading(sources, source.name, latex);
        Rows details;
        add_row(details, "Path", source.path);
        add_row(details, "Parent", source.parent);
        if (source.parent_line) add_row(details, "Parent line", std::to_string(source.parent_line));
        if (!source.hash.empty()) add_row(details, "Content hash", source.hash_algorithm + ": " + source.hash);
        add_table(node, "snt-source", details);
    }

    auto& units = report.section("Custom units");
    if (document.units.empty()) units.paragraph("No custom units are registered.");
    for (const auto& unit : document.units) {
        auto& node = add_heading(units, unit.name, latex);
        Rows details;
        add_row(details, "Definition", unit.definition);
        add_table(node, "snt-metadata", details);
    }

    if (!document.functions.empty()) {
        auto& functions = report.section("Registered functions");
        for (const auto& function : document.functions) {
            auto& node = add_heading(functions, function.name, latex);
            Rows details;
            add_row(details, "Kind", function.kind);
            add_table(node, "snt-metadata", details);
        }
    }

    switch (format) {
    case ReportFormat::Tex:
    case ReportFormat::Pdf: {
        briefpp::LatexRenderer renderer;
        renderer.package("xcolor").package("array").preamble(preamble(document));
        renderer.table_column_spec("snt-primary", "@{}>{\\color{sntBlue}\\bfseries}p{0.25\\linewidth}p{0.69\\linewidth}@{}")
            .table_column_spec("snt-metadata", "@{}>{\\color{sntMeta}\\bfseries}p{0.25\\linewidth}p{0.69\\linewidth}@{}")
            .table_column_spec("snt-source", "@{}>{\\color{sntSource}\\bfseries}p{0.25\\linewidth}p{0.69\\linewidth}@{}");
        return renderer.render(report);
    }
    case ReportFormat::Markdown: return briefpp::MarkdownRenderer{}.render(report);
    case ReportFormat::Rst: return briefpp::RstRenderer{}.render(report);
    case ReportFormat::Html: return briefpp::HtmlRenderer{}.render(report);
    case ReportFormat::Typst: return briefpp::TypstRenderer{}.render(report);
    case ReportFormat::Text: return briefpp::PlainTextRenderer{}.render(report);
    case ReportFormat::Json: return briefpp::JsonRenderer{}.render(report);
    }
    throw std::invalid_argument("Unknown report format.");
}

} // namespace snt::dip::report
