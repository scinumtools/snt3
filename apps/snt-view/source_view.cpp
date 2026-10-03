#include "source_view.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace snt::view {

namespace {
std::vector<HighlightedLine> plain_lines(const std::string& content) {
    std::vector<HighlightedLine> lines;
    std::istringstream stream(content);
    std::string current;
    while (std::getline(stream, current)) {
        if (!current.empty() && current.back() == '\r') current.pop_back();
        lines.push_back({current, current.empty()
            ? std::vector<dipl::highlight::SyntaxSpan>{}
            : std::vector<dipl::highlight::SyntaxSpan>{{current, SyntaxKind::Text}}});
    }
    if (lines.empty() || (!content.empty() && content.back() == '\n'))
        lines.push_back({"", {}});
    return lines;
}

std::vector<HighlightedLine> table_lines(const std::string& content) {
    auto lines = plain_lines(content);
    std::string header;
    std::size_t count = 0;
    for (const auto& line : lines) {
        const auto start = line.text.find_first_not_of(" \t");
        if (start != std::string::npos) {
            const auto end = line.text.find_last_not_of(" \t");
            if (line.text.substr(start, end - start + 1) == "---") break;
        }
        if (count++) header += '\n';
        header += line.text;
    }
    if (count) {
        const auto highlighted = dipl::highlight::tokenize(header);
        for (std::size_t index = 0; index < count; ++index)
            lines[index].spans = highlighted[index].spans;
    }
    return lines;
}
} // namespace

bool SourceView::open(const std::filesystem::path& file, std::size_t line, std::string& error,
                      SourceFormat format) {
    error.clear();
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        error = "Cannot open the source file.";
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    if (!input.good() && !input.eof()) {
        error = "Cannot read the source file.";
        return false;
    }
    return set_content(file, line, line, buffer.str(), format, error);
}

bool SourceView::open_text(const std::filesystem::path& file, std::size_t source_line,
                           std::string content, SourceFormat format, std::string& error) {
    return set_content(file, source_line, 1, std::move(content), format, error);
}

bool SourceView::set_content(const std::filesystem::path& file, std::size_t source_line,
                             std::size_t target_line, std::string content, SourceFormat format,
                             std::string& error) {
    error.clear();
    if (content.find('\0') != std::string::npos) {
        error = "Source contains NUL bytes and cannot be displayed as text.";
        return false;
    }
    std::vector<HighlightedLine> highlighted;
    switch (format) {
    case SourceFormat::DIPL: highlighted = dipl::highlight::tokenize(content); break;
    case SourceFormat::Plain: highlighted = plain_lines(content); break;
    case SourceFormat::Table: highlighted = table_lines(content); break;
    }
    file_ = file;
    source_line_ = source_line;
    target_line_ = target_line;
    text_ = std::move(content);
    lines_ = std::move(highlighted);
    if (target_line == 0 || target_line > lines_.size())
        error = "The recorded line is outside the current file; reload the project to refresh locations.";
    return true;
}

} // namespace snt::view
