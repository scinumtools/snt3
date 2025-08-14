#include "unit_format.h"

namespace puq {

  const std::unordered_map<char, std::string> UnitFormat::superscript_map = {
      {'0', "\u2070"}, {'1', "\u00B9"}, {'2', "\u00B2"}, {'3', "\u00B3"}, // 0 1 2 3 -> ⁰ ¹ ² ³
      {'4', "\u2074"},
      {'5', "\u2075"},
      {'6', "\u2076"},
      {'7', "\u2077"}, // 4 5 6 7 -> ⁴ ⁵ ⁶ ⁷
      {'8', "\u2078"},
      {'9', "\u2079"},                        // 8 9 -> ⁸ ⁹
      {SYMBOL_MINUS[0], "\u207B"},            // - -> ⁻
      {SYMBOL_PLUS[0], ""},                   // + ->
      {SYMBOL_FRACTION[0], SYMBOL_FRACTION2}, // : -> ᐟ
  };

  const std::unordered_map<std::string, std::string> UnitFormat::symbol_map = {
      {SYMBOL_EXPONENT2, SYMBOL_EXPONENT}, // ×10 -> e
      {SYMBOL_MINUS2, SYMBOL_MINUS},       // −   -> -
      {SYMBOL_MULTIPLY2, SYMBOL_MULTIPLY}, // ⋅   -> *
  };

  bool UnitFormat::preprocess_system(std::string& expression, const std::string& abbrev) {
    std::string prefix = abbrev + SYMBOL_SYSTEM;
    size_t pos = expression.find(prefix);
    if (pos != std::string::npos && pos == 0) {
      expression = expression.substr(prefix.size(), expression.size());
      return true;
    }
    prefix = abbrev + " ";
    pos = expression.find(prefix);
    if (pos != std::string::npos && pos == 0) {
      expression = expression.substr(prefix.size(), expression.size());
      return true;
    }
    return false;
  }

  void UnitFormat::preprocess_symbols(std::string& expression) {
    // convert superscripts
    for (auto item : UnitFormat::superscript_map) {
      if (item.second == "")
        continue;
      size_t pos = expression.find(item.second);
      while (pos != std::string::npos) {
        std::string first(1, item.first);
        expression.replace(pos, item.second.size(), first);
        pos = expression.find(item.second, pos + item.second.size());
      }
    }
    // convert mathematical symbols
    for (auto item : UnitFormat::symbol_map) {
      size_t pos = expression.find(item.first);
      while (pos != std::string::npos) {
        expression.replace(pos, item.first.size(), item.second);
        pos = expression.find(item.first, pos + item.first.size());
      }
    }
  }

  std::string UnitFormat::multiply_symbol() const {
    if (math == Format::Math::UNICODE || math == Format::Math::HTML) {
      return SYMBOL_MULTIPLY2;
    }
    return SYMBOL_MULTIPLY;
  }

  std::string UnitFormat::format_exponents(std::string expression) const {
    if (math != Format::Math::ASCII) {
      // removing leading zeros
      if (expression[0] == '0') {
        expression.erase(0, 0);
      } else if (expression[0] == SYMBOL_PLUS[0] || expression[0] == SYMBOL_MINUS[0]) {
        if (expression[1] == '0')
          expression.erase(1, 1);
      }
    }
    if (math == Format::Math::UNICODE) {
      // translating speccial characters
      std::string superscript_str;
      for (char c : expression) {
        superscript_str += UnitFormat::superscript_map.at(c);
      }
      return superscript_str;
    } else if (math == Format::Math::HTML) {
      return "<sup>" + expression + "</sup>";
    }
    return expression;
  }

  std::string UnitFormat::format_order(std::string expression) const {
    if (math == Format::Math::UNICODE || math == Format::Math::HTML) {
      size_t pos = expression.find(SYMBOL_EXPONENT);
      if (pos != std::string::npos) {
        std::string exponent_str = expression.substr(pos + std::string(SYMBOL_EXPONENT).size(), expression.size());
        expression = expression.substr(0, pos) + std::string(SYMBOL_EXPONENT2) + format_exponents(exponent_str);
      }
    }
    return expression;
  }

  std::string UnitFormat::format_system(std::string expression, const std::string& abbrev) const {
    if (system == Format::System::SHOW) {
      if (math == Format::Math::UNICODE || math == Format::Math::HTML)
        return abbrev + " " + expression;
      else
        return abbrev + SYMBOL_SYSTEM + expression;
    }
    return expression;
  };

  bool UnitFormat::display_magnitude() const {
    return (part == Format::Display::BOTH) || (part == Format::Display::MAGNITUDE);
  }

  bool UnitFormat::display_units() const {
    return (part == Format::Display::BOTH) || (part == Format::Display::UNITS);
  }

  bool UnitFormat::display_error() const {
    return (error == Format::Error::SHOW);
  }

} // namespace puq
