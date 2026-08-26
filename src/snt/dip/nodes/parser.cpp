#include "../helpers.h"
#include "../parsers.h"

#include <iostream>
#include <regex>
#include <snt/dip/exceptions.h>
#include <snt/dip/nodes/parser.h>
#include <sstream>
#include <stdexcept>

// TODO: Using of regular expressions is not efficient, but fast to implement.
//       In the future, this should be optimised!

namespace snt::dip {

    // constexpr std::array<std::string, 2> Parser::ESCAPE_SYMBOLS = {"\\\"", "\\n"};

    void Parser::strip(const std::string& text) {
        code = code.substr(text.length(), code.length());
    }

    bool Parser::do_continue() {
        return code.length();
    }

    /*
     * Escape symbol handling
     */

    void Parser::encode_escape_symbols(std::string& str) {
        std::array<std::string, 2> ESCAPE_SYMBOLS = {"\\\"", "\\n"};
        for (size_t i = 0; i < ESCAPE_SYMBOLS.size(); i++) {
            std::string replace_symbol = "Z@" + std::to_string(i) + ";";
            size_t pos = 0;
            const std::string& symbol = ESCAPE_SYMBOLS[i];
            while ((pos = str.find(symbol, pos)) != std::string::npos) {
                str.replace(pos, symbol.length(), replace_symbol);
                pos += replace_symbol.length();
            }
        }
    }

    void Parser::decode_escape_symbols(std::string& str) {
        std::array<std::string, 2> ESCAPE_SYMBOLS = {"\"", "\n"};
        for (size_t i = 0; i < ESCAPE_SYMBOLS.size(); i++) {
            std::string replace_symbol = "Z@" + std::to_string(i) + ";";
            size_t pos = 0;
            const std::string& symbol = ESCAPE_SYMBOLS[i];
            while ((pos = str.find(replace_symbol, pos)) != std::string::npos) {
                str.replace(pos, replace_symbol.length(), symbol);
                pos += symbol.length();
            }
        }
    }

    /*
     * Directive keywords
     */

    bool Parser::kwd_case() {
        constexpr auto pstr =
            ce_concat<50>("^(", PATTERN_PATH, "*[", SIGN_CONDITION, "](", KEYWORD_IF, "|", KEYWORD_ELIF, "))[ ]*");
        std::regex pattern(pstr.data());
        std::smatch matchResult;
        if (std::regex_search(code, matchResult, pattern)) {
            path = Path(matchResult[1].str());
            strip(matchResult[0].str());
            return true;
        } else {
            constexpr auto pstr = ce_concat<50>(
                "^",
                PATTERN_PATH,
                "*(",
                "[",
                SIGN_CONDITION,
                "]",
                KEYWORD_ELSE,
                "|"
                "[",
                SIGN_CONDITION,
                "]",
                KEYWORD_END,
                ")"
            );
            pattern = pstr.data();
            if (std::regex_search(code, matchResult, pattern)) {
                path = Path(matchResult[0].str());
                strip(matchResult[0].str());
                return true;
            }
        }
        return false;
    }

    bool Parser::kwd_unit() {
        constexpr std::string_view keyword = KEYWORD_UNIT;
        if (code.compare(0, keyword.size(), keyword) != 0)
            return false;
        std::size_t pos = keyword.size();
        while (pos < code.size() && code[pos] == ' ')
            ++pos;
        strip(std::string(code.substr(0, pos)));
        return true;
    }

    bool Parser::kwd_source() {
        constexpr std::string_view keyword = KEYWORD_SOURCE;
        if (code.compare(0, keyword.size(), keyword) != 0)
            return false;
        std::size_t pos = keyword.size();
        while (pos < code.size() && code[pos] == ' ')
            ++pos;
        strip(std::string(code.substr(0, pos)));
        return true;
    }

    bool Parser::kwd_schema() {
        constexpr std::string_view keyword = KEYWORD_SCHEMA;
        if (code.compare(0, keyword.size(), keyword) != 0)
            return false;
        std::size_t pos = keyword.size();
        while (pos < code.size() && code[pos] == ' ')
            ++pos;
        strip(std::string(code.substr(0, pos)));
        return true;
    }

    bool Parser::kwd_property(PropertyType& ptype) {
        if (code.empty() || (code[0] != '!' && code[0] != '?'))
            return false;
        struct Keyword {
            std::string_view text;
            PropertyType type;
        };
        static constexpr Keyword keywords[] = {
            // directives
            {KEYWORD_OPTIONS, PropertyType::Options},
            {KEYWORD_CONSTANT, PropertyType::Constant},
            {KEYWORD_FORMAT, PropertyType::Format},
            {KEYWORD_TAGS, PropertyType::Tags},
            {KEYWORD_CONDITION, PropertyType::Condition},
            {KEYWORD_DELIMITER, PropertyType::Delimiter},
            // metadata
            {KEYWORD_DESCRIPTION, PropertyType::Description},
            {KEYWORD_AUTHORS, PropertyType::Authors},
            {KEYWORD_TITLE, PropertyType::Title},
            {KEYWORD_JOURNAL, PropertyType::Journal},
            {KEYWORD_YEAR, PropertyType::Year},
            {KEYWORD_VOLUME, PropertyType::Volume},
            {KEYWORD_ISSUE, PropertyType::Issue},
            {KEYWORD_PAGES, PropertyType::Pages},
            {KEYWORD_DOI, PropertyType::DOI},
            {KEYWORD_URL, PropertyType::URL},
            {KEYWORD_VERSION, PropertyType::Version},
            {KEYWORD_CREATED, PropertyType::Created},
            {KEYWORD_MODIFIED, PropertyType::Modified},
            {KEYWORD_LICENSE, PropertyType::License},
        };
        for (const auto& kw : keywords) {
            if (code.compare(0, kw.text.size(), kw.text) != 0)
                continue;
            std::size_t pos = kw.text.size();
            while (pos < code.size() && code[pos] == ' ')
                ++pos;
            ptype = kw.type;
            dimension.push_back({0, val::Array::max_range});
            strip(std::string(code.substr(0, pos)));
            return true;
        }
        return false;
    }

    /*
     * Node Parts
     */

    bool Parser::part_trim() {
        std::size_t pos = 0;
        while (pos < code.size() && (code[pos] == ' ' || code[pos] == '\t'))
            ++pos;
        if (pos == 0)
            return false;
        if (pos < code.size() && code[pos] != '\n')
            return false;
        strip(std::string(code.substr(0, pos)));
        return true;
    }

    bool Parser::part_indent() {
        std::size_t pos = 0;
        while (pos < code.size() && code[pos] == ' ')
            ++pos;
        if (pos == 0)
            return false;
        indent = pos;
        strip(std::string(code.substr(0, pos)));
        return true;
    }

    bool Parser::part_path(const bool required) {
        size_t pos = code.find(' ');
        path = Path(code.substr(0, pos));
        strip(path.name);
        return true;
    }

    bool Parser::part_schema() {
        std::size_t pos = 0;
        while (pos < code.size() && code[pos] == ' ')
            ++pos;
        if (pos >= code.size() || code[pos] != ':')
            return false;
        ++pos;
        // parse multiple schema names separated by a comma
        while (true) {
            while (pos < code.size() && code[pos] == ' ')
                ++pos;
            const std::size_t start = pos;
            while (pos < code.size() &&
                   (std::isalnum(static_cast<unsigned char>(code[pos])) || code[pos] == '_' || code[pos] == '-')) {
                ++pos;
            }
            if (start == pos)
                return false;
            value_raw.push_back(code.substr(start, pos - start));
            while (pos < code.size() && code[pos] == ' ')
                ++pos;
            if (pos >= code.size() || code[pos] != ',')
                break;
            ++pos;
        }
        // register value origin remove from the code line
        value_origin = ValueOrigin::Schema;
        strip(code.substr(0, pos));
        return true;
    }

    bool Parser::part_type(const bool required) {
        constexpr std::string_view types[] = {
            KEYWORD_BOOLEAN, KEYWORD_INTEGER, KEYWORD_FLOAT, KEYWORD_STRING, KEYWORD_TABLE, KEYWORD_MAP, KEYWORD_LIST
        };
        constexpr std::string_view precisions[] = {"128", "64", "32", "16", "x"};
        const auto fail = [&] {
            if (required)
                throw dip::SyntaxException(
                    "Could not determine the node type",
                    "The node type must be specified explicitly.",
                    "No valid node type or type precision could be determined from the current line.",
                    "Specify a node type such as boolean, integer, float, string, table, map, or list, optionally with "
                    "a supported precision such as 128, 64, 32, or 16.",
                    __FILE__,
                    __LINE__,
                    line
                );
            return false;
        };
        // stripping empty characters at the beginning
        std::size_t n = code.find_first_not_of(' ');
        if (n == std::string::npos)
            return fail();
        // match types
        const auto match = [&](auto const& values) -> std::string {
            for (auto v : values)
                if (code.compare(n, v.size(), v) == 0) {
                    n += v.size();
                    return std::string(v);
                }
            return {};
        };
        std::string sign, type, precision;
        type = match(types);
        if (type == KEYWORD_MAP || type == KEYWORD_LIST || type == KEYWORD_TABLE) {
            // nothing else
        } else {
            // match signess
            if (type.empty() && n < code.size() && code[n] == 'u') {
                sign = "u";
                ++n;
                type = match(types);
            }
            if (type.empty())
                return fail();
            // match precision
            precision = match(precisions);
        }
        // commit type
        dtype_raw = {sign, type, precision};
        strip(code.substr(0, n));
        return true;
    }

    bool Parser::part_literal_boolean(const std::string& str) {
        if (str == core::KEYWORD_TRUE || str == core::KEYWORD_FALSE) {
            dtype_raw = {"", std::string(KEYWORD_BOOLEAN), ""};
            value_raw.push_back(str);
            value_origin = ValueOrigin::Boolean;
            return true;
        }
        return false;
    }

    bool Parser::part_literal_string(const std::string& str) {
        if ((str.front() == '"' && str.back() == '"')) {
            dtype_raw = {"", std::string(KEYWORD_STRING), ""};
            value_raw.push_back(str.substr(1, str.length() - 2));
            value_origin = ValueOrigin::String;
            return true;
        }
        return false;
    }

    bool Parser::part_literal_integer(const std::string& str) {
        size_t i = 0;
        if (str[i] == '+' || str[i] == '-')
            ++i;
        std::string::difference_type i_signed = static_cast<std::string::difference_type>(i);
        if (i < str.size() && std::all_of(str.begin() + i_signed, str.end(), ::isdigit)) {
            dtype_raw = {"", std::string(KEYWORD_INTEGER), ""};
            value_raw.push_back(str);
            value_origin = ValueOrigin::Number;
            return true;
        }
        return false;
    }

    bool Parser::part_literal_float(const std::string& str) {
        size_t i = 0;
        if (str[i] == '+' || str[i] == '-')
            ++i;
        bool has_digits = false;
        while (i < str.size() && std::isdigit(str[i])) {
            ++i;
            has_digits = true;
        }
        bool is_float = false;
        if (i < str.size() && str[i] == '.') {
            ++i;
            is_float = true;
            while (i < str.size() && std::isdigit(str[i])) {
                ++i;
                has_digits = true;
            }
        }
        if (has_digits && i < str.size() && (str[i] == 'e' || str[i] == 'E')) {
            ++i;
            if (str[i] == '+' || str[i] == '-')
                ++i;
            bool exp_digits = false;
            while (i < str.size() && std::isdigit(str[i])) {
                ++i;
                exp_digits = true;
            }
            if (exp_digits && i == str.size()) {
                dtype_raw = {"", std::string(KEYWORD_FLOAT), ""};
                value_raw.push_back(str);
                value_origin = ValueOrigin::Number;
                return true;
            }
        } else if (is_float && has_digits && i == str.size()) {
            dtype_raw = {"", std::string(KEYWORD_FLOAT), ""};
            value_raw.push_back(str);
            value_origin = ValueOrigin::Number;
            return true;
        }
        return false;
    }

    bool Parser::part_literal_units(const std::string& str) {
        if (str.empty())
            return false;
        char c = str[0];
        if (c == '/' || c == '*' || c == '+' || c == '-')
            return false;
        size_t i = 0;
        while (i < str.size()) {
            char ch = str[i];
            if (ch == '#' || ch == '=' || ch == ' ')
                break;
            ++i;
        }
        if (i == 0)
            return false;
        units_raw = str.substr(0, i);
        strip(units_raw);
        return true;
    }

    bool Parser::part_literal() {
        std::string trimmed = code;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
        if (part_literal_boolean(trimmed))
            return true;
        else if (part_literal_string(trimmed))
            return true;
        size_t pos = trimmed.find(' ');
        if (pos != std::string::npos) {
            std::string number = trimmed.substr(0, pos);
            std::string units = trimmed.substr(pos + 1);
            units.erase(0, units.find_first_not_of(" \t\n\r"));
            if (part_literal_integer(number) && part_literal_units(units))
                return true;
            else if (part_literal_float(number) && part_literal_units(units))
                return true;
        } else {
            if (part_literal_integer(trimmed))
                return true;
            else if (part_literal_float(trimmed))
                return true;
        }
        return false;
    }

    bool Parser::part_dimension() {
        if (code.empty() || code[0] != '[') {
            return false;
        }

        size_t pos = 1;
        while (pos < code.size()) {
            char c = code[pos];
            // End of dimension block
            if (c == ']') {
                std::string slices = code.substr(1, pos - 1);
                parse_slices(slices, dimension);
                if (dimension.empty()) {
                    throw dip::SyntaxException(
                        "Empty dimension settings",
                        "The dimension block must specify at least one array dimension.",
                        "The dimension block is empty or does not contain a valid dimension specification.",
                        "Specify at least one dimension using `:`, `x:`, `:y`, or `x:y`; separate multiple dimensions "
                        "with commas, for example `[:,:10]` or `[1:10,:]`.",
                        __FILE__,
                        __LINE__,
                        line
                    );
                }
                strip(code.substr(0, pos + 1));
                return true;
            }
            // Allowed characters
            if (!(std::isdigit(static_cast<unsigned char>(c)) || c == ':' || c == ',')) {
                return false;
            }
            ++pos;
        }

        return false;
    }

    bool Parser::part_equal(bool required) {
        std::size_t pos = 0;
        while (pos < code.size() && code[pos] == ' ')
            ++pos;
        if (pos >= code.size() || code[pos] != SIGN_EQUAL) {
            if (required) {
                throw dip::SyntaxException(
                    "Missing assignment operator",
                    "The `=` sign must be present at this position.",
                    "The expected `=` sign was not found.",
                    "Add an `=` sign after the preceding expression.",
                    __FILE__,
                    __LINE__,
                    line
                );
            }
            return false;
        }
        ++pos;
        while (pos < code.size() && code[pos] == ' ')
            ++pos;
        strip(std::string(code.substr(0, pos)));
        return true;
    }

    bool Parser::part_reference() {
        size_t pos = 0;
        // match all empty characters
        while (pos < code.size() && std::isspace(static_cast<unsigned char>(code[pos])))
            ++pos;
        size_t pos_start = pos + 1;
        // start with {
        if (pos >= code.size() || code[pos] != '{')
            return false;
        ++pos;
        // relative path starts with dots .
        int parent = 0;
        while (pos < code.size() && code[pos] == SIGN_SEPARATOR) {
            parent++;
            ++pos;
        }
        // match a source keyword
        std::string keyword;
        while (pos < code.size()) {
            unsigned char c = static_cast<unsigned char>(code[pos]);
            if (std::isalnum(c) || c == '_' || c == '-') {
                keyword += static_cast<char>(c);
                ++pos;
            } else {
                break;
            }
        }
        // match a node path
        if (pos < code.size() && code[pos] == '?') { // absolute path {keyword?path}
            ++pos;
            size_t path_begin = pos;
            while (pos < code.size() && code[pos] != '}')
                ++pos;
            if (pos == code.size())
                throw dip::SyntaxException(
                    "Missing closing brace",
                    "A reference expression must be terminated with `}`.",
                    "The reference expression was not closed.",
                    "Add a closing `}` after the reference path.",
                    __FILE__,
                    __LINE__,
                    line
                );
            std::string path = code.substr(path_begin, pos - path_begin);
            value_origin = ValueOrigin::Reference;
            if (!path.empty())
                Path expr(path); // test if request is a fully qualified path?
        } else {
            if (parent == 1) // self-reference {.} or relative reference {.path}
                value_origin = ValueOrigin::ReferenceRel;
            else if (keyword.empty())
                throw dip::SyntaxException(
                    "Empty reference",
                    "A non-relative reference without query must specify a source.",
                    "The reference does not contain a source keyword.",
                    "Specify a reference source, for example `{source}`.",
                    __FILE__,
                    __LINE__,
                    line
                );
            else if (parent > 1) // relative reference {...path}
                value_origin = ValueOrigin::ReferenceRel;
            else // raw reference {source}
                value_origin = ValueOrigin::ReferenceRaw;
        }
        // match closing }
        if (pos >= code.size() || code[pos] != '}')
            return false;
        ++pos;
        // Commit only after the entire parse succeeded.
        value_raw.push_back(code.substr(pos_start, pos - pos_start - 1));
        strip(code.substr(0, pos));
        part_slice();
        return true;
    }

    bool Parser::part_function() {
        constexpr auto pstr = ce_concat<50>("^[ ]*(", PATTERN_KEYWORD, "+)[(][)]");
        std::regex pattern(pstr.data());
        std::smatch matchResult;
        if (std::regex_search(code, matchResult, pattern)) {
            value_raw.push_back(matchResult[1].str());
            value_origin = ValueOrigin::Function;
            strip(matchResult[0].str());
            return true;
        }
        return false;
    }

    bool Parser::part_expression() {
        if (code.empty())
            return false;
        // Skip leading whitespace
        size_t start = code.find_first_not_of(" \t\n\r");
        // Try to parse numerical or logical expression
        if (start == std::string::npos || (code[start] != '('))
            return false;
        if (dtype_raw[1] == std::string(KEYWORD_STRING))
            throw dip::SyntaxException(
                "Invalid template expression",
                "String template expressions must use f-prefixed string notation.",
                "A parenthesized expression was used for a string template.",
                "Use an f-prefixed string such as `f\"str\"` or `f\"\"\"str\"\"\"`.",
                __FILE__,
                __LINE__,
                line
            );
        int depth = 0;
        size_t i = start;
        // Parse parentheses from first '('
        for (; i < code.size(); ++i) {
            if (code[i] == '(')
                depth++;
            else if (code[i] == ')') {
                depth--;
                if (depth == 0)
                    break;
            }
        }
        // No matching closing parenthesis
        if (depth != 0)
            throw dip::SyntaxException(
                "Unclosed expression",
                "The expression must have a matching closing parenthesis.",
                "The opening parenthesis was not closed.",
                "Add a closing `)` to complete the expression.",
                __FILE__,
                __LINE__,
                line
            );
        // Extract inside content
        std::string inside = code.substr(start + 1, i - start - 1);
        if (inside.empty())
            throw dip::ParserException(
                "Empty expression",
                "The expression must contain a value or operation.",
                "The expression contains no content between the parentheses.",
                "Add a valid expression between the parentheses.",
                __FILE__,
                __LINE__,
                line
            );
        value_raw.push_back(inside);
        value_origin = ValueOrigin::Expression;
        // Strip including leading whitespace + full "(...)"
        strip(code.substr(0, i + 1));
        return true;
    }

    bool Parser::part_array() {
        if (code.empty() || code.at(0) != SIGN_ARRAY_OPEN)
            return false;
        std::string rm = parse_array(code, value_raw, value_shape);
        value_origin = ValueOrigin::Array;
        strip(rm);
        return true;
    }

    bool Parser::part_string() {
        if (code.empty()) {
            return false;
        }
        bool is_expression = false;
        size_t start = 0;

        // Detect f-prefixed strings
        if (code[0] == 'f') {
            is_expression = true;
            start = 1;
            if (start >= code.size() || code[start] != '"') {
                return false;
            }
        }

        // Triple-quoted string
        if (code.compare(start, 3, "\"\"\"") == 0) {
            size_t content_start = start + 3;
            size_t pos = content_start;
            while (pos + 2 < code.size()) {
                if (code[pos] == '"' && code[pos + 1] == '"' && code[pos + 2] == '"') {
                    std::string value = code.substr(content_start, pos - content_start);
                    value_raw.push_back(value);
                    value_origin = is_expression ? ValueOrigin::Expression : ValueOrigin::String;
                    strip(code.substr(0, pos + 3));
                    return true;
                }
                ++pos;
            }
            return false;
        }

        // Normal quoted string
        if (code[start] == '"') {
            size_t pos = start + 1;
            bool escaped = false;
            while (pos < code.size()) {
                char c = code[pos];
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '"') {
                    std::string value = code.substr(start + 1, pos - start - 1);
                    value_raw.push_back(value);
                    value_origin = is_expression ? ValueOrigin::Expression : ValueOrigin::String;
                    strip(code.substr(0, pos + 1));
                    return true;
                }
                ++pos;
            }
        }
        return false;
    }

    bool Parser::part_number(const bool required, const char delimiter) {
        constexpr auto pstr = ce_concat<50>("^", PATTERN_NUMBER);
        std::regex pattern(pstr.data());
        std::smatch matchResult;
        if (std::regex_search(code, matchResult, pattern)) {
            value_raw.push_back(matchResult[0].str());
            value_origin = ValueOrigin::Number;
            strip(matchResult[0].str());
            if (do_continue() && code[0] != delimiter)
                throw dip::SyntaxException(
                    "Incomplete number",
                    "The complete number must end before the expected delimiter.",
                    "Only part of the number could be parsed.",
                    "Check the number format and make sure it is followed by the expected delimiter.",
                    __FILE__,
                    __LINE__,
                    line
                );
            return true;
        } else if (required) {
            throw dip::SyntaxException(
                "Invalid number",
                "The value must be a valid number.",
                "The input does not match the expected number format.",
                "Check the number format and provide a valid numerical value.",
                __FILE__,
                __LINE__,
                line
            );
        }
        return false;
    }

    bool Parser::part_keyword(const bool required, const char delimiter) {
        constexpr auto pstr = ce_concat<50>("^", PATTERN_KEYWORD, "+");
        std::regex pattern(pstr.data());
        std::smatch matchResult;
        if (std::regex_search(code, matchResult, pattern)) {
            value_raw.push_back(matchResult[0].str());
            value_origin = ValueOrigin::Keyword;
            strip(matchResult[0].str());
            if (do_continue() && code[0] != delimiter)
                throw dip::SyntaxException(
                    "Incomplete keyword",
                    "The complete keyword must end before the expected delimiter '" + std::string(1, delimiter) + "'.",
                    "Only part of the keyword could be parsed.",
                    "Check the keyword and make sure it is followed by the expected delimiter.",
                    __FILE__,
                    __LINE__,
                    line
                );
            return true;
        } else if (required) {
            throw dip::SyntaxException(
                "Invalid keyword",
                "The keyword must contain only letters, digits, underscores, and hyphens.",
                "The input does not match the expected keyword format: " + code,
                "Use only characters from `a-z`, `A-Z`, `0-9`, `_`, and `-`.",
                __FILE__,
                __LINE__,
                line
            );
        }
        return false;
    }

    bool Parser::part_none() {
        auto pos = code.find_first_not_of(" ");
        if (pos == std::string::npos)
            return false; // string contains only whitespace
        if (code.compare(pos, 4, KEYWORD_NONE) == 0) {
            value_origin = ValueOrigin::None;
            strip(code.substr(0, pos + 4));
            return true;
        }
        return false;
    }

    bool Parser::part_value() {
        if (part_reference())
            return true;
        if (part_function())
            return true;
        if (part_expression())
            return true;
        if (part_array())
            return true;
        if (part_string())
            return true;
        if (part_number(false))
            return true;
        if (part_none()) // needs to test before of keywords
            return true;
        if (part_keyword(false))
            return true;
        return false;
    }

    bool Parser::part_slice() {
        std::regex pattern(R"(^\[([0-9:,]*)\])");
        std::smatch matchResult;
        if (std::regex_search(code, matchResult, pattern)) {
            std::string slices = matchResult[1].str();
            parse_slices(slices, value_slice);
            if (value_slice.empty())
                return false;
            strip(matchResult[0].str());
            return true;
        }
        return false;
    }

    bool Parser::part_format() {
        std::regex pattern(R"(^:([0-9]*)(?:[.]([0-9]+))?([sfegd]+))"); //^\\[([0-9:,]*)\\]");
        std::smatch matchResult;
        if (std::regex_search(code, matchResult, pattern)) {
            formatting = {matchResult[1].str(), matchResult[2].str(), matchResult[3].str()};
            strip(matchResult[0].str());
            return true;
        }
        return false;
    }

    bool Parser::part_units(const char delimiter) {
        // Check delimiter, but don't strip yet
        if (delimiter != '\0') {
            if (code.empty() || code[0] != delimiter)
                return false;
        }
        // Run regex on the correct substring
        std::string target = (delimiter != '\0') ? code.substr(1) : code;
        // In numerical expressions starting signs +-*/ have to be explicitely excluded
        std::regex pattern(R"(^(?![/*+\-]+)([^#= ]+))");
        std::smatch matchResult;
        if (std::regex_search(target, matchResult, pattern)) {
            units_raw = matchResult[1].str();
            // Now strip delimiter + match
            if (delimiter != '\0') {
                strip(std::string(1, delimiter));
            }
            strip(matchResult[0].str());
            return true;
        }
        return false;
    }

    bool Parser::part_comment() {
        size_t i = 0;
        // Skip leading spaces.
        while (i < code.size() && code[i] == ' ')
            ++i;
        // Must start with '#'.
        if (i == code.size() || code[i] != '#')
            return false;
        ++i;
        // Skip spaces after '#'.
        while (i < code.size() && code[i] == ' ')
            ++i;
        comment = code.substr(i);
        strip(code);
        return true;
    }

    bool Parser::part_delimiter(const char symbol, const bool required) {
        std::ostringstream oss;
        oss << "^[ ]*[" << symbol << "][ ]*";
        std::regex pattern(oss.str());
        std::smatch matchResult;
        if (std::regex_search(code, matchResult, pattern)) {
            strip(matchResult[0].str());
            return true;
        } else if (required) {
            throw dip::SyntaxException(
                "Missing delimiter",
                "The delimiter `" + std::string(1, symbol) + "` must be present at this position.",
                "The expected delimiter was not found.",
                "Add the `" + std::string(1, symbol) + "` delimiter at this position.",
                __FILE__,
                __LINE__,
                line
            );
        }
        return false;
    }

} // namespace snt::dip
