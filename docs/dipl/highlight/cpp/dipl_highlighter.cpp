#include "dipl_highlighter.h"

#include <cctype>
#include <sstream>
#include <string_view>

namespace snt::dipl::highlight {
namespace {

struct LexerState {
    bool in_triple_string = false;
    bool formatted = false;
    char quote = '\0';
};

void append(std::vector<SyntaxSpan>& spans, SyntaxKind kind, std::string_view text) {
    if (text.empty()) return;
    if (!spans.empty() && spans.back().kind == kind)
        spans.back().text.append(text.data(), text.size());
    else
        spans.push_back({std::string(text), kind});
}

bool name_char(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-' || ch == '.';
}

bool type_name(std::string_view name) {
    if (name == "bool" || name == "str" || name == "table" || name == "map" || name == "list") return true;
    if (name.substr(0, 3) == "int" || name.substr(0, 4) == "uint" || name.substr(0, 5) == "float") return true;
    return false;
}

std::size_t scan_reference(std::string_view line, std::size_t start) {
    int depth = 0;
    std::size_t pos = start;
    for (; pos < line.size(); ++pos) {
        if (line[pos] == '{') ++depth;
        else if (line[pos] == '}' && --depth == 0) return pos + 1;
    }
    return line.size();
}

std::size_t scan_string(std::string_view line, std::size_t pos, LexerState& state,
                        std::vector<SyntaxSpan>& spans) {
    const char quote = state.quote;
    const std::string_view close = state.in_triple_string
        ? (quote == '"' ? std::string_view("\"\"\"") : std::string_view("'''"))
        : std::string_view(&state.quote, 1);
    std::size_t segment = pos;
    while (pos < line.size()) {
        if (state.formatted && line.substr(pos, 2) == "{{") {
            const auto end = line.find("}}", pos + 2);
            if (end != std::string_view::npos) {
                append(spans, SyntaxKind::String, line.substr(segment, pos - segment));
                append(spans, SyntaxKind::Reference, line.substr(pos, end + 2 - pos));
                pos = end + 2;
                segment = pos;
                continue;
            }
        }
        if (line[pos] == '\\' && pos + 1 < line.size()) {
            pos += 2;
            continue;
        }
        if (line.substr(pos, close.size()) == close) {
            pos += close.size();
            append(spans, SyntaxKind::String, line.substr(segment, pos - segment));
            state = {};
            return pos;
        }
        ++pos;
    }
    append(spans, SyntaxKind::String, line.substr(segment));
    if (!state.in_triple_string) state = {};
    return pos;
}

std::vector<SyntaxSpan> highlight(std::string_view line, LexerState& state) {
    std::vector<SyntaxSpan> spans;
    std::size_t pos = 0;
    bool leading = true;
    bool expect_name = false;
    bool expect_type = false;
    bool after_number = false;
    int expression_depth = 0;
    while (pos < line.size()) {
        if (state.in_triple_string) {
            pos = scan_string(line, pos, state, spans);
            continue;
        }
        const char ch = line[pos];
        if (ch == ' ' || ch == '\t') {
            const auto start = pos;
            while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) ++pos;
            append(spans, SyntaxKind::Text, line.substr(start, pos - start));
            continue;
        }
        if (ch == '#') {
            append(spans, SyntaxKind::Comment, line.substr(pos));
            break;
        }
        if ((ch == 'f' || ch == 'F') && pos + 1 < line.size() &&
            (line[pos + 1] == '"' || line[pos + 1] == '\'')) {
            append(spans, SyntaxKind::String, line.substr(pos, 1));
            ++pos;
            state.formatted = true;
        }
        if (line[pos] == '"' || line[pos] == '\'') {
            state.quote = line[pos];
            state.in_triple_string = line.substr(pos, 3) ==
                (state.quote == '"' ? std::string_view("\"\"\"") : std::string_view("'''"));
            const std::size_t opening = state.in_triple_string ? 3 : 1;
            append(spans, SyntaxKind::String, line.substr(pos, opening));
            pos = scan_string(line, pos + opening, state, spans);
            leading = false;
            continue;
        }
        if (ch == '{') {
            const auto end = scan_reference(line, pos);
            append(spans, SyntaxKind::Reference, line.substr(pos, end - pos));
            pos = end;
            if (pos < line.size() && line[pos] == '[') {
                const auto close = line.find(']', pos + 1);
                if (close != std::string_view::npos) {
                    append(spans, SyntaxKind::Reference, line.substr(pos, 1));
                    append(spans, SyntaxKind::Dimension, line.substr(pos + 1, close - pos - 1));
                    append(spans, SyntaxKind::Reference, line.substr(close, 1));
                    pos = close + 1;
                }
            }
            leading = false;
            continue;
        }
        if (leading && (ch == '$' || ch == '!' || ch == '?' || ch == '@')) {
            const auto start = pos++;
            while (pos < line.size() && name_char(line[pos])) ++pos;
            const auto directive = line.substr(start, pos - start);
            append(spans, ch == '?' ? SyntaxKind::Metadata : SyntaxKind::Keyword, directive);
            expect_name = ch == '$' && directive != "$override";
            leading = false;
            continue;
        }
        if (ch == '(') {
            ++expression_depth;
            append(spans, SyntaxKind::Expression, line.substr(pos++, 1));
            leading = false;
            continue;
        }
        if (ch == ')' && expression_depth) {
            --expression_depth;
            append(spans, SyntaxKind::Expression, line.substr(pos++, 1));
            continue;
        }
        if (ch == '=' || ch == ',' || ch == '[' || ch == ']') {
            append(spans, SyntaxKind::Text, line.substr(pos++, 1));
            if (ch == '=') { expect_type = false; after_number = false; }
            continue;
        }
        if (std::isdigit(static_cast<unsigned char>(ch)) ||
            ((ch == '-' || ch == '+') && pos + 1 < line.size() &&
             std::isdigit(static_cast<unsigned char>(line[pos + 1])))) {
            const auto start = pos++;
            while (pos < line.size() && (std::isdigit(static_cast<unsigned char>(line[pos])) ||
                                          line[pos] == '.' || line[pos] == 'e' || line[pos] == 'E' ||
                                          line[pos] == '+' || line[pos] == '-')) ++pos;
            append(spans, expression_depth ? SyntaxKind::Expression : SyntaxKind::Number,
                   line.substr(start, pos - start));
            after_number = !expression_depth;
            leading = false;
            continue;
        }
        if (name_char(ch)) {
            const auto start = pos++;
            while (pos < line.size() && name_char(line[pos])) ++pos;
            const auto word = line.substr(start, pos - start);
            SyntaxKind kind = SyntaxKind::Text;
            if (expression_depth) kind = SyntaxKind::Expression;
            else if (leading || expect_name) kind = SyntaxKind::Name;
            else if (expect_type && type_name(word)) kind = SyntaxKind::Type;
            else if (word == "true" || word == "false") kind = SyntaxKind::Boolean;
            else if (word == "none") kind = SyntaxKind::Keyword;
            else if (after_number) kind = SyntaxKind::Unit;
            append(spans, kind, word);
            if (leading) expect_type = true;
            expect_name = false;
            leading = false;
            if (kind != SyntaxKind::Unit) after_number = false;
            continue;
        }
        append(spans, expression_depth ? SyntaxKind::Expression : SyntaxKind::Text, line.substr(pos++, 1));
        leading = false;
    }
    return spans;
}

} // namespace

std::vector<HighlightedLine> tokenize(std::string_view source) {
    std::vector<HighlightedLine> highlighted;
    LexerState state;
    std::istringstream stream{std::string(source)};
    std::string current;
    while (std::getline(stream, current)) {
        if (!current.empty() && current.back() == '\r') current.pop_back();
        highlighted.push_back({current, highlight(current, state)});
    }
    if (highlighted.empty() || (!source.empty() && source.back() == '\n'))
        highlighted.push_back({"", {}});
    return highlighted;
}

Rgb color(SyntaxKind kind) {
    switch (kind) {
    case SyntaxKind::Keyword: return {117, 117, 117};
    case SyntaxKind::Metadata: return {0, 131, 143};
    case SyntaxKind::Name: return {97, 97, 97};
    case SyntaxKind::Type: return {97, 97, 97};
    case SyntaxKind::Dimension: return {158, 158, 158};
    case SyntaxKind::String: return {85, 139, 47};
    case SyntaxKind::Number: return {183, 28, 28};
    case SyntaxKind::Boolean: return {13, 71, 161};
    case SyntaxKind::Reference: return {230, 74, 25};
    case SyntaxKind::Expression: return {141, 110, 99};
    case SyntaxKind::Unit: return {96, 125, 139};
    case SyntaxKind::Comment: return {251, 140, 0};
    case SyntaxKind::Text: return {0, 0, 0};
    }
    return {0, 0, 0};
}

} // namespace snt::dipl::highlight
