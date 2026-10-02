#ifndef SNT_VIEW_SOURCE_VIEW_H
#define SNT_VIEW_SOURCE_VIEW_H

#include <dipl_highlighter.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace snt::view {

using SyntaxKind = dipl::highlight::SyntaxKind;
using HighlightedLine = dipl::highlight::HighlightedLine;

/** A read-only file buffer with DIPL token categories for the viewer. */
class SourceView {
public:
    bool open(const std::filesystem::path& file, std::size_t line, std::string& error);
    const std::filesystem::path& file() const { return file_; }
    std::size_t target_line() const { return target_line_; }
    const std::string& text() const { return text_; }
    const std::vector<HighlightedLine>& lines() const { return lines_; }

private:
    std::filesystem::path file_;
    std::size_t target_line_ = 0;
    std::string text_;
    std::vector<HighlightedLine> lines_;
};

} // namespace snt::view

#endif
