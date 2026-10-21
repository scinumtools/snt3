#include <algorithm>
#include <snt/dip/tools/formatter.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace snt::dip::tools {
    namespace {
        bool space(char c) {
            return c == ' ' || c == '\t';
        }

        void trim_right(std::string& value) {
            while (!value.empty() && space(value.back()))
                value.pop_back();
        }

        std::string trim_left(std::string value) {
            const auto first = std::find_if_not(value.begin(), value.end(), space);
            value.erase(value.begin(), first);
            return value;
        }

        struct ScanState {
            // Strings and arrays may span lines, so their lexical state cannot
            // be reset at each newline. No evaluated DIPL nodes are involved.
            char quote = 0;
            bool triple = false;
            int brackets = 0;
            int braces = 0;
            int parens = 0;
            bool array_value = false;
        };

        bool is_property(const std::string& text) {
            return !text.empty() && (text.front() == '?' || text.front() == '!');
        }

        // Only edit punctuation outside strings, comments, expressions, and references.
        std::string normalize_line(std::string line, ScanState& state) {
            if (state.triple) {
                // Triple-quoted text may contain a table or an arbitrary
                // string. Leave every byte of its physical lines untouched.
                const auto end = line.find("\"\"\"");
                if (end == std::string::npos)
                    return line;
                state.triple = false;
                return line; // Scalar contents, including the closing line, are opaque.
            }

            trim_right(line);
            std::string out;
            out.reserve(line.size() + 4);
            bool escaped = false;
            bool assignment_done = false;
            const std::string property_name = trim_left(line);
            const bool property = is_property(property_name);
            const bool array_directive = property_name.rfind("!options", 0) == 0 ||
                                         property_name.rfind("!labels", 0) == 0 || property_name.rfind("!tags", 0) == 0;
            for (size_t i = 0; i < line.size(); ++i) {
                const char c = line[i];
                // Ordinary quoted text is copied verbatim. An escaped quote
                // does not end the string and must not expose its punctuation.
                if (state.quote) {
                    out += c;
                    if (escaped)
                        escaped = false;
                    else if (c == '\\')
                        escaped = true;
                    else if (c == state.quote)
                        state.quote = 0;
                    continue;
                }
                // Inline triple strings are copied in one piece. If there is
                // no closing marker, later physical lines use state.triple.
                if (line.compare(i, 3, "\"\"\"") == 0) {
                    const auto close = line.find("\"\"\"", i + 3);
                    if (close != std::string::npos) {
                        out += line.substr(i, close + 3 - i);
                        i = close + 2;
                        continue;
                    }
                    state.triple = true;
                    out += line.substr(i);
                    return out; // Scalar contents are opaque.
                }
                // Enter quote mode before checking comments or delimiters.
                if (c == '\'' || c == '"') {
                    state.quote = c;
                    out += c;
                    continue;
                }
                // A comment owns the rest of its line, even if it contains
                // brackets, colons, or equals signs.
                if (c == '#') {
                    out += line.substr(i);
                    break;
                }
                if (c == '[') {
                    // A bracket in a path or dimension declaration is not a
                    // value array. Only array values get comma spacing.
                    if (state.brackets == 0)
                        state.array_value = assignment_done || array_directive;
                    ++state.brackets;
                    out += c;
                    continue;
                }
                if (c == ']') {
                    // Only the outer closing bracket ends an array value;
                    // nested brackets keep the array context active.
                    state.brackets = std::max(0, state.brackets - 1);
                    if (state.brackets == 0)
                        state.array_value = false;
                    out += c;
                    continue;
                }
                // References and expressions can occur inside arrays. Track
                // their depth so commas within them remain untouched.
                if (c == '{')
                    ++state.braces;
                if (c == '}')
                    state.braces = std::max(0, state.braces - 1);
                if (c == '(')
                    ++state.parens;
                if (c == ')')
                    state.parens = std::max(0, state.parens - 1);
                // Replace arbitrary spacing after an array-element comma
                // with one space, except directly before a closing bracket.
                if (c == ',' && state.brackets > 0 && state.array_value && state.braces == 0 && state.parens == 0) {
                    while (!out.empty() && space(out.back()))
                        out.pop_back();
                    out += ',';
                    size_t next = i + 1;
                    while (next < line.size() && space(line[next]))
                        ++next;
                    if (next < line.size() && line[next] != ']')
                        out += ' ';
                    i = next - 1;
                    continue;
                }
                // A top-level colon binds a node to a schema. Leave colons
                // in properties, nested syntax, and :: sequences unchanged.
                if (c == ':' && !property && state.brackets == 0 && state.braces == 0 && state.parens == 0 &&
                    !assignment_done && (i == 0 || line[i - 1] != ':') &&
                    (i + 1 == line.size() || line[i + 1] != ':')) {
                    while (!out.empty() && space(out.back()))
                        out.pop_back();
                    if (!out.empty())
                        out += ' ';
                    out += ':';
                    size_t next = i + 1;
                    while (next < line.size() && space(line[next]))
                        ++next;
                    if (next < line.size() && line[next] != '#')
                        out += ' ';
                    i = next - 1;
                    continue;
                }
                // Format the first top-level assignment only. The neighboring
                // character checks exclude comparison operators such as ==.
                if (c == '=' && state.brackets == 0 && state.braces == 0 && state.parens == 0 && !assignment_done &&
                    (i == 0 ||
                     (line[i - 1] != '=' && line[i - 1] != '!' && line[i - 1] != '<' && line[i - 1] != '>')) &&
                    (i + 1 == line.size() || line[i + 1] != '=')) {
                    while (!out.empty() && space(out.back()))
                        out.pop_back();
                    if (!out.empty())
                        out += ' ';
                    out += '=';
                    size_t next = i + 1;
                    while (next < line.size() && space(line[next]))
                        ++next;
                    if (next < line.size() && line[next] != '#')
                        out += ' ';
                    i = next - 1;
                    assignment_done = true;
                    continue;
                }
                out += c;
            }
            return out;
        }

        // Indentation determines block relationships; it is never rewritten
        // because malformed nesting cannot be inferred safely.
        int indentation(const std::string& line) {
            int count = 0;
            for (char c : line) {
                if (c == ' ')
                    ++count;
                else if (c == '\t')
                    count += 2;
                else
                    break;
            }
            return count;
        }

    } // namespace

    std::string format(std::string_view source, const FormatOptions& options) {
        // Keep the project setting bounded so a mistaken value cannot insert
        // an unbounded number of blank lines for every sibling node.
        if (options.blank_lines_between_nodes < 0 || options.blank_lines_between_nodes > 4)
            throw std::invalid_argument("format.blank_lines_between_nodes must be between 0 and 4");

        // Keep the input's line-ending convention, including inside blocks.
        const size_t first_newline = source.find('\n');
        const std::string_view newline =
            first_newline != std::string_view::npos && first_newline > 0 && source[first_newline - 1] == '\r' ? "\r\n"
                                                                                                              : "\n";

        std::vector<std::string> output;
        std::vector<std::string> pending_comments;
        ScanState state;
        int previous_node_indent = -1;
        size_t offset = 0;
        while (offset < source.size()) {
            const size_t end = source.find('\n', offset);
            const size_t length = (end == std::string_view::npos ? source.size() : end) - offset;
            std::string line(source.substr(offset, length));
            // Strip the line-ending CR before scanning; the selected newline
            // convention is restored when output is assembled.
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            offset = end == std::string_view::npos ? source.size() : end + 1;

            const bool continuation = state.triple || state.brackets > 0;
            // Outside multiline values, blank lines are regenerated by the
            // node-spacing rule. Inside them, blank lines are source content.
            if (!continuation) {
                trim_right(line);
                if (line.empty())
                    continue;
            }
            const int indent = indentation(line);
            const std::string stripped = trim_left(line);
            if (!continuation && !stripped.empty() && stripped.front() == '#') {
                // A comment run belongs to the following node block. Put the
                // sibling separator before those comments, not after them.
                pending_comments.push_back(line);
                continue;
            }

            if (!continuation && !stripped.empty() && !is_property(stripped)) {
                // Properties and the first child remain attached to their
                // node; only a sibling or outdented node starts a new block.
                if (previous_node_indent >= 0 && indent <= previous_node_indent) {
                    for (int n = 0; n < options.blank_lines_between_nodes; ++n)
                        output.emplace_back();
                }
                previous_node_indent = indent;
            }
            // Emit comments immediately before the node they introduce, after
            // any sibling separator already added above.
            output.insert(output.end(), pending_comments.begin(), pending_comments.end());
            pending_comments.clear();
            output.push_back(normalize_line(std::move(line), state));
        }
        // Comments at end of file have no following node to attach to.
        output.insert(output.end(), pending_comments.begin(), pending_comments.end());

        std::string result;
        for (size_t i = 0; i < output.size(); ++i) {
            if (i)
                result += newline;
            result += output[i];
        }
        // final_newline controls only the final terminator, not inner lines.
        if (options.final_newline && !result.empty())
            result += newline;
        return result;
    }
} // namespace snt::dip::tools
