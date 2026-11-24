#include "model.h"

#include <snt/core/process.h>
#include <snt/dip/dip.h>
#include <snt/dip/inspect/inspector.h>
#include <snt/dip/report/docs.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>

namespace snt::dip::report {
    namespace {
        namespace fs = std::filesystem;

        std::string read_file(const fs::path& path) {
            std::ifstream input(path, std::ios::binary);
            if (!input)
                throw std::runtime_error("Cannot read " + path.string());
            return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        }

        std::pair<std::string, std::string> leading_source_text(const fs::path& path) {
            std::istringstream source(read_file(path));
            static const std::regex field(R"DIPL(^\s*\?(title|descr)\s+"((?:\\.|[^"\\])*)"\s*$)DIPL");
            std::string title, description, line;
            while (std::getline(source, line)) {
                if (line.empty() || line[0] == '#') continue;
                std::smatch match;
                if (!std::regex_match(line, match, field)) break;
                std::string value;
                const auto raw = match[2].str();
                for (size_t i = 0; i < raw.size(); ++i)
                    if (raw[i] == '\\' && i + 1 < raw.size()) {
                        const auto escaped = raw[++i];
                        value += escaped == 'n' ? '\n' : escaped;
                    } else value += raw[i];
                if (match[1] == "title") title = value;
                else description = value;
            }
            return {title, description};
        }

        void write_file(const fs::path& path, const std::string& content) {
            fs::create_directories(path.parent_path());
            std::ofstream output(path, std::ios::binary);
            if (!output || !(output << content))
                throw std::runtime_error("Cannot write " + path.string());
        }

        bool within(const fs::path& path, const fs::path& root) {
            const auto relative = path.lexically_relative(root);
            return !relative.empty() && *relative.begin() != "..";
        }

        std::string label(fs::path path) {
            auto name = path.filename().string();
            if (name.empty() || name == ".")
                return "Settings";
            if (name == "schemas") return "Schemas";
            for (auto& ch : name)
                if (ch == '_' || ch == '-') ch = ' ';
            if (!name.empty()) name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
            return name;
        }

        std::string escape_md(std::string value) {
            for (size_t pos = 0; (pos = value.find('`', pos)) != std::string::npos; pos += 2)
                value.insert(pos, "\\");
            return value;
        }

        std::string code(const std::string& value) { return "`" + escape_md(value) + "`"; }

        bool mentions_override(std::string description) {
            std::transform(description.begin(), description.end(), description.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
            return description.find("override") != std::string::npos;
        }

        bool has_local_links(const std::string& markdown) {
            static const std::regex link(R"(\]\(([^)]+)\))");
            for (std::sregex_iterator it(markdown.begin(), markdown.end(), link), end; it != end; ++it) {
                const auto target = (*it)[1].str();
                if (!target.empty() && target.front() != '#' && target.front() != '/' &&
                    target.find(':') == std::string::npos) return true;
            }
            return false;
        }

        std::string escape_html(const std::string& value) {
            std::string escaped;
            for (char ch : value) {
                if (ch == '&') escaped += "&amp;";
                else if (ch == '<') escaped += "&lt;";
                else if (ch == '>') escaped += "&gt;";
                else if (ch == '"') escaped += "&quot;";
                else escaped += ch;
            }
            return escaped;
        }

        std::string anchor(const std::string& value) {
            const char* digits = "0123456789abcdef";
            std::string id = "setting-";
            for (const unsigned char ch : value) {
                if (std::isalnum(ch)) id += static_cast<char>(std::tolower(ch));
                else { id += '-'; id += digits[ch >> 4]; id += digits[ch & 15]; }
            }
            return id;
        }

        std::string source_page(const fs::path& relative) { return relative.generic_string() + ".md"; }
        std::string directory_page(const fs::path& relative) {
            return (relative / "index.md").generic_string();
        }

        std::string link_from(const fs::path& from_page, const fs::path& to_page, bool html) {
            auto relative = to_page.lexically_relative(from_page.parent_path());
            if (html) relative.replace_extension(".html");
            return relative.generic_string();
        }

        fs::path source_relative(const fs::path& path, const fs::path& root, const fs::path& project_dir) {
            if (path.empty()) return {};
            if (path.is_absolute()) return path.lexically_relative(root);
            if (fs::exists(root / project_dir / path)) return (project_dir / path).lexically_normal();
            if (fs::exists(root / path)) return path;
            return (project_dir / path).lexically_normal();
        }

        std::string parent_setting(const std::string& path) {
            const auto dot = path.rfind('.');
            return dot == std::string::npos ? std::string{} : path.substr(0, dot);
        }

        std::string shared_setting_parent(std::string first, const std::string& second) {
            while (!first.empty() && second != first && second.rfind(first + ".", 0) != 0)
                first = parent_setting(first);
            return first;
        }

        void collect_overrides(const fs::path& manifest, std::set<fs::path>& visited, std::set<fs::path>& overrides) {
            const auto absolute = fs::absolute(manifest).lexically_normal();
            if (!visited.insert(absolute).second) return;
            const auto project = parse_project_manifest(absolute);
            if (project.hierarchy.has_collection("overrides")) {
                for (const auto& id : project.hierarchy.get_collection("overrides").items)
                    overrides.insert(fs::absolute(absolute.parent_path() /
                        project["overrides[" + id + "].file"].as<std::string>()).lexically_normal());
            }
            if (project.hierarchy.has_collection("diplfile")) {
                for (const auto& id : project.hierarchy.get_collection("diplfile").items)
                    collect_overrides(absolute.parent_path() /
                        project["diplfile[" + id + "].file"].as<std::string>(), visited, overrides);
            }
        }

        // README links are source-relative. Resolve links to generated pages when those pages exist.
        std::string rewrite_links(
            const std::string& markdown, const fs::path& source_file, const fs::path& output_page,
            const fs::path& root, const fs::path& output, const std::set<fs::path>& pages,
            std::map<fs::path, fs::path>& extra_markdown, bool html, DocsResult& result
        ) {
            const std::regex target_pattern(R"(\]\(([^)]+)\))");
            std::string result_text;
            auto cursor = markdown.cbegin();
            std::smatch match;
            while (std::regex_search(cursor, markdown.cend(), match, target_pattern)) {
                result_text.append(cursor, match[0].first);
                const std::string target = match[1].str();
                const auto hash = target.find('#');
                const auto path_part = target.substr(0, hash);
                const auto fragment = hash == std::string::npos ? "" : target.substr(hash);
                std::string rewritten = target;
                if (!path_part.empty() && path_part[0] != '#' && path_part[0] != '/' &&
                    path_part.find(':') == std::string::npos) {
                    const auto absolute = fs::weakly_canonical(source_file.parent_path() / path_part);
                    if (within(absolute, root)) {
                        const auto relative = absolute.lexically_relative(root);
                        fs::path target_page;
                        if (absolute.filename() == "README.md" || fs::is_directory(absolute))
                            target_page = directory_page(absolute.filename() == "README.md" ? relative.parent_path() : relative);
                        else if (absolute.filename() == "DIPfile" || absolute.extension() == ".dip" ||
                                 absolute.extension() == ".dipl")
                            target_page = source_page(relative);
                        else if (absolute.extension() == ".md")
                            target_page = relative;
                        if (!target_page.empty() && pages.count(target_page))
                            rewritten = link_from(output_page, target_page, html) + fragment;
                        else if (!target_page.empty())
                            result.warnings.push_back("No generated page for " + absolute.string());
                        else if (fs::is_regular_file(absolute)) {
                            fs::create_directories((output / relative).parent_path());
                            fs::copy_file(absolute, output / relative, fs::copy_options::overwrite_existing);
                            rewritten = link_from(output_page, relative, false) + fragment;
                        } else result.warnings.push_back("Unresolved README link: " + target);
                    } else if (within(absolute, root.parent_path()) && fs::is_regular_file(absolute)) {
                        const auto copy = fs::path("_project") / absolute.lexically_relative(root.parent_path());
                        fs::create_directories((output / copy).parent_path());
                        if (!extra_markdown.count(copy))
                            fs::copy_file(absolute, output / copy, fs::copy_options::overwrite_existing);
                        if (copy.extension() == ".md") {
                            extra_markdown.emplace(copy, absolute);
                            auto linked = copy;
                            if (html) linked.replace_extension(".html");
                            rewritten = link_from(output_page, linked, false) + fragment;
                        } else rewritten = link_from(output_page, copy, false) + fragment;
                    } else {
                        result.warnings.push_back("Unresolved README link: " + target);
                    }
                }
                result_text += "](" + rewritten + ")";
                cursor = match[0].second;
            }
            result_text.append(cursor, markdown.cend());
            return result_text;
        }

        struct FileContents {
            struct Entry {
                std::string path, title, description, url;
                size_t fields = 0;
            };
            std::vector<const Parameter*> parameters;
            std::vector<const SchemaDefinitionInspection*> schemas;
            std::vector<std::string> registrations;
            std::vector<Entry> entries;
            std::string title, description;
        };

        struct SearchEntry {
            std::string path, description, scope, category, visibility;
            fs::path page;
        };

        std::string collection_item(const std::string& path) {
            const auto close = path.rfind(']');
            if (close == std::string::npos || close + 1 >= path.size() || path[close + 1] != '.') return {};
            const auto open = path.rfind('[', close);
            if (open == std::string::npos) return {};
            return path.substr(0, close + 1);
        }

        std::string entry_anchor(const std::string& path) { return anchor("entry:" + path); }

        void collect_guided_members(const std::vector<SchemaMemberInspection>& members,
                                    std::set<std::string>& guided) {
            for (const auto& member : members) {
                if (member.kind == "value" && member.setup) guided.insert(member.relative_path);
                collect_guided_members(member.members, guided);
            }
        }

        void schema_members(std::ostringstream& out, const std::string& schema,
                            const std::vector<SchemaMemberInspection>& members) {
            for (const auto& member : members) {
                if (member.kind == "value") {
                    out << "<a id=\"" << anchor(schema + "." + member.relative_path) << "\"></a>\n\n";
                    out << "### " << code(member.relative_path) << "\n\n";
                    if (!member.metadata.description.empty()) out << member.metadata.description << "\n\n";
                    out << "- **Type:** " << code(member.type.empty() ? "inferred" : member.type) << "\n";
                    if (!member.units.empty()) out << "- **Units:** " << code(member.units) << "\n";
                    if (!member.default_value.empty()) out << "- **Default:** " << code(member.default_value) << "\n";
                    if (member.setup) out << "- **Guided setup:** yes\n";
                    if (!member.options.empty()) {
                        out << "- **Choices:** ";
                        for (size_t i = 0; i < member.options.size(); ++i) {
                            if (i) out << ", ";
                            out << code(member.options[i]);
                            if (i < member.option_labels.size()) out << " (" << member.option_labels[i] << ")";
                        }
                        out << "\n";
                    }
                    if (!member.condition.empty()) out << "- **Validation:** " << code(member.condition) << "\n";
                    for (const auto& example : member.metadata.example)
                        out << "- **Example:** " << code(example) << "\n";
                    out << "\n";
                }
                schema_members(out, schema, member.members);
            }
        }

        void collect_schema_search(
            const std::string& schema, const std::vector<SchemaMemberInspection>& members,
            const fs::path& page, std::vector<SearchEntry>& searchable
        ) {
            for (const auto& member : members) {
                if (member.kind == "value")
                    searchable.push_back({schema + "." + member.relative_path,
                                          member.metadata.description, "schema", member.metadata.category,
                                          member.metadata.visibility, page});
                collect_schema_search(schema, member.members, page, searchable);
            }
        }

        std::string page_header(const fs::path& relative, const std::string& title, bool html) {
            std::ostringstream out;
            out << "[All settings](" << link_from(relative, "index.md", html) << ")";
            out << " · [Search](" << link_from(relative, "search.md", html) << ")";
            auto parent = relative.parent_path();
            std::vector<fs::path> ancestors;
            while (!parent.empty() && parent != ".") {
                ancestors.push_back(parent);
                parent = parent.parent_path();
            }
            std::reverse(ancestors.begin(), ancestors.end());
            for (const auto& ancestor : ancestors)
                out << " / [" << label(ancestor) << "](" << link_from(relative, directory_page(ancestor), html) << ")";
            out << "\n\n# " << title << "\n\n";
            return out.str();
        }

        void render_html(const fs::path& markdown, const fs::path& output, const fs::path& css) {
            const auto result = core::process({"pandoc", "--from=gfm", "--to=html5", "--standalone",
                                               "--metadata", "title=Settings reference", "--css=" + css.string(),
                                               "--output=" + output.string(), markdown.string()});
            if (result.status)
                throw std::runtime_error("pandoc failed for " + markdown.string() + ": " + result.output);
        }

        std::string book_id(const fs::path& page, const std::string& fragment = {}) {
            return anchor(page.generic_string()) + (fragment.empty() ? "" : "-" + fragment);
        }

        std::string book_links(
            const std::string& line, const fs::path& page, const std::set<fs::path>& pages,
            const std::map<fs::path, fs::path>& extras, bool summary
        ) {
            static const std::regex target_pattern(R"(\]\(([^)]+)\))");
            std::string result;
            auto cursor = line.cbegin();
            std::smatch match;
            while (std::regex_search(cursor, line.cend(), match, target_pattern)) {
                result.append(cursor, match[0].first);
                const auto original = match[1].str();
                const auto hash = original.find('#');
                const auto path_part = original.substr(0, hash);
                const auto fragment = hash == std::string::npos ? "" : original.substr(hash + 1);
                std::string target = original;
                if (original.find(':') == std::string::npos && (path_part.empty() || path_part[0] != '/')) {
                    auto destination = path_part.empty() ? page : (page.parent_path() / path_part).lexically_normal();
                    if (destination.extension() == ".html") destination.replace_extension(".md");
                    if (pages.count(destination) || extras.count(destination))
                        target = "#" + book_id(destination, summary && pages.count(destination) &&
                            fragment.rfind("setting-entry", 0) != 0 ? "" : fragment);
                    else if (!path_part.empty())
                        target = destination.generic_string() + (fragment.empty() ? "" : "#" + fragment);
                }
                result += "](" + target + ")";
                cursor = match[0].second;
            }
            result.append(cursor, line.cend());
            return result;
        }

        void append_book_page(
            std::ostringstream& book, const fs::path& output, const fs::path& page, unsigned level,
            const std::set<fs::path>& pages, const std::map<fs::path, fs::path>& extras,
            bool directory, bool overview, bool summary, bool skip_navigation = true
        ) {
            std::istringstream input(read_file(output / page));
            std::string line;
            bool first_line = true, first_heading = true, fenced = false, skip_explore = false;
            static const std::regex heading(R"(^(#{1,6})[ \t]+(.+)$)");
            static const std::regex named_anchor(R"REGEX(^<a id="([^"]+)"></a>$)REGEX");
            while (std::getline(input, line)) {
                if (first_line) {
                    first_line = false;
                    if (skip_navigation) continue;
                }
                if (overview && line == "## Explore the reference") { skip_explore = true; continue; }
                if (skip_explore) {
                    if (line != "## Examples to explore") continue;
                    skip_explore = false;
                }
                if (directory && line == "## Contents") break;
                if (line.find("[Browse every guided setting]") != std::string::npos) continue;
                if (summary && !directory &&
                    (line == "## Evaluated settings" || line.rfind("## Schema ", 0) == 0)) break;
                if (line.rfind("```", 0) == 0 || line.rfind("~~~", 0) == 0) fenced = !fenced;
                std::smatch match;
                if (!fenced && std::regex_match(line, match, heading)) {
                    auto text = match[2].str();
                    if (first_heading && overview) text = "Overview";
                    const auto actual_level = std::min<unsigned>(6, level + match[1].str().size() - 1);
                    book << std::string(actual_level, '#') << ' ' << text;
                    if (first_heading) book << " {#" << book_id(page) << '}';
                    else if (text == "Evaluated settings")
                        book << " {#" << book_id(page, "evaluated-settings") << " .unnumbered .unlisted}";
                    else if (text.rfind("Schema ", 0) == 0)
                        book << " {.unnumbered .unlisted}";
                    book << "\n";
                    first_heading = false;
                } else if (!fenced && std::regex_match(line, match, named_anchor)) {
                    book << "[]{#" << book_id(page, match[1].str()) << "}\n";
                } else {
                    auto rewritten = fenced ? line : book_links(line, page, pages, extras, summary);
                    if (!fenced) {
                        static const std::regex inline_anchor(R"REGEX(<a id="([^"]+)"></a>)REGEX");
                        std::smatch inline_match;
                        std::string converted;
                        auto cursor = rewritten.cbegin();
                        while (std::regex_search(cursor, rewritten.cend(), inline_match, inline_anchor)) {
                            converted.append(cursor, inline_match[0].first);
                            converted += "[]{#" + book_id(page, inline_match[1].str()) + "}";
                            cursor = inline_match[0].second;
                        }
                        converted.append(cursor, rewritten.cend());
                        rewritten = std::move(converted);
                    }
                    book << rewritten << "\n";
                }
            }
            book << "\n\n";
        }

        void render_pdf_book(
            const fs::path& output, const std::string& title, const std::set<fs::path>& directories,
            const std::map<fs::path, FileContents>& files, const std::set<fs::path>& pages,
            const std::map<fs::path, fs::path>& extras, const std::string& engine,
            const fs::path& cheatsheet_pdf, bool full_reference
        ) {
            std::ostringstream book;
            std::function<void(const fs::path&, unsigned)> visit = [&](const fs::path& dir, unsigned level) {
                const auto index = fs::path(directory_page(dir));
                append_book_page(book, output, index, level, pages, extras, true, dir.empty(), !full_reference);
                for (const auto& [file, _] : files)
                    if (file.parent_path() == dir)
                        append_book_page(book, output, source_page(file), level + 1, pages, extras,
                                         false, false, !full_reference);
                for (const auto& child : directories)
                    if (child != dir && child.parent_path() == dir)
                        visit(child, dir.empty() ? 1 : level + 1);
            };
            visit({}, 1);
            for (const auto& [page, _] : extras)
                append_book_page(book, output, page, 1, pages, extras, false, false, !full_reference, false);
            if (!cheatsheet_pdf.empty()) {
                if (!fs::is_regular_file(cheatsheet_pdf))
                    throw std::runtime_error("Cannot read DIPL cheat sheet PDF: " + cheatsheet_pdf.string());
                const auto appendix = fs::absolute(output / "settings-reference-cheatsheet.pdf").lexically_normal();
                if (fs::absolute(cheatsheet_pdf).lexically_normal() != appendix)
                    fs::copy_file(cheatsheet_pdf, appendix, fs::copy_options::overwrite_existing);
                book << "\n\\clearpage\n"
                     << "\\includepdf[pages=-,pagecommand={},addtotoc={1,chapter,1,DIPL and PUEL quick reference,dipl-cheatsheet}]"
                     << "{\\detokenize{" << appendix.generic_string() << "}}\n";
            }
            const auto manuscript = output / "settings-reference.md";
            write_file(manuscript, book.str());
            const auto header = output / "settings-reference-style.tex";
            write_file(header, R"TEX(\usepackage{xcolor}
\usepackage{hyperref}
\usepackage{pdfpages}
\usepackage{microtype}
\usepackage{fancyhdr}
\definecolor{SNTBlue}{HTML}{204B70}
\definecolor{SNTTeal}{HTML}{246B62}
\makeatletter
\renewcommand{\maketitle}{%
  \begin{titlepage}\raggedright
  \vspace*{0.13\textheight}
  {\sffamily\bfseries\large\color{SNTTeal}DIPL / SETTINGS HANDBOOK\par}
  \vspace{1.9cm}
  {\sffamily\bfseries\fontsize{34}{40}\selectfont\color{SNTBlue}\@title\par}
  \vspace{1.2cm}{\color{SNTTeal}\rule{0.72\textwidth}{1.2pt}\par}
  \vfill
  {\sffamily\small Generated from registered DIPfile projects\par}
  \end{titlepage}}
\makeatother
\setkomafont{chapter}{\sffamily\bfseries\color{SNTBlue}\Huge}
\setkomafont{section}{\sffamily\bfseries\color{SNTBlue}\Large}
\setkomafont{subsection}{\sffamily\bfseries\color{SNTTeal}\large}
\hypersetup{colorlinks=true,linkcolor=SNTBlue,urlcolor=SNTTeal}
\pagestyle{fancy}
\fancyhf{}
\fancyhead[LE,RO]{\small\nouppercase{\leftmark}}
\fancyfoot[C]{\thepage}
\setlength{\headheight}{16pt}
\emergencystretch=3em
)TEX");
            const auto temporary = output / "settings-reference.tmp.pdf";
            std::error_code ignored;
            fs::remove(temporary, ignored);
            const auto result = core::process({
                "pandoc", "--from=markdown+bracketed_spans+fenced_code_attributes", "--standalone",
                "--toc", full_reference ? "--toc-depth=3" : "--toc-depth=2",
                "--number-sections", "--top-level-division=chapter",
                "--pdf-engine=" + engine, "--resource-path=" + output.string(),
                "--metadata=title:" + title, "--metadata=subtitle:DIPL settings reference",
                "--variable=documentclass:scrreprt", "--variable=geometry:margin=24mm",
                "--variable=fontsize:10pt", "--variable=secnumdepth:2",
                "--include-in-header=" + header.string(),
                "--output=" + temporary.string(), manuscript.string()
            });
            if (result.status) {
                fs::remove(temporary, ignored);
                throw std::runtime_error("PDF compilation failed (" + engine + "): " + result.output);
            }
            fs::copy_file(temporary, output / "settings-reference.pdf", fs::copy_options::overwrite_existing);
            fs::remove(temporary, ignored);
        }

        std::string contents_page(
            const fs::path& page, const FileContents& contents, const std::string& title, bool html,
            const std::map<std::string, fs::path>& schema_pages,
            const std::map<std::string, fs::path>& setting_pages,
            const std::set<std::string>& override_targets
        ) {
            std::ostringstream out;
            out << page_header(page, title, html);
            if (!contents.registrations.empty()) {
                out << "**Registered as:** ";
                for (size_t i = 0; i < contents.registrations.size(); ++i) {
                    if (i) out << ", ";
                    out << contents.registrations[i];
                }
                out << "\n\n";
            }
            if (!contents.description.empty()) out << contents.description << "\n\n";
            if (!contents.entries.empty()) {
                out << "## Entries\n\nAll " << contents.entries.size() << " entries are listed below.\n\n";
                for (const auto& entry : contents.entries) {
                    out << "- <a id=\"" << entry_anchor(entry.path) << "\"></a>**"
                        << (entry.title.empty() ? code(entry.path) : entry.title) << "**";
                    if (!entry.title.empty()) out << " (" << code(entry.path) << ")";
                    if (!entry.description.empty()) out << " — " << entry.description;
                    out << " (" << entry.fields << " fields)";
                    if (!entry.url.empty()) out << " · [Source](<" << entry.url << ">)";
                    out << "\n";
                }
                out << "\n";
            }
            if (!contents.parameters.empty()) {
                const bool small = contents.parameters.size() <= 8;
                std::vector<const Parameter*> selected;
                for (const auto* item : contents.parameters) {
                    if (small || item->setup || item->metadata.visibility == "basic")
                        selected.push_back(item);
                }
                out << "## Settings overview\n\n";
                if (small) out << "All " << contents.parameters.size() << " evaluated settings are listed below.\n\n";
                else out << selected.size() << " of " << contents.parameters.size()
                         << " evaluated settings are shown below (guided setup or `?visibility \"basic\"`). "
                         << "[View all settings](#evaluated-settings).\n\n";
                for (const auto* item : selected) {
                    out << "- " << code(item->path);
                    if (item->link_target.empty())
                        out << " = " << code(item->value + (item->units.empty() ? "" : " " + item->units));
                    if (!item->metadata.description.empty()) out << " — " << item->metadata.description;
                    out << "\n";
                }
                out << "\n";
            }
            for (const auto* schema : contents.schemas) {
                out << "## Schema " << code(schema->name) << "\n\n";
                if (!schema->metadata.description.empty()) out << schema->metadata.description << "\n\n";
                schema_members(out, schema->name, schema->members);
            }
            if (!contents.parameters.empty()) out << "## Evaluated settings\n\n";
            for (const auto* item : contents.parameters) {
                out << "<a id=\"" << anchor(item->path) << "\"></a>\n\n";
                out << "### " << code(item->path) << "\n\n";
                if (!item->metadata.description.empty()) out << item->metadata.description << "\n\n";
                if (item->link_target.empty())
                    out << "- **Value:** " << code(item->value + (item->units.empty() ? "" : " " + item->units)) << "\n";
                else {
                    const auto target = item->link_source.empty() ? item->link_target
                                                                   : item->link_source + "?" + item->link_target;
                    auto found = setting_pages.find(item->link_target);
                    if (found == setting_pages.end())
                        found = std::find_if(setting_pages.begin(), setting_pages.end(), [&](const auto& entry) {
                            return entry.first.rfind(item->link_target + ".", 0) == 0;
                        });
                    out << "- **Links to:** ";
                    if (found != setting_pages.end())
                        out << "[" << code(target) << "](" << link_from(page, found->second, html) << ")\n";
                    else out << code(target) << "\n";
                }
                out << "- **Type:** " << code(item->link_target.empty() ? item->type : "link") << "\n";
                if (!item->options.empty()) {
                    out << "- **Choices:** ";
                    for (size_t i = 0; i < item->options.size(); ++i) {
                        if (i) out << ", ";
                        out << code(item->options[i]);
                    }
                    out << "\n";
                }
                out << "\n";
                if (html) out << "<details class=\"technical\"><summary>Validation and provenance</summary>\n\n";
                if (!item->condition.empty()) out << "- **Validation:** " << code(item->condition) << "\n";
                if (!item->contributing_schema.empty()) {
                    const auto found = schema_pages.find(item->contributing_schema);
                    out << "- **Schema:** ";
                    if (found != schema_pages.end())
                        out << "[" << code(item->contributing_schema) << "](" <<
                            link_from(page, found->second, html) << ")\n";
                    else out << code(item->contributing_schema) << "\n";
                }
                if (item->declaration.line)
                    out << "- **" << (item->contributing_schema.empty() ? "Declared at" : "Declared in schema")
                        << ":** " << code(item->declaration.path + ":" +
                           std::to_string(item->declaration.line)) << "\n";
                if (!item->modifications.empty()) {
                    const auto& assignment = item->modifications.back();
                    out << "- **Value assigned in:** " << code(assignment.path + ":" +
                           std::to_string(assignment.line)) << "\n";
                }
                if (item->overridden)
                    out << "- **Override applied in:** " << code(item->replacement.path + ":" +
                           std::to_string(item->replacement.line)) << "\n";
                if (html) out << "\n</details>\n";
                out << "\n";
                if (override_targets.count(item->path) &&
                    (item->setup || mentions_override(item->metadata.description)) &&
                    item->link_target.empty() && item->shape.empty() && item->value.size() < 200 &&
                    item->value.find('\n') == std::string::npos) {
                    const bool needs_adaptation = item->type == "str" || item->value.find('/') != std::string::npos ||
                                                  item->value.find('\\') != std::string::npos;
                    if (needs_adaptation) {
                        out << "**Override guidance:** Assign a value suitable for your environment to "
                            << code(item->path) << " in a registered override source.\n\n";
                    } else {
                        const auto assignment = item->path + " = " + item->value +
                            (item->units.empty() ? "" : " " + item->units);
                        if (html)
                            out << "<details class=\"override\"><summary>Override syntax using the current value</summary>\n\n"
                                << "Change the value as needed.\n\n<pre><code>" << escape_html(assignment)
                                << "</code></pre>\n\n</details>\n\n";
                        else
                            out << "Override syntax using the current value (change as needed):\n\n```dipl\n"
                                << assignment << "\n```\n\n";
                    }
                }
            }
            return out.str();
        }
    } // namespace

    DocsResult generate_docs(const DocsOptions& options) {
        namespace fs = std::filesystem;
        if (options.root.empty() || options.output_dir.empty())
            throw std::invalid_argument("Documentation needs a source root and output directory.");
        if (!options.cheatsheet_pdf.empty() && !options.pdf)
            throw std::invalid_argument("A cheat sheet PDF can be appended only to documentation PDF output.");
        const auto root = fs::canonical(options.root);
        const auto output = fs::absolute(options.output_dir).lexically_normal();
        if (!fs::is_directory(root)) throw std::invalid_argument("Documentation root is not a directory.");
        if (within(output, root)) throw std::invalid_argument("Output directory must be outside the source root.");

        DocsResult result;
        std::vector<fs::path> projects;
        for (fs::recursive_directory_iterator it(root), end; it != end; ++it) {
            if (it->is_directory()) {
                const auto name = it->path().filename().string();
                if (!name.empty() && name[0] == '.') it.disable_recursion_pending();
            } else if (it->path().filename() == "DIPfile") {
                projects.push_back(it->path());
            }
        }
        std::sort(projects.begin(), projects.end());
        if (projects.empty()) throw std::runtime_error("No DIPfile found under " + root.string());
        result.projects = projects.size();

        std::map<fs::path, FileContents> files;
        std::set<fs::path> directories{{}};
        std::vector<SearchEntry> searchable;
        std::set<fs::path> known_overrides;
        for (const auto& manifest : projects) {
            auto project_dir = manifest.parent_path().lexically_relative(root);
            if (project_dir == ".") project_dir.clear();
            std::set<fs::path> overrides;
            std::set<fs::path> visited;
            collect_overrides(manifest, visited, overrides);
            known_overrides.insert(overrides.begin(), overrides.end());
            DIP parser;
            parser.set_project_file_reader([overrides](const fs::path& requested) {
                if (overrides.count(fs::absolute(requested).lexically_normal())) return std::string{};
                return read_file(requested);
            });
            parser.add_project(manifest);
            const auto env = parser.parse(false);
            auto document = build_document(env, {}, {}, {}, {}, {}, {}, root);
            const Inspector inspector(env);
            const auto hierarchy = inspector.schema_hierarchy();
            std::set<fs::path> schema_sources;
            for (const auto& definition : hierarchy.definitions) {
                const auto source = hierarchy.sources.find(definition.origin.source);
                if (source != hierarchy.sources.end())
                    schema_sources.insert(source_relative(source->second.path, root, project_dir));
            }
            std::map<fs::path, std::vector<std::string>> source_hints;
            for (const auto& parameter : document.parameters) {
                const auto declared = source_relative(parameter.declaration.path, root, project_dir);
                if (!declared.empty() && !schema_sources.count(declared))
                    source_hints[declared].push_back(parameter.path);
                else if (!parameter.modifications.empty()) {
                    const auto source = source_relative(parameter.modifications.front().path, root, project_dir);
                    if (!source.empty() && !schema_sources.count(source))
                        source_hints[source].push_back(parameter.path);
                }
            }
            std::map<std::string, fs::path> setting_pages;
            for (const auto& parameter : document.parameters) {
                fs::path owner = source_relative(parameter.declaration.path, root, project_dir);
                if (!parameter.modifications.empty())
                    owner = source_relative(parameter.modifications.back().path, root, project_dir);
                else if (schema_sources.count(owner)) {
                    size_t best_depth = 0, best_votes = 0;
                    for (const auto& [source, hints] : source_hints) {
                        size_t depth = 0, votes = 0;
                        for (const auto& hint : hints) {
                            const auto common = shared_setting_parent(parent_setting(parameter.path), hint);
                            if (common.empty()) continue;
                            if (common.size() > depth) { depth = common.size(); votes = 0; }
                            if (common.size() == depth) ++votes;
                        }
                        if (depth > best_depth || (depth == best_depth && votes > best_votes)) {
                            owner = source;
                            best_depth = depth;
                            best_votes = votes;
                        }
                    }
                }
                setting_pages[parameter.path] = source_page(owner);
            }
            std::map<std::string, std::set<std::string>> guided_members;
            for (const auto& definition : hierarchy.definitions)
                collect_guided_members(definition.members, guided_members[definition.name]);
            for (auto& parameter : document.parameters)
                if (!parameter.setup && !parameter.contributing_schema.empty()) {
                    const auto found = guided_members.find(parameter.contributing_schema);
                    if (found == guided_members.end()) continue;
                    for (const auto& member : found->second)
                        if (parameter.path.size() > member.size() &&
                            parameter.path.compare(parameter.path.size() - member.size(), member.size(), member) == 0 &&
                            parameter.path[parameter.path.size() - member.size() - 1] == '.') {
                            parameter.setup = true;
                            break;
                        }
                }
            std::map<std::string, fs::path> schema_pages;
            for (const auto& definition : hierarchy.definitions) {
                const auto source = hierarchy.sources.find(definition.origin.source);
                if (source != hierarchy.sources.end())
                    schema_pages[definition.name] = source_page(source_relative(source->second.path, root, project_dir));
            }
            const auto manifest_relative = manifest.lexically_relative(root);
            files[manifest_relative].registrations.push_back("project manifest");
            directories.insert(project_dir);
            for (const auto& entry : env.project_entries()) {
                if (entry.resolved_path.empty() || entry.kind == ProjectEntry::Kind::Override) continue;
                const auto absolute = fs::absolute(entry.resolved_path).lexically_normal();
                if (!within(absolute, root)) {
                    result.warnings.push_back("Registered source outside documentation root: " + absolute.string());
                    continue;
                }
                const auto relative = absolute.lexically_relative(root);
                const char* kind = entry.kind == ProjectEntry::Kind::Schema ? "schema" :
                                   entry.kind == ProjectEntry::Kind::Source ? "named source" :
                                   entry.kind == ProjectEntry::Kind::Include ? "included project" : "code";
                files[relative].registrations.push_back(entry.name.empty() ? kind : std::string(kind) + " " + code(entry.name));
                for (auto dir = relative.parent_path(); !dir.empty() && dir != "."; dir = dir.parent_path())
                    directories.insert(dir);
            }
            for (const auto& source : env.get_source_manifest()) {
                const auto relative = source_relative(source.path, root, project_dir);
                const auto found = files.find(relative);
                if (found == files.end()) continue;
                if (found->second.title.empty()) found->second.title = source.metadata.title;
                if (found->second.description.empty()) found->second.description = source.metadata.description;
            }
            for (const auto& [_, source] : env.sources.entries()) {
                const auto relative = source_relative(source.path, root, project_dir);
                const auto found = files.find(relative);
                if (found == files.end()) continue;
                if (found->second.title.empty()) found->second.title = source.metadata.title;
                if (found->second.description.empty()) found->second.description = source.metadata.description;
            }
            for (auto& [relative, contents] : files) {
                if (!within(root / relative, manifest.parent_path()) || !fs::is_regular_file(root / relative)) continue;
                if (!contents.title.empty() && !contents.description.empty()) continue;
                const auto [title, description] = leading_source_text(root / relative);
                if (contents.title.empty()) contents.title = title;
                if (contents.description.empty()) contents.description = description;
            }
            // The model above is temporary; render this project's value pages below while it lives.
            for (auto& [relative, contents] : files) {
                if (!within(root / relative, manifest.parent_path())) continue;
                contents.parameters.clear();
                contents.entries.clear();
                for (const auto& parameter : document.parameters)
                    if (setting_pages.at(parameter.path) == fs::path(source_page(relative)))
                        contents.parameters.push_back(&parameter);
                std::map<std::string, FileContents::Entry> entries;
                for (const auto* parameter : contents.parameters) {
                    const auto item = collection_item(parameter->path);
                    if (item.empty()) continue;
                    auto& entry = entries[item];
                    entry.path = item;
                    ++entry.fields;
                    const auto leaf = parameter->path.substr(item.size() + 1);
                    if (leaf == "name" || leaf == "title" || leaf == "label") {
                        entry.title = parameter->value;
                        if (entry.title.size() >= 2 && entry.title.front() == '"' && entry.title.back() == '"')
                            entry.title = entry.title.substr(1, entry.title.size() - 2);
                    }
                }
                for (auto& [_, entry] : entries) {
                    try {
                        const auto described = inspector.describe(entry.path, 0);
                        entry.description = described.metadata.description;
                        entry.url = described.metadata.url;
                    } catch (const std::exception&) {
                        // The evaluated fields remain useful when the parent item has no inspectable metadata.
                    }
                    contents.entries.push_back(std::move(entry));
                }
                if (contents.entries.size() == 1 && contents.title.empty())
                    contents.title = contents.entries.front().title;
                for (const auto& definition : hierarchy.definitions) {
                    const auto source = hierarchy.sources.find(definition.origin.source);
                    if (source != hierarchy.sources.end() &&
                        source_relative(source->second.path, root, project_dir) == relative)
                        contents.schemas.push_back(&definition);
                }
                if (contents.description.empty() && !contents.schemas.empty())
                    contents.description = contents.schemas.front()->metadata.description;
                if (contents.description.empty() && contents.entries.size() == 1)
                    contents.description = contents.entries.front().description;
                const auto page = fs::path(source_page(relative));
                for (const auto* parameter : contents.parameters)
                    searchable.push_back({parameter->path, parameter->metadata.description,
                                          std::string(overrides.empty() ? "no-override" : "override-capable") +
                                          (parameter->setup ? " guided" : "") +
                                          (collection_item(parameter->path).empty() ? "" : " collection"),
                                          parameter->metadata.category, parameter->metadata.visibility, page});
                for (const auto& entry : contents.entries)
                    searchable.push_back({entry.path, entry.description.empty() ? contents.description : entry.description,
                                          "entry", "", "", page});
                for (const auto* definition : contents.schemas)
                    collect_schema_search(definition->name, definition->members, page, searchable);
                if (!contents.description.empty())
                    searchable.push_back({contents.title.empty() ? relative.filename().string() : contents.title,
                                          contents.description, "file", "", "", page});
                std::set<std::string> override_targets;
                if (!overrides.empty())
                    for (const auto* parameter : contents.parameters)
                        if (parameter->link_target.empty() && parameter->shape.empty() &&
                            (parameter->setup || mentions_override(parameter->metadata.description)) &&
                            inspector.override_contract(parameter->path).kind == OverrideTargetKind::ExistingValue)
                            override_targets.insert(parameter->path);
                write_file(output / page, contents_page(page, contents,
                                                         contents.title.empty() ? relative.filename().string() : contents.title,
                                                         options.html,
                                                         schema_pages, setting_pages, override_targets));
                contents.parameters.clear();
                contents.schemas.clear();
            }
        }

        std::set<fs::path> pages;
        for (const auto& [file, _] : files) pages.insert(source_page(file));
        for (const auto& dir : directories) pages.insert(directory_page(dir));
        pages.insert("search.md");
        std::map<fs::path, fs::path> extra_markdown;

        for (fs::recursive_directory_iterator it(root), end; it != end; ++it) {
            if (it->is_directory()) {
                const auto name = it->path().filename().string();
                if (!name.empty() && name[0] == '.') it.disable_recursion_pending();
            } else if ((it->path().extension() == ".dip" || it->path().extension() == ".dipl") &&
                       !files.count(it->path().lexically_relative(root)) &&
                       !known_overrides.count(fs::absolute(it->path()).lexically_normal())) {
                result.warnings.push_back("DIPL file is not registered by a DIPfile: " + it->path().string());
            }
        }

        {
            std::sort(searchable.begin(), searchable.end(), [](const SearchEntry& left, const SearchEntry& right) {
                if (left.path != right.path) return left.path < right.path;
                return left.scope < right.scope;
            });
            std::ostringstream out;
            out << page_header("search.md", "Setting index", options.html);
            if (options.html)
                out << "<label for=\"setting-filter\">Search paths and descriptions</label> "
                       "<input id=\"setting-filter\" type=\"search\" placeholder=\"Type a term or exact path\"> "
                       "<label for=\"setting-scope\">Show</label> "
                       "<select id=\"setting-scope\">"
                       "<option value=\"all\">All</option>"
                       "<option value=\"guided\">Guided setup</option>"
                       "<option value=\"override-capable\">Override-capable values</option>"
                       "<option value=\"entry\">Collection entries</option>"
                       "<option value=\"collection\">Collection fields</option>"
                       "<option value=\"schema\">Schemas</option>"
                       "<option value=\"no-override\">Values without registered overrides</option>"
                       "<option value=\"file\">Files</option>"
                       "</select><p id=\"search-count\" aria-live=\"polite\"></p>\n\n";
            if (options.html) out << "<ul id=\"setting-list\">\n";
            for (const auto& entry : searchable) {
                const auto fragment = entry.scope == "file" ? "" :
                                      "#" + (entry.scope == "entry" ? entry_anchor(entry.path) : anchor(entry.path));
                const auto href = link_from("search.md", entry.page, options.html) + fragment;
                const auto note = entry.description +
                    (entry.category.empty() ? "" : " · Category: " + entry.category) +
                    (entry.visibility.empty() ? "" : " · Visibility: " + entry.visibility);
                if (options.html)
                    out << "<li data-scope=\"" << entry.scope << "\"><a href=\"" << href
                        << "\">" << escape_html(entry.path) << "</a>"
                        << (note.empty() ? "" : "<span class=\"search-description\">" +
                            escape_html(note) + "</span>") << "</li>\n";
                else
                    out << "- [" << code(entry.path) << "](" << href << ")"
                        << (note.empty() ? "" : " — " + note) << "\n";
            }
            if (options.html) out << "</ul>\n";
            if (options.html)
                out << "<script>(()=>{const q=document.getElementById('setting-filter');"
                       "const scope=document.getElementById('setting-scope');"
                       "const rows=[...document.querySelectorAll('#setting-list li')];"
                       "const requested=new URLSearchParams(location.search).get('scope');"
                       "if(requested&&[...scope.options].some(x=>x.value===requested))scope.value=requested;"
                       "function update(){const term=q.value.toLowerCase();let shown=0;let matches=0;"
                       "for(const row of rows){const ok=(scope.value==='all'||row.dataset.scope.split(' ').includes(scope.value))"
                       "&&row.textContent.toLowerCase().includes(term);if(ok)matches++;"
                       "row.hidden=!ok||(!term&&shown>=80);if(ok)shown++;}"
                       "document.getElementById('search-count').textContent=matches>80&&!term?"
                       "'Showing 80 of '+matches+' matches. Type to narrow the list.':matches+' matches';}"
                       "q.addEventListener('input',update);scope.addEventListener('change',update);update()})()</script>\n";
            write_file(output / "search.md", out.str());
        }

        for (const auto& dir : directories) {
            const auto page = fs::path(directory_page(dir));
            std::ostringstream out;
            const auto readme = root / dir / "README.md";
            std::string body;
            std::string title = dir.empty() ? label(root) : label(dir);
            if (fs::is_regular_file(readme)) {
                body = read_file(readme);
                if (body.rfind("# ", 0) == 0) {
                    const auto end = body.find('\n');
                    title = body.substr(2, end == std::string::npos ? std::string::npos : end - 2);
                    body.erase(0, end == std::string::npos ? body.size() : end + 1);
                }
            }
            if (dir.empty() && !options.title.empty()) title = options.title;
            out << page_header(page, title, options.html);
            const bool authored_navigation = has_local_links(body);
            if (!body.empty()) {
                out << rewrite_links(body, readme, page, root, output, pages, extra_markdown, options.html, result) << "\n\n";
            }
            if (dir.empty()) {
                out << "## Explore the reference\n\n"
                    << "- [Guided setup](" << link_from(page, "search.md", options.html) << "?scope=guided)\n"
                    << "- [Override-capable values](" << link_from(page, "search.md", options.html)
                    << "?scope=override-capable)\n"
                    << "- [Collection entries](" << link_from(page, "search.md", options.html) << "?scope=entry)\n"
                    << "- [Schemas](" << link_from(page, "search.md", options.html) << "?scope=schema)\n\n";
                if (!authored_navigation) {
                    std::vector<const SearchEntry*> examples;
                    for (const auto& entry : searchable)
                        if (entry.scope.find("guided") != std::string::npos && !entry.description.empty() &&
                            (entry.visibility.empty() || entry.visibility == "basic"))
                            examples.push_back(&entry);
                    if (!examples.empty()) {
                        out << "## Examples to explore\n\nAll " << examples.size()
                            << " guided settings with descriptions and basic or unspecified visibility are listed below.\n\n"
                            << "[Browse every guided setting](" << link_from(page, "search.md", options.html)
                            << "?scope=guided).\n\n";
                        for (const auto* entry : examples)
                            out << "- [" << code(entry->path) << "](" << link_from(page, entry->page, options.html)
                                << "#" << anchor(entry->path) << ") — " << entry->description << "\n";
                        out << "\n";
                    }
                }
            }
            if (!dir.empty()) {
                size_t entries = 0;
                for (const auto& [file, contents] : files)
                    if (file.parent_path() == dir) entries += contents.entries.size();
                if (entries) {
                    out << "## Collection entries\n\nAll " << entries << " entries are listed below.\n\n";
                    for (const auto& [file, contents] : files)
                        if (file.parent_path() == dir)
                            for (const auto& entry : contents.entries) {
                                out << "- [" << (entry.title.empty() ? code(entry.path) : entry.title) << "](" <<
                                    link_from(page, source_page(file), options.html) << "#" << entry_anchor(entry.path)
                                    << ")" << (entry.description.empty() ? "" : " — " + entry.description);
                                if (!entry.url.empty()) out << " · [Source](<" << entry.url << ">)";
                                out << "\n";
                            }
                    out << "\n";
                }
            }
            out << "## Contents\n\n";
            for (const auto& child : directories)
                if (child != dir && child.parent_path() == dir)
                    out << "- [" << label(child) << "](" << link_from(page, directory_page(child), options.html) << ")\n";
            for (const auto& [file, contents] : files)
                if (file.parent_path() == dir)
                    out << "- [" << (contents.title.empty() ? file.filename().string() : contents.title) << "](" <<
                        link_from(page, source_page(file), options.html) << ") — " <<
                        (!contents.description.empty() ? contents.description :
                         contents.registrations.empty() ? "source" : contents.registrations.front()) << "\n";
            write_file(output / page, out.str());
        }

        for (auto it = extra_markdown.begin(); it != extra_markdown.end(); ++it) {
            const auto body = rewrite_links(read_file(it->second), it->second, it->first, root, output,
                                            pages, extra_markdown, options.html, result);
            write_file(output / it->first, body);
        }

        result.pages = pages.size();
        if (options.html) {
            write_file(output / "style.css", R"CSS(
:root{color-scheme:light;--ink:#20323d;--muted:#5f7380;--accent:#156e79;--line:#d7e4e7;--panel:#f5f9f9}
*{box-sizing:border-box}body{font:16px/1.65 system-ui,-apple-system,sans-serif;color:var(--ink);max-width:1060px;margin:auto;padding:2rem 3rem 5rem}
a{color:var(--accent);text-underline-offset:3px}a:hover{color:#0c4c57}body>p:first-child{font-size:.9rem;color:var(--muted);padding:.8rem 0;border-bottom:1px solid var(--line)}
h1,h2,h3{line-height:1.25;color:#173e4a;scroll-margin-top:1.5rem}h1{font-size:2.25rem;margin:1.5rem 0;border-bottom:2px solid var(--line);padding-bottom:.6rem}
h2{font-size:1.55rem;margin-top:2.5rem}h3{font-size:1.12rem;margin-top:2rem}h3+ p{max-width:75ch}li{margin:.3rem 0}
code{background:#eaf2f3;padding:.12rem .3rem;border-radius:4px;overflow-wrap:anywhere}pre{background:#eaf2f3;padding:1rem 1.2rem;border-left:3px solid var(--accent);overflow-x:auto;border-radius:5px}pre code{padding:0}
ul{padding-left:1.5rem}#setting-list{columns:2;column-gap:2rem;list-style:none;padding:0}#setting-list li{break-inside:avoid;border-bottom:1px solid var(--line);padding:.4rem 0}
#setting-filter{font:inherit;border:1px solid #a9c5c8;border-radius:5px;padding:.5rem .7rem;width:min(100%,28rem)}:focus-visible{outline:3px solid #e0a04a;outline-offset:2px}
#setting-scope{font:inherit;border:1px solid #a9c5c8;border-radius:5px;padding:.5rem .7rem;margin-left:.5rem;background:white}
#setting-list small{display:block;color:var(--muted);font-size:.85rem;line-height:1.35}
.search-description{display:block;color:var(--muted);font-size:.88rem;line-height:1.4}
details{border:1px solid var(--line);border-radius:6px;background:var(--panel);padding:.45rem .8rem;margin:.8rem 0}
details summary{cursor:pointer;color:var(--accent);font-weight:600}details[open] summary{margin-bottom:.5rem}
details pre{margin:.4rem 0}details ul{margin:.4rem 0}
@media(max-width:700px){body{padding:1rem 1.2rem 3rem}h1{font-size:1.8rem}#setting-list{columns:1}}
)CSS");
            for (const auto& page : pages) {
                auto html = page;
                html.replace_extension(".html");
                const auto css = fs::path("style.css").lexically_relative(page.parent_path());
                render_html(output / page, output / html, css);
            }
            for (const auto& [page, _] : extra_markdown) {
                auto html = page;
                html.replace_extension(".html");
                const auto css = fs::path("style.css").lexically_relative(page.parent_path());
                render_html(output / page, output / html, css);
            }
        }
        if (options.pdf) {
            std::string title = options.title.empty() ? label(root) : options.title;
            if (options.title.empty()) {
                std::istringstream index(read_file(output / "index.md"));
                std::string line;
                while (std::getline(index, line))
                    if (line.rfind("# ", 0) == 0) { title = line.substr(2); break; }
            }
            render_pdf_book(output, title, directories, files, pages, extra_markdown, options.pdf_engine,
                            options.cheatsheet_pdf, options.pdf_full_reference);
        }
        return result;
    }
} // namespace snt::dip::report
