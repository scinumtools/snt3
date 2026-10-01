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

briefpp::Node& add_heading(briefpp::Node& section, const std::string& name, bool latex,
                           const std::string& id = {}) {
    if (latex) {
        if (!id.empty()) section.raw(briefpp::Backend::Latex, "\\hypertarget{" + id + "}{}");
        section.raw(briefpp::Backend::Latex, "\\sntnode{" + briefpp::detail::escape_latex(name) + "}");
        return section;
    }
    return section.section(name).label(id);
}

std::string parameter_id(size_t index) { return "snt-parameter-" + std::to_string(index + 1); }

std::string overview_value(const Parameter& parameter) {
    if (!parameter.shape.empty())
        return "Array (" + parameter.shape + ")" + (parameter.units.empty() ? "" : " " + parameter.units);
    const auto value = parameter.value + (parameter.units.empty() ? "" : " " + parameter.units);
    constexpr size_t limit = 72;
    if (value.size() <= limit) return value;
    size_t end = limit;
    while (end > 0 && (static_cast<unsigned char>(value[end]) & 0xc0) == 0x80) --end;
    return value.substr(0, end) + "...";
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

std::string displayed_hash(const std::string& hash) {
    constexpr std::size_t visible_characters = 16;
    return hash.size() > visible_characters
        ? hash.substr(0, visible_characters) + "..."
        : hash;
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
\setlength{\LTleft}{0pt}
\renewcommand{\arraystretch}{0.82}
\newcommand{\sntnode}[1]{%
  \par\vspace{0.35em}\noindent
  \colorbox{sntNodeFill}{\parbox{\dimexpr\linewidth-2\fboxsep\relax}{%
    \textcolor{sntBlue}{\textbf{\texttt{#1}}}}}%
  \par\nobreak\vspace{0.03em}%
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

    auto& guide = report.section("Parameter guide");
    guide.paragraph("A quick index to the full reference entries. Scalars show effective values; arrays show shape and units.");
    if (document.parameters.empty()) {
        guide.paragraph("No evaluated parameters.");
    } else if (latex) {
        std::string table = "\\begin{longtable}{@{}p{0.48\\linewidth}p{0.47\\linewidth}@{}}\n"
                            "\\textbf{Parameter} & \\textbf{Value / summary} \\\\\n";
        for (size_t i = 0; i < document.parameters.size(); ++i) {
            const auto& item = document.parameters[i];
            table += "\\hyperlink{" + parameter_id(i) + "}{\\texttt{" +
                briefpp::detail::escape_latex(item.path) + "}} & " +
                briefpp::detail::escape_latex(overview_value(item)) + " \\\\\n";
        }
        guide.raw(briefpp::Backend::Latex, table + "\\end{longtable}\n");
    } else {
        auto& table = guide.table().columns("Parameter", "Value / summary");
        for (size_t i = 0; i < document.parameters.size(); ++i) {
            const auto& item = document.parameters[i];
            if (format == ReportFormat::Text)
                table.row(item.path, overview_value(item));
            else {
                table.cell().link(item.path, "#" + parameter_id(i));
                table.cell().text(overview_value(item));
            }
        }
    }
    if (!document.graph_recorded)
        guide.paragraph("Calculation relationships are unavailable because no dependency graph was recorded.");
    else
        guide.paragraph("Inputs below are values read during evaluation; a read does not always mean the value was necessary for the result.");

    auto& parameters = report.section("Parameters");
    if (document.parameters.empty()) parameters.paragraph("No evaluated parameters.");
    for (size_t i = 0; i < document.parameters.size(); ++i) {
        const auto& item = document.parameters[i];
        auto& node = add_heading(parameters, item.path, latex, parameter_id(i));
        Rows primary, metadata, explanation, source;
        add_row(primary, "Value", item.value);
        add_row(primary, "Type", item.type);
        add_row(primary, "Shape", item.shape);
        add_row(primary, "Units", item.units);
        add_row(metadata, "Description", item.description);
        add_row(metadata, "Applied schemas", joined(item.applied_schemas));
        add_row(metadata, "Supplied by schema", item.contributing_schema);
        add_row(explanation, "Expression", item.expression);
        add_row(explanation, "Reads during evaluation", joined(item.reads));
        add_row(explanation, "Selected by", joined(item.selected_by));
        add_row(explanation, "Validation condition", item.condition);
        add_row(explanation, "Used by", joined(item.used_by));
        add_row(source, "Declared at", location(item.declaration));
        add_row(source, "Declaration", item.declaration.code);
        for (const auto& modification : item.modifications) {
            add_row(source, "Modified at", location(modification));
            add_row(source, "Modification", modification.code);
        }
        if (item.overridden) {
            add_row(source, "Override at", location(item.replacement));
            add_row(source, "Override", item.replacement.code);
        }
        add_table(node, "snt-primary", primary);
        add_table(node, "snt-metadata", metadata);
        add_table(node, "snt-metadata", explanation);
        add_table(node, "snt-source", source);
        add_table(node, "snt-metadata", publication_rows(item.publication));
    }

    if (!document.tables.empty()) {
        auto& tables = report.section("Tables");
        tables.paragraph("Column values are listed under Parameters.");
        for (const auto& table : document.tables) {
            auto& node = add_heading(tables, table.path, latex);
            Rows details, columns;
            add_row(details, "Rows", std::to_string(table.rows));
            add_row(details, "Columns", std::to_string(table.columns.size()));
            for (size_t i = 0; i < table.columns.size(); ++i) {
                const auto& column = table.columns[i];
                std::string description = column.name + " (" + column.type;
                if (!column.units.empty()) description += ", " + column.units;
                description += ")";
                columns.emplace_back("Column " + std::to_string(i + 1), std::move(description));
            }
            add_table(node, "snt-primary", details);
            add_table(node, "snt-metadata", columns);
        }
    }

    if (!document.schemas.empty()) {
        auto& schemas = report.section("Schemas");
        schemas.paragraph("Each schema lists the evaluated parameters it supplied. A schema may also apply to a path without supplying every value on that path.");
        for (const auto& schema : document.schemas) {
            auto& node = add_heading(schemas, schema.name, latex);
            Rows details;
            add_row(details, "Description", schema.description);
            add_row(details, "Declared at", location(schema.origin));
            add_row(details, "Supplied parameters", joined(schema.supplied_parameters));
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
        if (!source.hash.empty()) {
            const auto& hash = source.hash;
            add_row(details, "Content hash", source.hash_algorithm + ": " +
                (format == ReportFormat::Json ? hash : displayed_hash(hash)));
        }
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
        renderer.table_column_spec("snt-primary", "@{}>{\\color{sntBlue}\\bfseries}w{l}{0.28\\linewidth}@{\\hspace{0.8em}}>{\\raggedright\\arraybackslash}p{0.67\\linewidth}@{}")
            .table_column_spec("snt-metadata", "@{}>{\\color{sntMeta}\\bfseries}w{l}{0.28\\linewidth}@{\\hspace{0.8em}}>{\\raggedright\\arraybackslash}p{0.67\\linewidth}@{}")
            .table_column_spec("snt-source", "@{}>{\\color{sntSource}\\bfseries}w{l}{0.28\\linewidth}@{\\hspace{0.8em}}>{\\raggedright\\arraybackslash}p{0.67\\linewidth}@{}");
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
