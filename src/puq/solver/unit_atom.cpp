#include "unit_atom.h"

#include "../nostd/nostd.h"

#include <algorithm>
#include <regex>

namespace puq {

  inline void _parse_number(std::string& expr, UnitValue& uv, const std::smatch& m) {
    if (m[6] == "") {
#ifdef MAGNITUDE_ERRORS
#if defined(MAGNITUDE_ARRAYS)
      uv.magnitude.value = nostd::to_number(expr);
#elif defined(MAGNITUDE_VALUES)
      uv.magnitude.value = std::make_unique<val::ArrayValue<double>>(nostd::to_number(expr));
#endif
#else
      uv.magnitude = nostd::to_number(expr);
#endif
    } else {
      std::string decimals = m[3].str() == "" ? "." : m[3].str();
#ifdef MAGNITUDE_ERRORS
#if defined(MAGNITUDE_ARRAYS)
      uv.magnitude.value = nostd::to_number(m[1].str() + decimals + m[8].str());
      if (m[10] == "")
        uv.magnitude.error = nostd::to_number(m[7]) * std::pow(10, 1 - (int)decimals.size());
      else
        uv.magnitude.error = nostd::to_number(m[7]) * std::pow(10, 1 - (int)decimals.size() + std::stoi(m[10]));
#elif defined(MAGNITUDE_VALUES)
      uv.magnitude.value = std::make_unique<val::ArrayValue<double>>(nostd::to_number(m[1].str() + decimals + m[8].str()));
      if (m[10] == "")
        uv.magnitude.error = std::make_unique<val::ArrayValue<double>>(nostd::to_number(m[7]) * std::pow(10, 1 - (int)decimals.size()));
      else
        uv.magnitude.error = std::make_unique<val::ArrayValue<double>>(nostd::to_number(m[7]) * std::pow(10, 1 - (int)decimals.size() + std::stoi(m[10])));
#endif
#else
      uv.magnitude = nostd::to_number(m[1].str() + decimals + m[8].str());
#endif
    }
  }

  inline void _parse_exponent(BaseUnit& bu, std::string& expr, const std::smatch& m) {
#ifdef EXPONENT_FRACTIONS
    bu.exponent = Exponent((m[2] == "" ? 1 : std::stoi(m[2])), (m[4] == "" ? 1 : std::stoi(m[4])));
#else
    bu.exponent = m[2] == "" ? 1 : std::stoi(m[2]);
#endif
    expr = std::string(m[1]);
  }

  inline void _parse_quantity(std::string& expr, UnitValue& uv, const std::smatch& m) {
    BaseUnit bu;
    _parse_exponent(bu, expr, m);
    bu.unit = expr;
    uv.magnitude = 1;
    uv.baseunits.append(bu);
  }

  inline void _parse_unit(std::string& expr, UnitValue& uv, const std::smatch& m, const std::string& expr_orig) {
    BaseUnit bu;
    _parse_exponent(bu, expr, m);
    // determine unit
    std::pair<std::string, UnitStruct> munit; // current candidate unit
    for (auto& unit : UnitSystem::Data->UnitList) {
      if (unit.first.size() > expr.size()) // symbol is longer than the expression
        continue;
      if (unit.first.size() <= munit.first.size()) // symbol is smaller or equal to the current candidate symbol
        continue;
      if (expr.compare(expr.size() - unit.first.size(), unit.first.size(), unit.first) == 0) {
        munit = unit;
      }
    }
    if (munit.first == "") {
      throw AtomParsingExcept("Unknown unit base \"" + expr_orig + "\" in " + UnitSystem::Data->SystemAbbrev + " system!");
    } else {
      bu.unit = munit.first;
    }
    expr = expr.substr(0, expr.size() - bu.unit.size());
    // determine prefix
    if (expr.size() > 0) {
      // no prefixes are allowed
      if (!munit.second.use_prefixes)
        throw AtomParsingExcept("Prefixes are not allowed for this unit: " + expr_orig);
      // is symbol in the prefix list
      if (UnitPrefixList.find(expr) == UnitPrefixList.end())
        throw AtomParsingExcept("Unknown prefix '" + expr + "' detected in unit: " + expr_orig);
      else
        bu.prefix = expr;
      // is prefix allowed
      if (munit.second.allowed_prefixes.size() > 0) {
        if (std::find(munit.second.allowed_prefixes.begin(), munit.second.allowed_prefixes.end(), bu.prefix) == munit.second.allowed_prefixes.end()) {
          std::stringstream ss;
          ss << "Given prefix is not allowed in unit: " + expr_orig << '\n';
          ss << "Allowed prefixes are:";
          for (auto& prefix : munit.second.allowed_prefixes) {
            ss << " " << prefix;
          }
          throw AtomParsingExcept(ss.str());
        }
      }
    }
    // fill UnitValue properties
    uv.magnitude = 1;
    uv.baseunits.append(bu);
  }

  UnitValue UnitAtom::from_string(const std::string& expr_orig) {
    std::string expr = expr_orig;
    struct UnitValue uv;
    std::smatch m;
#ifdef EXPONENT_FRACTIONS
    std::regex rx_unit("^(\\[?[a-zA-Z0_%#']+\\]?)([+-]?[0-9]*)(" + std::string(SYMBOL_FRACTION) + "([0-9]+)|)$");
    std::regex rx_quantity("^(\\<[a-zA-Z_]+\\>)([+-]?[0-9]*)(" + std::string(SYMBOL_FRACTION) + "([0-9]+)|)$");
    std::regex rx_sifactor("^(\\|[a-zA-Z_]+\\|)([+-]?[0-9]*)(" + std::string(SYMBOL_FRACTION) + "([0-9]+)|)$");
#else
    std::regex rx_unit("^(\\[?[a-zA-Z0_%#']+\\]?)([+-]?[0-9]*)$");
    std::regex rx_quantity("^(\\<[a-zA-Z_]+\\>)([+-]?[0-9]*)$");
    std::regex rx_sifactor("^(\\|[a-zA-Z_]+\\|)([+-]?[0-9]*)$");
#endif
    std::regex rx_number("^((\\+|-)?[0-9]+)(\\.(([0-9]+)?))?(\\(([0-9]+)\\))?((e|E)((\\+|-)?[0-9]+))?$");
    if (std::regex_match(expr, m, rx_number)) {
      _parse_number(expr, uv, m);
    } else if (std::regex_match(expr, m, rx_quantity)) {
      _parse_quantity(expr, uv, m);
    } else if (std::regex_match(expr, m, rx_sifactor)) {
      _parse_quantity(expr, uv, m);
    } else if (std::regex_match(expr, m, rx_unit)) {
      _parse_unit(expr, uv, m, expr_orig);
    } else {
      throw AtomParsingExcept("Invalid unit expression: " + expr_orig);
    }
    return uv;
  }

  std::string UnitAtom::to_string() {
    return value.to_string();
  }

  void UnitAtom::math_power(EXPONENT_TYPE& e) {
#ifdef DEBUG_UNIT_SOLVER
    std::clog << "UNIT:    pow(" << value.to_string() << "," << e.to_string() << ") = ";
#endif
    value.pow(e);
#ifdef DEBUG_UNIT_SOLVER
    std::clog << value.to_string() << std::endl;
#endif
  }

  void UnitAtom::math_multiply(UnitAtom* other) {
#ifdef DEBUG_UNIT_SOLVER
    std::clog << "UNIT:    " << value.to_string() << " * " << other->value.to_string() << " = ";
#endif
    value *= other->value;
#ifdef DEBUG_UNIT_SOLVER
    std::clog << value.to_string() << std::endl;
#endif
  }

  void UnitAtom::math_divide(UnitAtom* other) {
#ifdef DEBUG_UNIT_SOLVER
    std::clog << "UNIT:    " << value.to_string() << " / " << other->value.to_string() << " = ";
#endif
    value /= other->value;
#ifdef DEBUG_UNIT_SOLVER
    std::clog << value.to_string() << std::endl;
#endif
  }

} // namespace puq
