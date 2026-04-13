#include "../helpers.h"
#include "../parsers.h"

#include <iostream>
#include <regex>
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
      name = matchResult[1].str();
      strip(matchResult[0].str());
      return true;
    } else {
      constexpr auto pstr =
          ce_concat<50>("^", PATTERN_PATH, "*(", "[", SIGN_CONDITION, "]", KEYWORD_ELSE,
                        "|"
                        "[",
                        SIGN_CONDITION, "]", KEYWORD_END, ")");
      pattern = pstr.data();
      if (std::regex_search(code, matchResult, pattern)) {
        name = matchResult[0].str();
        strip(matchResult[0].str());
        return true;
      }
    }
    return false;
  }

  bool Parser::kwd_unit() {
    constexpr auto pstr =
        ce_concat<50>("^(", PATTERN_PATH, "*[", SIGN_VARIABLE, "]", KEYWORD_UNIT, ")[ ]*");
    std::regex pattern(pstr.data());
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      name = matchResult[1].str();
      strip(matchResult[0].str());
      return true;
    }
    return false;
  }

  bool Parser::kwd_source() {
    constexpr auto pstr =
        ce_concat<50>("^(", PATTERN_PATH, "*[", SIGN_VARIABLE, "]", KEYWORD_SOURCE, ")[ ]*");
    std::regex pattern(pstr.data());
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      name = matchResult[1].str();
      strip(matchResult[0].str());
      return true;
    }
    return false;
  }

  bool Parser::kwd_property(PropertyType& ptype) {
    constexpr auto pstr = ce_concat<50>("^[", SIGN_VALIDATION, "](", PATTERN_KEYWORD, "+)[ ]*");
    std::regex pattern(pstr.data());
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      std::string key = matchResult[1].str();
      if (key == KEYWORD_OPTIONS)
        ptype = PropertyType::Options;
      else if (key == KEYWORD_CONSTANT)
        ptype = PropertyType::Constant;
      else if (key == KEYWORD_FORMAT)
        ptype = PropertyType::Format;
      else if (key == KEYWORD_TAGS)
        ptype = PropertyType::Tags;
      else if (key == KEYWORD_DESCRIPTION)
        ptype = PropertyType::Description;
      else if (key == KEYWORD_CONDITION)
        ptype = PropertyType::Condition;
      else if (key == KEYWORD_DELIMITER)
        ptype = PropertyType::Delimiter;
      dimension.push_back({0, val::Array::max_range});
      strip(matchResult[0].str());
      return true;
    }
    return false;
  }

  /*
   * Node Parts
   */

  bool Parser::part_space(const bool required) {
    return false;
  }

  bool Parser::part_indent() {
    std::regex pattern(R"(^[ ]+)");
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      indent = matchResult[0].str().length();
      strip(matchResult[0].str());
      return true;
    }
    return false;
  }

  bool Parser::part_name(const bool required) {
    constexpr auto pstr = ce_concat<50>("^", PATTERN_PATH, "+");
    std::regex pattern(pstr.data());
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      name = matchResult[0].str();
      strip(matchResult[0].str());
      if (do_continue() and code[0] != ' ')
        throw std::runtime_error("Name has an invalid format: " + line.code);
      return true;
    } else if (required) {
      throw std::runtime_error("Name has an invalid format: " + line.code);
    }
    return false;
  }

  bool Parser::part_type(const bool required) {
    constexpr auto pstr =
        ce_concat<70>("^[ ]+(u|)(", KEYWORD_BOOLEAN, "|", KEYWORD_INTEGER, "|", KEYWORD_FLOAT, "|",
                      KEYWORD_STRING, "|table)(16|32|64|128|x|)");
    std::regex pattern(pstr.data());
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      dtype_raw = {matchResult[1].str(), matchResult[2].str(), matchResult[3].str()};
      strip(matchResult[0].str());
      return true;
    } else if (required) {
      throw std::runtime_error("Type not recognized: " + line.code);
    }
    return false;
  }

  bool Parser::part_literal_boolean(const std::string& str) {
    if (str == core::KEYWORD_TRUE || str == core::KEYWORD_FALSE) {
      dtype_raw = {"", std::string(KEYWORD_BOOLEAN), ""};
      value_raw.push_back(str);
      value_origin = ValueOrigin::String;
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
      value_origin = ValueOrigin::String;
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
        value_origin = ValueOrigin::String;
        return true;
      }
    } else if (is_float && has_digits && i == str.size()) {
      dtype_raw = {"", std::string(KEYWORD_FLOAT), ""};
      value_raw.push_back(str);
      value_origin = ValueOrigin::String;
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
      std::string units  = trimmed.substr(pos + 1);
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
    std::regex pattern(R"(^\[([0-9:,]*)\])");
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      std::string slices = matchResult[1].str();
      parse_slices(slices, dimension);
      if (dimension.empty())
        throw std::runtime_error("Dimension settings cannot be empty: " + line.code);
      strip(matchResult[0].str());
      return true;
    }
    return false;
  }

  bool Parser::part_equal(const bool required) {
    constexpr auto pstr = ce_concat<50>("^[ ]*[", SIGN_EQUAL, "][ ]*");
    std::regex pattern(pstr.data());
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      strip(matchResult[0].str());
      return true;
    } else if (required) {
      throw std::runtime_error("Equal sign '" + std::string(1, SIGN_EQUAL) +
                               "' is required: " + line.code);
    }
    return false;
  }

  bool Parser::part_reference() {
    constexpr auto pstr = ce_concat<50>("^[ ]*[{](", PATTERN_KEYWORD, "*([?]", PATTERN_PATH, "*|))[}]");
    std::regex pattern(pstr.data());
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      value_raw.push_back(matchResult[1].str());
      if (!matchResult[2].str().empty())
        value_origin = ValueOrigin::Reference;
      else if (!matchResult[1].str().empty())
        value_origin = ValueOrigin::ReferenceRaw;
      else
        throw std::runtime_error("Reference cannot be empty: " + line.code);
      strip(matchResult[0].str());
      part_slice();
      return true;
    }
    return false;
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
    if (start == std::string::npos || code[start] != '(')
      return false;
    int depth = 0;
    size_t i = start;
    // Parse parentheses from first '('
    for (; i < code.size(); ++i) {
      if (code[i] == '(') depth++;
      else if (code[i] == ')') {
	depth--;
	if (depth == 0) break;
      }
    }
    // No matching closing parenthesis
    if (depth != 0)
      throw std::runtime_error("Expression contains unclosed parentheses: " + line.code);    
    // Extract inside content
    std::string inside = code.substr(start + 1, i - start - 1);
    if (inside.empty())
      throw std::runtime_error("Expression cannot be empty: " + line.code);    
    value_raw.push_back(inside);
    value_origin = ValueOrigin::Expression;    
    // Strip including leading whitespace + full "(...)"
    strip(code.substr(0, i + 1));
    return true;
  }

  bool Parser::part_array() {
    if (code.empty() or code.at(0) != SIGN_ARRAY_OPEN)
      return false;
    std::string rm = parse_array(code, value_raw, value_shape);
    strip(rm);
    return true;
  }

  bool Parser::part_string() {
    std::regex pattern(R"(^(\"\"\"([^\"]*)\"\"\"|\"([^\"]*)\"))");
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      for (int i = 2; i < 6; i++) {
        std::string vraw = matchResult[i].str();
        if (vraw != "") {
          value_raw.push_back(vraw);
          value_origin = ValueOrigin::String;
          break;
        }
      }
      strip(matchResult[0].str());
      return true;
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
      if (do_continue() and code[0] != delimiter)
        throw std::runtime_error("Number was not fully parsed: " + line.code);
      return true;
    } else if (required) {
      throw std::runtime_error("Number has an invalid format: " + line.code);
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
      if (do_continue() and code[0] != delimiter)
        throw std::runtime_error("Keyword was not fully parsed: " + line.code);
      return true;
    } else if (required) {
      throw std::runtime_error("Keyword has an invalid format: " + line.code);
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
    std::regex pattern(R"(^:([0-9]*)(?:[.]([0-9]+))?([sfeg]+))"); //^\\[([0-9:,]*)\\]");
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
    std::regex pattern(R"(^[ ]*#[ ]*(.*)$)");
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      comment = matchResult[1].str();
      strip(matchResult[0].str());
      return true;
    }
    return false;
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
      throw std::runtime_error("Delimiter '" + std::string(1, symbol) +
                               "' is required: " + line.code);
    }
    return false;
  }

} // namespace snt::dip
