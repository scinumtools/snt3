#include "source_view.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace snt::view {

bool SourceView::open(const std::filesystem::path& file, std::size_t line, std::string& error) {
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
    std::vector<HighlightedLine> highlighted = dipl::highlight::tokenize(content);
    file_ = file;
    target_line_ = line;
    text_ = std::move(content);
    lines_ = std::move(highlighted);
    if (line == 0 || line > lines_.size())
        error = "The recorded line is outside the current file; reload the project to refresh locations.";
    return true;
}

} // namespace snt::view
