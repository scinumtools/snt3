#include "unit_value.h"

#include "../converter.h"
#include "../exceptions.h"
#include "../nostd/log10.h"
#include "../nostd/pow.h"
#include "../solver/unit_solver.h"

#include <iostream>
#include <memory>
#include <sstream>

namespace snt::puq {

  UnitValue::UnitValue(const std::string& s) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(s);
    magnitude = atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }

  UnitValue::UnitValue(const MAGNITUDE_TYPE& m, const std::string& s) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(s);
    magnitude = m * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }

  UnitValue::UnitValue(const MAGNITUDE_TYPE& m, const Dimensions& dim) {
    magnitude = m * dim.numerical;
    for (int i = 0; i < Config::num_basedim; i++) {
      const std::string& symbol = SystemData::BaseUnitOrder[i];
      baseunits.append("", symbol, dim.physical[i]);
    }
  }

  UnitValue::UnitValue(const MagnitudeFloat& m, const std::string& s) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(s);
    magnitude = Magnitude(m) * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }
  UnitValue::UnitValue(const MagnitudeFloat& m, const MagnitudeFloat& e, const std::string& s) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(s);
    magnitude = Magnitude(m, e) * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }
  
  UnitValue::UnitValue(val::BaseValue::PointerType m, const std::string& s) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(s);
    magnitude = Magnitude(std::move(m)) * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }
  UnitValue::UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const std::string& s) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(s);
    magnitude = Magnitude(std::move(m), std::move(e)) * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }

  std::size_t UnitValue::size() const {
    return magnitude.size();
  }

  val::Array::ShapeType UnitValue::shape() const {
    return magnitude.shape();
  }

  std::string UnitValue::to_string(const UnitFormat& format) const {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    if (format.display_magnitude()) {
#if defined(MAGNITUDE_ERRORS)
#if defined(MAGNITUDE_VALUES)
      if (!magnitude.value->is_unity() || baseunits.size() == 0)
        ss << magnitude.to_string(format) << multiply;
#else
      if (magnitude.value != 1 || baseunits.size() == 0)
        ss << magnitude.to_string(format) << multiply;
#endif
#elif defined(MAGNITUDE_VALUES)
      if (!magnitude->is_unity() || baseunits.size() == 0)
        ss << magnitude->to_string(format) << multiply;
#else
      if (magnitude != 1 || baseunits.size() == 0) {
        ss << std::setprecision(format.precision);
        ss << magnitude << std::scientific << multiply;
      }
#endif
    }
    if (format.display_units()) {
      if (baseunits.size() > 0)
        ss << baseunits.to_string(format) << multiply;
    }
    std::string s = ss.str();
    return s.substr(0, s.size() - multiply.size());
  }

  bool operator==(const UnitValue& v1, const UnitValue& v2) {
    try {
      Converter conv(v2.baseunits, v1.baseunits);
      if constexpr (puq::Config::use_units_logarithmic) {
	if (conv.utype == Utype::LOG) {
	  MAGNITUDE_TYPE m1 = v1.magnitude * conv.dimensions1.numerical;
	  MAGNITUDE_TYPE m2 = conv.convert(v2.magnitude) * conv.dimensions2.numerical;
	  if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	    m1 = nostd::pow(10, m1);
	  if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	    m2 = nostd::pow(10, m2);
	  return m1 == m2;
	}
      }
      return v1.magnitude == conv.convert(v2.magnitude);
    } catch (const snt::puq::ConvDimExcept& e) {
      return false;
    }
  }
  
  bool operator!=(const UnitValue& v1, const UnitValue& v2) {
    try {
      Converter conv(v2.baseunits, v1.baseunits);
      if constexpr (puq::Config::use_units_logarithmic) {
	if (conv.utype == Utype::LOG) {
	  MAGNITUDE_TYPE m1 = v1.magnitude * conv.dimensions1.numerical;
	  MAGNITUDE_TYPE m2 = conv.convert(v2.magnitude) * conv.dimensions2.numerical;
	  if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	    m1 = nostd::pow(10, m1);
	  if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	    m2 = nostd::pow(10, m2);
	  return m1 != m2;
	}
      }
      return v1.magnitude != conv.convert(v2.magnitude);
    } catch (const snt::puq::ConvDimExcept& e) {
      return true;
    }
  }
  
  UnitValue operator+(const UnitValue& v1, const UnitValue& v2) {
    Converter conv(v2.baseunits, v1.baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
	MAGNITUDE_TYPE m1 = v1.magnitude * conv.dimensions1.numerical;
	MAGNITUDE_TYPE m2 = conv.convert(v2.magnitude) * conv.dimensions2.numerical;
	if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	  m1 = nostd::pow(10, m1);
	if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	  m2 = nostd::pow(10, m2);
	return UnitValue(nostd::log10(m1 + m2) / conv.dimensions1.numerical, v1.baseunits);
      }
    }
    return UnitValue(v1.magnitude + conv.convert(v2.magnitude), v1.baseunits);
  }

  UnitValue operator+(const UnitValue& v) {
    return v;
  }

  void UnitValue::operator+=(const UnitValue& v) {
    Converter conv(v.baseunits, baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
	MAGNITUDE_TYPE m1 = magnitude * conv.dimensions1.numerical;
	MAGNITUDE_TYPE m2 = conv.convert(v.magnitude) * conv.dimensions2.numerical;
	if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	  m1 = nostd::pow(10, m1);
	if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	  m2 = nostd::pow(10, m2);
	magnitude = nostd::log10(m1 + m2) / conv.dimensions1.numerical;
	return;
      }
    }
    magnitude += conv.convert(v.magnitude);
  }

  UnitValue operator-(const UnitValue& v1, const UnitValue& v2) {
    Converter conv(v2.baseunits, v1.baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
	MAGNITUDE_TYPE m1 = v1.magnitude * conv.dimensions1.numerical;
	MAGNITUDE_TYPE m2 = conv.convert(v2.magnitude) * conv.dimensions2.numerical;
	if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	  m1 = nostd::pow(10, m1);
	if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	  m2 = nostd::pow(10, m2);
	return UnitValue(nostd::log10(m1 - m2) / conv.dimensions1.numerical, v1.baseunits);
      }
    }
    return UnitValue(v1.magnitude - conv.convert(v2.magnitude), v1.baseunits);
  }

  UnitValue operator-(const UnitValue& v) {
    return UnitValue(-v.magnitude, v.baseunits);
  }

  void UnitValue::operator-=(const UnitValue& v) {
    Converter conv(v.baseunits, baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
	MAGNITUDE_TYPE m1 = magnitude * conv.dimensions1.numerical;
	MAGNITUDE_TYPE m2 = conv.convert(v.magnitude) * conv.dimensions2.numerical;
	if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	  m1 = nostd::pow(10, m1);
	if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	  m2 = nostd::pow(10, m2);
	magnitude = nostd::log10(m1 - m2) / conv.dimensions1.numerical;
	return;
      }
    }
    magnitude -= conv.convert(v.magnitude);
  }

  UnitValue operator*(const UnitValue& v1, const UnitValue& v2) {
    return UnitValue(v1.magnitude * v2.magnitude, v1.baseunits + v2.baseunits);
  }

  void UnitValue::operator*=(const UnitValue& v) {
    magnitude *= v.magnitude;
    baseunits += v.baseunits;
  }

  UnitValue operator/(const UnitValue& v1, const UnitValue& v2) {
    return UnitValue(v1.magnitude / v2.magnitude, v1.baseunits - v2.baseunits);
  }

  void UnitValue::operator/=(const UnitValue& v) {
    magnitude /= v.magnitude;
    baseunits -= v.baseunits;
  }

  std::ostream& operator<<(std::ostream& os, const UnitValue& v) {
    os << v.to_string();
    return os;
  }

  void UnitValue::pow(const EXPONENT_TYPE& e) {
#if defined(MAGNITUDE_ERRORS) || defined(MAGNITUDE_VALUES)
    magnitude.pow(e);
#else
    magnitude = std::pow(magnitude, (ExponentFloat)e);
#endif
    baseunits *= e;
  }

  UnitValue UnitValue::convert(const std::string& s) const {
    UnitValue uv = UnitValue(s);
    return convert(uv);
  }

  UnitValue UnitValue::convert(const UnitValue& v) const {
    Converter conv(baseunits, v.baseunits);
    MAGNITUDE_TYPE m = conv.convert(magnitude, v.magnitude);
    return UnitValue(m, v.baseunits);
  }

  UnitValue UnitValue::convert(const BaseUnits& bu) const {
    Converter conv(baseunits, bu);
    return UnitValue(conv.convert(magnitude, 1), bu);
  }

  UnitValue UnitValue::convert(const Format::Base& format) const {
    BaseUnits bu;
    Dimensions dim = baseunits.dimensions();
    for (int i = 0; i < Config::num_basedim; i++) {
      if (dim.physical[i] == 0)
        continue;
      if constexpr (Config::use_system_eus) {
	if (i == 0 && format == Format::Base::FPS) {
	  bu.append({"", "ft", dim.physical[i]});
	  continue;
	} else if (i == 1 && format == Format::Base::FPS) {
	  bu.append({"", "lb", dim.physical[i]});
	  continue;
	}
      }
      if (i == 1 && format == Format::Base::MKS) {
        bu.append({"k", "g", dim.physical[i]});
      } else if (i == 0 && format == Format::Base::CGS) {
        bu.append({"c", "m", dim.physical[i]});
      } else {
        bu.append({"", SystemData::BaseUnitOrder[i], dim.physical[i]});
      }
    }
    return convert(bu);
  }

  UnitValue UnitValue::rebase_prefixes() {
    MAGNITUDE_TYPE mag = magnitude;
    std::map<std::string, BaseUnit> bumap;
    for (const auto& bu : baseunits) {
      if (bumap.find(bu.unit) == bumap.end()) {
        bumap.insert({bu.unit, {bu.prefix, bu.unit, bu.exponent}});
      } else {
        auto prefix1 = UnitPrefixList.find(bu.prefix);
        auto prefix2 = UnitPrefixList.find(bumap[bu.unit].prefix);
        if (prefix1 != UnitPrefixList.end())
          mag *= nostd::pow(prefix1->second.magnitude, bu.exponent);
        if (prefix2 != UnitPrefixList.end())
          mag /= nostd::pow(prefix2->second.magnitude, bu.exponent);
        bumap[bu.unit].exponent += bu.exponent;
      }
    }
    BaseUnits bus;
    for (const auto& bum : bumap) {
      bus.append(bum.second);
    }
    return UnitValue(mag, bus);
  }

  UnitValue UnitValue::rebase_dimensions() {
    MAGNITUDE_TYPE mag = magnitude;
    std::map<std::string, BaseUnit> bumap;
    for (const auto& bu : baseunits) {
      Dimensions dim = BaseUnits(bu.prefix + bu.unit).dimensions();
      if ((dim.utype & Utype::LOG) == Utype::LOG)
        throw UnitValueExcept("Dimensions of logarithmic units cannot be rebased: " + baseunits.to_string());
      if ((dim.utype & Utype::TMP) == Utype::TMP)
        throw UnitValueExcept("Dimensions of temperature units cannot be rebased: " + baseunits.to_string());
      std::string key = dim.to_string({Format::Display::UNITS});
      if (bumap.find(key) == bumap.end()) {
        bumap.insert({key, {bu.prefix, bu.unit, bu.exponent}});
      } else {
        Dimensions dim0 = BaseUnits(bumap[key].prefix + bumap[key].unit).dimensions();
        mag *= nostd::pow(dim.numerical / dim0.numerical, bu.exponent);
        bumap[key].exponent += bu.exponent;
      }
    }
    BaseUnits bus;
    for (const auto& bum : bumap) {
      bus.append(bum.second);
    }
    return UnitValue(mag, bus);
  }

} // namespace snt::puq
