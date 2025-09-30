#include "parser.h"

#include "../helpers.h"
#include "../parsers.h"

#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace snt::dip {

  const std::array<std::string, 3> Parser::ESCAPE_SYMBOLS = {"\\\"", "\\'", "\\n"};

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
    for (int i = 0; i < ESCAPE_SYMBOLS.size(); i++) {
      std::string replace_symbol = "Z@" + std::to_string(i) + ";";
      size_t pos = 0;
      while ((pos = str.find(ESCAPE_SYMBOLS[i], pos)) != std::string::npos) {
        str.replace(pos, ESCAPE_SYMBOLS[i].length(), replace_symbol);
        pos += replace_symbol.length();
      }
    }
  }

  void Parser::decode_escape_symbols(std::string& str) {
    for (int i = 0; i < ESCAPE_SYMBOLS.size(); i++) {
      std::string replace_symbol = "Z@" + std::to_string(i) + ";";
      size_t pos = 0;
      while ((pos = str.find(replace_symbol, pos)) != std::string::npos) {
        str.replace(pos, replace_symbol.length(), ESCAPE_SYMBOLS[i]);
        pos += ESCAPE_SYMBOLS[i].length();
      }
    }
  }

  /*
   * Directive keywords
   */

  bool Parser::kwd_case() {
    constexpr auto pstr =
        ce_concat<50>("^(", PATTERN_PATH, "*[", SIGN_CONDITION, "]", KEYWORD_CASE, ")[ ]*");
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
    constexpr auto pstr = ce_concat<50>("^[", SIGN_VALIDATION, "](", PATTERN_KEY, "+)[ ]*");
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
    std::regex pattern("^[ ]+");
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

  bool Parser::part_literal() {
    std::string trimmed = code;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    // Check Boolean
    if (trimmed == snt::KEYWORD_TRUE || trimmed == snt::KEYWORD_FALSE) {
      dtype_raw = {"", std::string(KEYWORD_BOOLEAN), ""};
      value_raw.push_back(trimmed);
      value_origin = ValueOrigin::String;
      return true;
    }
    // Check String (quoted with "" or '')
    if ((trimmed.front() == '"' && trimmed.back() == '"') ||
        (trimmed.front() == '\'' && trimmed.back() == '\'')) {
      dtype_raw = {"", std::string(KEYWORD_STRING), ""};
      value_raw.push_back(trimmed.substr(1, trimmed.length() - 2));
      value_origin = ValueOrigin::String;
      return true;
    }
    // Check Integer
    size_t i = 0;
    if (trimmed[i] == '+' || trimmed[i] == '-')
      ++i;
    std::string::difference_type i_signed = static_cast<std::string::difference_type>(i);
    if (i < trimmed.size() && std::all_of(trimmed.begin() + i_signed, trimmed.end(), ::isdigit)) {
      dtype_raw = {"", std::string(KEYWORD_INTEGER), ""};
      value_raw.push_back(trimmed);
      value_origin = ValueOrigin::String;
      return true;
    }
    // Check Float
    bool has_digits = false;
    while (i < trimmed.size() && std::isdigit(trimmed[i])) {
      ++i;
      has_digits = true;
    }
    bool is_float = false;
    if (i < trimmed.size() && trimmed[i] == '.') {
      ++i;
      is_float = true;
      while (i < trimmed.size() && std::isdigit(trimmed[i])) {
        ++i;
        has_digits = true;
      }
    }
    if (has_digits && i < trimmed.size() && (trimmed[i] == 'e' || trimmed[i] == 'E')) {
      ++i;
      if (trimmed[i] == '+' || trimmed[i] == '-')
        ++i;
      bool exp_digits = false;
      while (i < trimmed.size() && std::isdigit(trimmed[i])) {
        ++i;
        exp_digits = true;
      }
      if (exp_digits && i == trimmed.size()) {
        dtype_raw = {"", std::string(KEYWORD_FLOAT), ""};
        value_raw.push_back(trimmed);
        value_origin = ValueOrigin::String;
        return true;
      }
    } else if (is_float && has_digits && i == trimmed.size()) {
      dtype_raw = {"", std::string(KEYWORD_FLOAT), ""};
      value_raw.push_back(trimmed);
      value_origin = ValueOrigin::String;
      return true;
    }
    return false;
  }

  bool Parser::part_dimension() {
    std::regex pattern("^\\[([0-9:,]*)\\]");
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
    constexpr auto pstr = ce_concat<50>("^[ ]*[{](", PATTERN_KEY, "*([?]", PATTERN_PATH, "*|))[}]");
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
    constexpr auto pstr = ce_concat<50>("^[ ]*[(](", PATTERN_KEY, "+)[)]");
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
    std::regex pattern("^[(](\"\"\"([^\"]*)\"\"\"|\"([^\"]*)\"|\'([^\']*)\')[)]");
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      if (matchResult[2].length())
        value_raw.push_back(matchResult[2].str());
      else if (matchResult[3].length())
        value_raw.push_back(matchResult[3].str());
      else if (matchResult[4].length())
        value_raw.push_back(matchResult[4].str());
      else
        throw std::runtime_error("Expression cannot be an empty string: " + line.code);
      value_origin = ValueOrigin::Expression;
      strip(matchResult[0].str());
      return true;
    }
    return false;
  }

  bool Parser::part_array() {
    if (code.empty() or code.at(0) != SIGN_ARRAY_OPEN)
      return false;
    std::string rm = parse_array(code, value_raw, value_shape);
    strip(rm);
    return true;
  }

  bool Parser::part_string() {
    std::regex pattern("^(\"\"\"([^\"]*)\"\"\"|\"([^\"]*)\"|\'([^\']*)\'|((?!#)[^ ]+))");
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

  bool Parser::part_keyword(const bool required) {
    constexpr auto pstr = ce_concat<50>("^", PATTERN_KEY, "+");
    std::regex pattern(pstr.data());
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      value_raw.push_back(matchResult[0].str());
      value_origin = ValueOrigin::Keyword;
      strip(matchResult[0].str());
      if (do_continue() and code[0] != ' ')
        throw std::runtime_error("Key has an invalid format: " + line.code);
      return true;
    } else if (required) {
      throw std::runtime_error("Key has an invalid format: " + line.code);
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
    return false;
  }

  bool Parser::part_slice() {
    std::regex pattern("^\\[([0-9:,]*)\\]");
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
    std::regex pattern("^:([0-9]*)(?:[.]([0-9]+))?([sfeg]+)"); //^\\[([0-9:,]*)\\]");
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern)) {
      formatting = {matchResult[1].str(), matchResult[2].str(), matchResult[3].str()};
      strip(matchResult[0].str());
      return true;
    }
    return false;
  }

  bool Parser::part_units() {
    // In numerical expressions starting signs +-*/ have to be explicitely excluded
    std::regex pattern1("^[ ]+([^#= ]+)");
    std::regex pattern2("^[ ]+[/*+-]+");
    std::smatch matchResult;
    if (std::regex_search(code, matchResult, pattern1) and !std::regex_match(code, pattern2)) {
      units_raw = matchResult[1].str();
      strip(matchResult[0].str());
      return true;
    }
    return false;
  }

  bool Parser::part_comment() {
    std::regex pattern("^[ ]*#[ ]*(.*)$");
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
