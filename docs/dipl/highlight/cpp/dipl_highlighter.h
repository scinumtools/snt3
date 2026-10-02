#ifndef SNT_DIPL_HIGHLIGHTER_H
#define SNT_DIPL_HIGHLIGHTER_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace snt::dipl::highlight {

enum class SyntaxKind {
    Text, Keyword, Metadata, Name, Type, Dimension, String, Number,
    Boolean, Reference, Expression, Unit, Comment
};

struct SyntaxSpan {
    std::string text;
    SyntaxKind kind = SyntaxKind::Text;
};

struct HighlightedLine {
    std::string text;
    std::vector<SyntaxSpan> spans;
};

struct Rgb {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
};

/** Tokenize DIPL text into lines and colored spans, excluding line separators. */
std::vector<HighlightedLine> tokenize(std::string_view source);

/** Palette matching the DIPL Pygments style; Text uses the host UI color. */
Rgb color(SyntaxKind kind);

} // namespace snt::dipl::highlight

#endif
