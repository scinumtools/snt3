#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../nostd/nostd.h"
#include "../settings.h"
#include "../solver/solver.h"
#include "value.h"

namespace puq {

  BaseUnits::BaseUnits(const std::string& s) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(s);
    baseunits = atom.value.baseunits.baseunits;
  }

  std::string BaseUnit::to_string(const UnitFormat& format) {
    std::stringstream ss;
    ss << prefix << unit;
    if (exponent != 1)
      ss << nostd::to_string(exponent, format);
    return ss.str();
  }

  void BaseUnits::append(BaseUnit bu) {
    bool exists = false;
    for (auto it = baseunits.begin(); it != baseunits.end(); ++it) {
      if (it->prefix == bu.prefix && it->unit == bu.unit) {
        exists = true;
        it->exponent += bu.exponent;
        // removing zero exponents
        if (it->exponent == 0) {
          baseunits.erase(it);
        }
        break;
      }
    }
    if (!exists && bu.exponent != 0) {
      baseunits.push_back(bu);
    }
  }

  void BaseUnits::append(std::string p, std::string u, EXPONENT_TYPE e) {
    append(BaseUnit(p, u, e));
  }

#ifdef EXPONENT_FRACTIONS
  void BaseUnits::append(std::string p, std::string u, EXPONENT_INT_PRECISION n, EXPONENT_INT_PRECISION d) {
    append(BaseUnit(p, u, n, d));
  }
#endif

  std::string BaseUnits::to_string(const UnitFormat& format) const {
    std::stringstream ss;
    std::string multiply = format.multiply_symbol();
    for (auto unit : baseunits) {
      ss << unit.to_string(format) << multiply;
    }
    std::string s = ss.str();
    return s.substr(0, s.size() - multiply.size());
  }

  const BaseUnit& BaseUnits::operator[](int index) const {
    return baseunits[index];
  }

  BaseUnits operator+(const BaseUnits& bu1, const BaseUnits& bu2) {
    BaseUnits nbu(bu1.baseunits);
    for (auto const& unit : bu2) {
      nbu.append(unit);
    }
    return nbu;
  }
  void BaseUnits::operator+=(const BaseUnits& bu) {
    for (auto unit : bu) {
      append(unit);
    }
  }

  BaseUnits operator-(const BaseUnits& bu1, const BaseUnits& bu2) {
    BaseUnits nbu(bu1.baseunits);
    for (auto unit : bu2) {
      unit.exponent = -unit.exponent;
      nbu.append(unit);
    }
    return nbu;
  }
  void BaseUnits::operator-=(const BaseUnits& bu) {
    for (auto unit : bu) {
      unit.exponent = -unit.exponent;
      append(unit);
    }
  }

  void BaseUnits::operator*=(const EXPONENT_TYPE& e) {
    for (auto& unit : baseunits) {
      unit.exponent *= e;
    }
  }

  std::ostream& operator<<(std::ostream& os, const BaseUnits& bu) {
    os << bu.to_string();
    return os;
  }

  BaseUnitsList::const_iterator BaseUnits::begin() const {
    return baseunits.begin();
  }

  BaseUnitsList::const_iterator BaseUnits::end() const {
    return baseunits.end();
  }

  std::size_t BaseUnits::size() const {
    return baseunits.size();
  }

  Dimensions BaseUnits::dimensions() const {
    Dimensions dim;
    for (auto& bu : baseunits) {
      // find prefix
      auto prefix = UnitPrefixList.find(bu.prefix);
      if (prefix != UnitPrefixList.end()) {
        dim.numerical *= nostd::pow(prefix->second.magnitude, (EXPONENT_TYPE)bu.exponent);
      }
      // quantity
      if (bu.unit.rfind(SYMBOL_QUANTITY_START, 0) == 0 || bu.unit.rfind(SYMBOL_SIFACTOR_START, 0) == 0) {
        auto dmap = UnitSystem::Data->DimensionMap.find(bu.unit);
        if (dmap != UnitSystem::Data->DimensionMap.end()) {
          dim.utype = dim.utype | Utype::LIN;
          dim.symbols.push_back(dmap->first);
#ifdef MAGNITUDE_ERRORS
          MAGNITUDE_TYPE magnitude(dmap->second.magnitude, dmap->second.error);
#else
          MAGNITUDE_TYPE magnitude(dmap->second.magnitude);
#endif
          dim.numerical *= nostd::pow(magnitude, (EXPONENT_TYPE)bu.exponent);
          for (int i = 0; i < NUM_BASEDIM; i++) {
            dim.physical[i] += dmap->second.dimensions[i] * bu.exponent;
          }
          continue;
        } else {
          throw UnitValueExcept("Undefined quantity symbol: " + bu.unit);
        }
      }
      // dimensions
      auto unit = UnitSystem::Data->UnitList.find(bu.unit);
      if (unit != UnitSystem::Data->UnitList.end()) {
        if (unit->first == bu.unit) {
          if ((unit->second.utype & Utype::LIN) == Utype::LIN) // standard linear conversion
            dim.utype = dim.utype | Utype::LIN;
          if (unit->second.utype == Utype::TMP) // unit requires conversion of temperatures
            dim.utype = dim.utype | unit->second.utype;
          if (unit->second.utype == Utype::LOG) // unit requires logarithmic conversions
            dim.utype = dim.utype | unit->second.utype;
          dim.symbols.push_back(unit->first);
          auto dmap = UnitSystem::Data->DimensionMap.find(unit->first);
          if (dmap != UnitSystem::Data->DimensionMap.end()) {
#ifdef MAGNITUDE_ERRORS
            MAGNITUDE_TYPE magnitude(dmap->second.magnitude, dmap->second.error);
#else
            MAGNITUDE_TYPE magnitude(dmap->second.magnitude);
#endif
            dim.numerical *= nostd::pow(magnitude, (EXPONENT_TYPE)bu.exponent);
            for (int i = 0; i < NUM_BASEDIM; i++) {
              dim.physical[i] += dmap->second.dimensions[i] * bu.exponent;
            }
          } else {
            throw UnitValueExcept("Undefined unit symbol: " + bu.unit);
          }
        }
      } else {
        throw UnitValueExcept("Undefined unit symbol: " + bu.unit);
      }
    }
    if ((dim.utype & Utype::NUL) == Utype::NUL) // add liear utype for dimensionless values
      dim.utype = dim.utype | Utype::LIN;
    // std::cout << "A " << std::bitset<8>((int)dim.utype) << std::endl;
    return dim;
  }

} // namespace puq
