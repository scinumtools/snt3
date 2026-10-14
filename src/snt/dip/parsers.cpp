#include "parsers.h"

#include "helpers.h"
#include "nodes/node_empty.h"
#include "nodes/node_group.h"
#include "nodes/node_import.h"
#include "nodes/node_modification.h"
#include "nodes/node_override.h"
#include "nodes/node_property.h"
#include "nodes/node_schema.h"
#include "nodes/node_source.h"
#include "nodes/node_table.h"
#include "nodes/node_unit.h"

#include <snt/dip/dip.h>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/node_boolean.h>
#include <snt/dip/nodes/node_case.h>
#include <snt/dip/nodes/node_float.h>
#include <snt/dip/nodes/node_integer.h>
#include <snt/dip/nodes/node_string.h>

namespace snt::dip {

    void check_indent(const BaseNode::PointerType& previous_node, const BaseNode::PointerType& current_node) {
        // We make sure that the indent spacing is always set by INDENT_STEP
        if ((current_node->indent % INDENT_STEP) != 0) {
            std::stringstream suggested;
            suggested << (current_node->indent - (current_node->indent % INDENT_STEP)) << ", ";
            suggested << (current_node->indent - (current_node->indent % INDENT_STEP) + INDENT_STEP) << ", ...";
            throw dip::SyntaxException(
                "Invalid indent length",
                "The indentation length is " + std::to_string(current_node->indent) + ", which is not a multiple of " +
                    std::to_string(INDENT_STEP) + ".",
                "Use an indentation length of " + suggested.str(),
                __FILE__,
                __LINE__,
                current_node->line
            );
        }
        if (previous_node != nullptr) {
            if ((current_node->indent > previous_node->indent) &&
                (current_node->indent - previous_node->indent) != INDENT_STEP) {
                throw dip::SyntaxException(
                    "Child node has an invalid indent",
                    "The child node is indented " + std::to_string(current_node->indent) +
                        " spaces, but it should be " + std::to_string(previous_node->indent + INDENT_STEP) + " spaces.",
                    "Indent the child node " + std::to_string(INDENT_STEP) + " spaces more than the preceding node.",
                    __FILE__,
                    __LINE__,
                    current_node->line
                );
            }
        }
    }

    inline EnvSource parse_file_source(
        const std::string& source_name,
        const std::string& source_file,
        const Source& parent,
        const ProjectFileReader& reader
    ) {
        DIP d(parent);
        d.set_project_file_reader(reader);
        d.add_file(source_file, source_name);
        Environment senv = d.parse();
        EnvSource source{source_name, source_file, senv.sources.at(source_name).code, parent, senv.nodes};
        source.hierarchy = std::move(senv.hierarchy);
        source.declarations = senv.declarations();
        return source;
    }

    EnvSource parse_source(
        const std::string& source_name,
        const std::string& source_file,
        const Source& parent,
        const ProjectFileReader& reader
    ) {
        if (source_file.size() >= FILE_SUFFIX_DIP1.size() &&
            source_file.compare(
                source_file.size() - FILE_SUFFIX_DIP1.size(), FILE_SUFFIX_DIP1.size(), FILE_SUFFIX_DIP1
            ) == 0) {
            return parse_file_source(source_name, source_file, parent, reader);
        } else if (
            source_file.size() >= FILE_SUFFIX_DIP2.size() &&
            source_file.compare(
                source_file.size() - FILE_SUFFIX_DIP2.size(), FILE_SUFFIX_DIP2.size(), FILE_SUFFIX_DIP2
            ) == 0
        ) {
            return parse_file_source(source_name, source_file, parent, reader);
        } else {
            EnvSource source{source_name, source_file, reader(source_file), parent};
            source.raw_text = true;
            return source;
        }
    }

    std::queue<Line> parse_lines(
        std::queue<Line>& lines, const std::string& source_code, const std::string& source_name
    ) {
        // parse nodes from a text
        std::string line;
        std::istringstream ss(source_code);
        size_t line_number = 1;
        while (std::getline(ss, line)) {
            // Project files are read as bytes, so CRLF retains its carriage return.
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            // skip empty lines
            if (!line.empty() && !std::all_of(line.begin(), line.end(), isspace))
                lines.push({line, {source_name, line_number}});
            line_number++;
        }
        return lines;
    }

    inline int count_group_balance(const std::string& str, char sign_open, char sign_close) {
        int balance = 0;
        bool in_string = false;
        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];
            // Handle entering/exiting string
            if (c == '"' && (i == 0 || str[i - 1] != '\\')) {
                in_string = !in_string;
                continue;
            }
            if (in_string)
                continue;
            if (c == sign_open)
                balance++;
            else if (c == sign_close)
                balance--;
        }
        return balance;
    }

    inline std::string remove_comments(const std::string& str) {
        bool in_quotes = false;
        bool escaped = false;

        for (size_t i = 0; i < str.size(); ++i) {
            char c = str[i];

            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == '#' && !in_quotes) {
                return str.substr(0, i);
            }
        }

        return str;
    }

    BaseNode::ListType parse_code_nodes(std::queue<Line>& lines) {
        BaseNode::ListType nodes;
        while (!lines.empty()) {
            Line line = lines.front();
            lines.pop();
            {
                // add node definition (after = sign) on a new line
                // to the node declaration on the current line
                if (!lines.empty()) {
                    const Line& next = lines.front();
                    std::size_t line_indent = line.code.find_first_not_of(' ');
                    std::size_t next_indent = next.code.find_first_not_of(' ');
                    if (next.code[next_indent] == SIGN_EQUAL) {
                        if (line_indent != std::string::npos && next_indent == line_indent + INDENT_STEP &&
                            next_indent < next.code.size()) {
                            line.code = remove_comments(line.code) + " " + next.code.substr(next_indent);
                            lines.pop();
                        } else {
                            throw dip::SyntaxException(
                                "Node definition on a new line has an invalid indent",
                                "The node is indented " + std::to_string(next_indent) + " spaces, but it should be " +
                                    std::to_string(line_indent + INDENT_STEP) + " spaces.",
                                "Indent the node " + std::to_string(INDENT_STEP) +
                                    " spaces more than the preceding node.",
                                __FILE__,
                                __LINE__,
                                next
                            );
                        }
                    }
                }
            }
            {
                // group block strings
                size_t pos = 0;
                if ((pos = line.code.find(SIGN_BLOCK)) != std::string::npos) { // opening block quotes
                    pos += SIGN_BLOCK.length();
                    std::ostringstream oss;
                    oss << line.code;
                    if (line.code.find(SIGN_BLOCK, pos) == std::string::npos) { // closing block quotes on the same line
                        while (lines.size() > 0) {
                            Line block_line = lines.front();
                            lines.pop();
                            oss << SEPARATOR_NEWLINE << block_line.code;
                            if (block_line.code.find(SIGN_BLOCK) !=
                                std::string::npos) { // closing block quotes on a subsequent line
                                break;
                            }
                        }
                    }
                    line.code = oss.str();
                }
            }
            {
                // group expression parentheses group
                int balance = count_group_balance(line.code, SIGN_EXPRESSION_OPEN, SIGN_EXPRESSION_CLOSE);
                while (balance > 0 && !lines.empty()) {
                    Line next = lines.front();
                    lines.pop();
                    line.code += remove_comments(next.code);
                    balance += count_group_balance(next.code, SIGN_EXPRESSION_OPEN, SIGN_EXPRESSION_CLOSE);
                }
                if (balance != 0) {
                    throw dip::SyntaxException(
                        "Unbalanced parentheses",
                        "The expression contains unmatched parentheses.",
                        "Check that every opening `(` has a corresponding closing `)`.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                }
            }
            {
                // group array brackets group
                int balance = count_group_balance(line.code, SIGN_ARRAY_OPEN, SIGN_ARRAY_CLOSE);
                while (balance > 0 && !lines.empty()) {
                    Line next = lines.front();
                    lines.pop();
                    line.code += remove_comments(next.code);
                    balance += count_group_balance(next.code, SIGN_ARRAY_OPEN, SIGN_ARRAY_CLOSE);
                }
                if (balance != 0) {
                    throw dip::SyntaxException(
                        "Unbalanced array brackets",
                        "The array expression contains unmatched brackets.",
                        "Check that every opening `[` has a corresponding closing `]`.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                }
            }

            // add replacement mark for escape symbols
            Parser::encode_escape_symbols(line.code);

            // determine node type
            Parser parser(line);
            BaseNode::PointerType node = nullptr;
            node = EmptyNode::is_node(parser);
            if (node == nullptr)
                parser.part_indent();
            if (node == nullptr)
                node = ImportNode::is_node(parser);
            if (node == nullptr)
                node = UnitNode::is_node(parser);
            if (node == nullptr)
                node = SourceNode::is_node(parser);
            if (node == nullptr)
                node = SchemaNode::is_node(parser);
            if (node == nullptr)
                node = OverrideNode::is_node(parser);
            if (node == nullptr)
                node = CaseNode::is_node(parser);
            if (node == nullptr)
                node = PropertyNode::is_node(parser);
            if (node == nullptr)
                parser.part_path();
            if (node == nullptr)
                node = GroupNode::is_node(parser); // groups
            if (node == nullptr)
                node = ImportNode::is_node(parser);
            if (node == nullptr)
                node = ModificationNode::is_node(parser);
            if (node == nullptr)
                parser.part_type();
            if (node == nullptr)
                node = BooleanNode::is_node(parser);
            if (node == nullptr)
                node = IntegerNode::is_node(parser);
            if (node == nullptr)
                node = FloatNode::is_node(parser);
            if (node == nullptr)
                node = StringNode::is_node(parser);
            if (node == nullptr)
                node = TableNode::is_node(parser);
            if (node == nullptr)
                node = GroupNode::is_node(parser); // maps and lists

            // make sure that everything was parsed
            if (node == nullptr)
                throw dip::SyntaxException(
                    "Unknown node",
                    "No node could be determined from the line.",
                    "Check the syntax of the node definition.",
                    __FILE__,
                    __LINE__,
                    line
                );
            if (parser.do_continue())
                throw dip::SyntaxException(
                    "Unparsed input",
                    "Additional text remains after the parsed node type.",
                    "Check the syntax and remove or correct the unparsed text.",
                    __FILE__,
                    __LINE__,
                    line
                );

            // convert escape symbols to original characterss
            for (auto& rval : node->value_raw)
                Parser::decode_escape_symbols(rval);
            Parser::decode_escape_symbols(node->line.code);

            // put node into the list
            nodes.push_back(node);
        }
        return nodes;
    }

    BaseNode::ListType parse_table_nodes(std::queue<Line>& lines, ValueMetadata& table_metadata, const char delimiter) {
        // parse nodes from a table header
        BaseNode::ListType nodes;
        while (!lines.empty()) {
            Line line = lines.front();
            lines.pop();
            const auto first = line.code.find_first_not_of(' ');
            const size_t indent = first == std::string::npos ? 0 : first;
            std::string normalized = line.code;
            trim(normalized);
            // stop when the end of the table header is reached
            if (normalized == SEPARATOR_TABLE_HEADER)
                break;
            // Metadata at header indentation describes the table; one level
            // beneath a column definition describes that column.
            Parser property_parser(line);
            property_parser.part_indent();
            if (auto property = std::dynamic_pointer_cast<PropertyNode>(PropertyNode::is_node(property_parser))) {
                const bool is_metadata =
                    property->ptype >= PropertyType::Description && property->ptype < PropertyType::Delimiter;
                if (!is_metadata)
                    throw dip::SyntaxException(
                        "Unsupported table header property",
                        "Only metadata properties are supported in table headers.",
                        "Use metadata at indentation zero for the table or indentation two under a column.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                if (indent == 0 && nodes.empty()) {
                    table_metadata.set_property(property->ptype, property->value_raw);
                } else if (indent == INDENT_STEP && !nodes.empty()) {
                    auto& column = nodes.back();
                    if (!column->set_property(property->ptype, property->value_raw, property->units_raw))
                        throw dip::SyntaxException(
                            "Unsupported column metadata",
                            "The table column does not accept this metadata property.",
                            "Use a defined metadata property under the column definition.",
                            __FILE__,
                            __LINE__,
                            line
                        );
                } else {
                    throw dip::SyntaxException(
                        "Invalid table metadata indentation",
                        "Table metadata must precede columns at indentation zero, and column metadata must be indented "
                        "two spaces beneath its column.",
                        "Move the property to the table header or beneath its column definition.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                }
                continue;
            }
            line.code = std::move(normalized);
            // parse a node from the current line
            Parser parser(line);
            parser.part_path();
            parser.part_type();
            parser.part_dimension();
            parser.part_units();
            if (parser.do_continue())
                throw dip::SyntaxException(
                    "Invalid table header format",
                    "Additional text remains after the header has been parsed.",
                    "Check the header syntax and remove or correct the remaining text.",
                    __FILE__,
                    __LINE__,
                    line
                );
            // initialize actual node
            BaseNode::PointerType node(nullptr);
            if (node == nullptr)
                node = BooleanNode::is_node(parser);
            if (node == nullptr)
                node = IntegerNode::is_node(parser);
            if (node == nullptr)
                node = FloatNode::is_node(parser);
            if (node == nullptr)
                node = StringNode::is_node(parser);
            // make sure that everything was parsed
            if (node == nullptr)
                throw dip::SyntaxException(
                    "Unknown node",
                    "No supported node type could be determined from the line.",
                    "Check the node type and syntax.",
                    __FILE__,
                    __LINE__,
                    line
                );
            if (parser.do_continue())
                throw dip::SyntaxException(
                    "Unparsed input",
                    "Additional text remains after the node has been parsed.",
                    "Check the node syntax and remove or correct the remaining text.",
                    __FILE__,
                    __LINE__,
                    line
                );
            node->value_raw.reserve(lines.size()); // roughly reserve some memory to avoid reallocations
            nodes.push_back(node);
        }
        // read values from the rest of the table data and assign them to the node raw values
        while (!lines.empty()) {
            Line line = lines.front();
            lines.pop();
            trim(line.code);
            // split a table line into node values
            Parser parser(line);
            for (size_t i = 0; i < nodes.size(); i++) {
                // parse delimiter
                if (i > 0) {
                    const auto& node = nodes.at(i - 1);
                    if (node->value_raw.back().back() == delimiter) {
                        // delimiter was parsed with the first value (e.g. if value was not given in
                        // quote marks)
                        node->value_raw.back().pop_back();
                        parser.part_indent();
                    } else {
                        // delimiter still needs to be parsed
                        parser.part_delimiter(delimiter);
                    }
                }
                // parse a column value
                const auto& node = nodes.at(i);
                parser.value_raw.clear();
                if (parser.part_string()) {
                    node->value_raw.push_back(parser.value_raw.at(0));
                } else if (parser.part_number(false, delimiter)) {
                    node->value_raw.push_back(parser.value_raw.at(0));
                } else if (parser.part_keyword(false, delimiter)) {
                    node->value_raw.push_back(parser.value_raw.at(0));
                } else {
                    throw dip::SyntaxException(
                        "Invalid table column value",
                        "The value for column `" + node->path.name + "` could not be parsed from the table row.",
                        "Provide a valid string, number, or keyword value.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                }
            }
            if (parser.do_continue())
                throw dip::SyntaxException(
                    "Unparsed input",
                    "Additional text remains after the line has been parsed.",
                    "Check the syntax and remove or correct the remaining text.",
                    __FILE__,
                    __LINE__,
                    line
                );
        }
        return nodes;
    }

    std::string parse_array(
        const std::string& value_string, val::Array::StringType& value_raw, val::Array::ShapeType& value_shape
    ) {
        std::stringstream ss(value_string), rm;
        char ch;

        // test if array is empty
        if (value_string.substr(0, 2) == std::string{SIGN_ARRAY_OPEN, SIGN_ARRAY_CLOSE}) {
            value_shape.push_back(0);
            value_raw.push_back("");
            ss.get(ch);
            rm << ch;
            ss.get(ch);
            rm << ch;
            return rm.str();
        }

        // test for an openning bracket
        ss.get(ch);
        rm << ch;
        if (ch != SIGN_ARRAY_OPEN)
            throw dip::SyntaxException(
                "Invalid array",
                "The array must start with `" + std::string(1, SIGN_ARRAY_OPEN) + "`, but the input starts with `" +
                    std::string(1, ch) + "`.",
                "Add `" + std::string(1, SIGN_ARRAY_OPEN) + "` at the beginning of the array.",
                __FILE__,
                __LINE__
            );

        std::string value;
        bool has_value = false;
        int dim = 1;
        value_shape.push_back(0);

        while (ss.get(ch) && dim > 0) {
            rm << ch;
            if (ch == SIGN_ARRAY_OPEN) {
                dim++;
                if (value_shape.size() < dim)
                    value_shape.push_back(0);
                value.clear();
            } else if (ch == SEPARATOR_ARRAY) {
                if (has_value) {
                    value_raw.push_back(value);
                    value.clear();
                    has_value = false;
                }
                value_shape[dim - 1]++;
            } else if (ch == SIGN_ARRAY_CLOSE) {
                if (has_value) {
                    value_raw.push_back(value);
                    value.clear();
                    has_value = false;
                }
                value_shape[dim - 1]++;
                dim--;
            } else if (ch == '"') {
                value.clear();
                has_value = true;
                while (ss.get(ch) && ch != '"') {
                    value += ch;
                    rm << ch;
                }
                rm << ch;
            } else if (ch == ' ') {
                continue;
            } else {
                value += ch;
                has_value = true;
            }
        }

        // Check if all nested arrays are closed
        if (dim != 0)
            throw dip::SyntaxException(
                "Unclosed array",
                "The array definition `" + value_string + "` contains unclosed brackets or quotes.",
                "Close all brackets and quotes before the end of the array definition.",
                __FILE__,
                __LINE__
            );

        // Normalize shape and check coherence of nested arrays
        size_t coef = 1;
        for (int d = 1; d < value_shape.size(); d++) {
            coef *= value_shape[d - 1];
            if (value_shape[d] % coef != 0)
                throw dip::SyntaxException(
                    "Inconsistent array shape",
                    "Items in dimension " + std::to_string(d + 1) + " do not have a consistent shape in `" +
                        value_string + "`.",
                    "Make sure that all nested arrays have the same shape.",
                    __FILE__,
                    __LINE__
                );
            value_shape[d] /= coef;
        }
        // std::cout << ss.str() << std::endl;
        // std::cout << rm.str() << std::endl;
        return rm.str();
    }

    void parse_value(std::string value_string, val::Array::StringType& value_raw, val::Array::ShapeType& value_shape) {
        trim(value_string);
        value_string.erase(std::remove(value_string.begin(), value_string.end(), '\n'), value_string.end());
        if (value_string.empty())
            throw dip::SyntaxException(
                "Empty value",
                "The source code of the value is empty.",
                "Provide a value or an array expression.",
                __FILE__,
                __LINE__
            );
        else if (value_string.at(0) == SIGN_ARRAY_OPEN)
            parse_array(value_string, value_raw, value_shape);
        else
            value_raw.push_back(value_string);
    }

    void parse_slices(const std::string& value_string, val::Array::RangeType& dimension) {
        std::istringstream ss_dims(value_string);
        std::string dim;
        while (getline(ss_dims, dim, SEPARATOR_DIMENSION)) {
            val::Array::RangeStruct range = {};
            std::size_t pos = dim.find_first_of(SEPARATOR_SLICE);
            if (pos == std::string::npos) {
                range = {static_cast<size_t>(std::stoul(dim)), static_cast<size_t>(std::stoul(dim))};
            } else {
                std::string dmin = dim.substr(0, pos), dmax = dim.substr(pos + 1);
                if (dmin == "" && dmax == "")
                    range = {0, val::Array::max_range};
                else if (dmin == "")
                    range = {0, static_cast<size_t>(std::stoul(dmax))};
                else if (dmax == "")
                    range = {static_cast<size_t>(std::stoul(dmin)), val::Array::max_range};
                else
                    range = {static_cast<size_t>(std::stoul(dmin)), static_cast<size_t>(std::stoul(dmax))};
            }
            if (range.dmax != val::Array::max_range && range.dmax < range.dmin)
                throw dip::SyntaxException(
                    "Invalid dimension range",
                    "The maximum range value `" + std::to_string(range.dmax) + "` is smaller than the minimum value `" +
                        std::to_string(range.dmin) + "` in `" + dim + "`.",
                    "Specify a range whose maximum value is greater than or equal to its minimum value.",
                    __FILE__,
                    __LINE__
                );
            dimension.push_back(range);
        }
    }

} // namespace snt::dip
