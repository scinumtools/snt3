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
} // namespace

bool SourceView::open(const std::filesystem::path& file, std::size_t line, std::string& error,
                      bool plain_text) {
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
    std::string content = buffer.str();
    if (plain_text && content.find('\0') != std::string::npos) {
        error = "Raw source contains NUL bytes and cannot be displayed as text.";
        return false;
    }
    std::vector<HighlightedLine> highlighted = plain_text
        ? plain_lines(content) : dipl::highlight::tokenize(content);
    file_ = file;
    target_line_ = line;
    text_ = std::move(content);
    lines_ = std::move(highlighted);
    if (line == 0 || line > lines_.size())
        error = "The recorded line is outside the current file; reload the project to refresh locations.";
    return true;
}

} // namespace snt::view
