#include <snt/puq/value/measurement.h>

#include <snt/puq/converter.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/nostd/log10.h>
#include <snt/puq/nostd/pow.h>
#include <snt/puq/solver/unit_solver.h>

#include <iostream>
#include <memory>
#include <sstream>

namespace snt::puq {

  /*
   * Convert absolute and relative (in %) uncertainties
   */
  MagnitudeFloat Measurement::abs_to_rel(const MagnitudeFloat& v, const MagnitudeFloat& e) {
    return 100 * e / v;
  };

  MagnitudeFloat Measurement::rel_to_abs(const MagnitudeFloat& v, const MagnitudeFloat& e) {
    return v * e / 100;
  };

  // DEBUG: need to implement properly
  val::BaseValue::PointerType Measurement::abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
    return e->math_div(v.get())->math_mul(std::make_unique<val::ArrayValue<double>>(100).get());
  };

  val::BaseValue::PointerType Measurement::rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType e) {
    return v->math_mul(e.get())->math_div(std::make_unique<val::ArrayValue<double>>(100).get());
  };

  /*
   * Constructors
   */
  
  Measurement::Measurement(const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    magnitude = atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }

  Measurement::Measurement(const Magnitude& est, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    magnitude = est * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }
  
  Measurement::Measurement(const Magnitude& est, const Dimensions& dim) {
    magnitude = est * dim.numerical;
    for (int i = 0; i < Config::num_basedim; i++) {
      const std::string& symbol = SystemData::BaseUnitOrder[i];
      baseunits.append("", symbol, dim.physical[i]);
    }
  }

  Measurement::Measurement(const Measurement& msr, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    magnitude = msr.magnitude * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }

  Measurement::Measurement(const Measurement& msr, const Dimensions& dim) {
    magnitude = msr.magnitude * dim.numerical;
    for (int i = 0; i < Config::num_basedim; i++) {
      const std::string& symbol = SystemData::BaseUnitOrder[i];
      baseunits.append("", symbol, dim.physical[i]);
    }
  }

  Measurement::Measurement(const MagnitudeFloat& est, const Dimensions& dim) {
    magnitude = Magnitude(est) * dim.numerical;
    for (int i = 0; i < Config::num_basedim; i++) {
      const std::string& symbol = SystemData::BaseUnitOrder[i];
      baseunits.append("", symbol, dim.physical[i]);
    }
  }
  
  Measurement::Measurement(const MagnitudeFloat& est, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    magnitude = Magnitude(est) * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }
  Measurement::Measurement(const MagnitudeFloat& est, const MagnitudeFloat& unc, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    magnitude = Magnitude(est, unc) * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }
  
  Measurement::Measurement(val::BaseValue::PointerType est, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    magnitude = Magnitude(std::move(est)) * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }
  Measurement::Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    magnitude = Magnitude(std::move(est), std::move(unc)) * atom.value.magnitude;
    baseunits = atom.value.baseunits;
  }

  std::size_t Measurement::size() const {
    return magnitude.size();
  }

  val::Array::ShapeType Measurement::shape() const {
    return magnitude.shape();
  }

  std::string Measurement::to_string(const UnitFormat& format) const {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    if (format.display_magnitude()) {
      if (!magnitude.estimate->is_unity() || baseunits.size() == 0)
        ss << magnitude.to_string(format) << multiply;
      //if (magnitude.estimate != 1 || baseunits.size() == 0)
      //  ss << magnitude.to_string(format) << multiply;
    }
    if (format.display_units()) {
      if (baseunits.size() > 0)
        ss << baseunits.to_string(format) << multiply;
    }
    std::string s = ss.str();
    return s.substr(0, s.size() - multiply.size());
  }

  bool operator==(const Measurement& msr1, const Measurement& msr2) {
    try {
      Converter conv(msr2.baseunits, msr1.baseunits);
      if constexpr (puq::Config::use_units_logarithmic) {
	if (conv.utype == Utype::LOG) {
	  Magnitude mag1 = msr1.magnitude * conv.dimensions1.numerical;
	  Magnitude mag2 = conv.convert(msr2.magnitude) * conv.dimensions2.numerical;
	  if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	    mag1 = nostd::pow(10, mag1);
	  if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	    mag2 = nostd::pow(10, mag2);
	  return mag1 == mag2;
	}
      }
      return msr1.magnitude == conv.convert(msr2.magnitude);
    } catch (const snt::puq::ConvDimExcept& e) {
      return false;
    }
  }
  
  bool operator!=(const Measurement& msr1, const Measurement& msr2) {
    try {
      Converter conv(msr2.baseunits, msr1.baseunits);
      if constexpr (puq::Config::use_units_logarithmic) {
	if (conv.utype == Utype::LOG) {
	  Magnitude mag1 = msr1.magnitude * conv.dimensions1.numerical;
	  Magnitude mag2 = conv.convert(msr2.magnitude) * conv.dimensions2.numerical;
	  if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	    mag1 = nostd::pow(10, mag1);
	  if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	    mag2 = nostd::pow(10, mag2);
	  return mag1 != mag2;
	}
      }
      return msr1.magnitude != conv.convert(msr2.magnitude);
    } catch (const snt::puq::ConvDimExcept& e) {
      return true;
    }
  }
  
  Measurement operator+(const Measurement& msr1, const Measurement& msr2) {
    Converter conv(msr2.baseunits, msr1.baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
	Magnitude mag1 = msr1.magnitude * conv.dimensions1.numerical;
	Magnitude mag2 = conv.convert(msr2.magnitude) * conv.dimensions2.numerical;
	if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	  mag1 = nostd::pow(10, mag1);
	if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	  mag2 = nostd::pow(10, mag2);
	return Measurement(nostd::log10(mag1 + mag2) / conv.dimensions1.numerical, msr1.baseunits);
      }
    }
    return Measurement(msr1.magnitude + conv.convert(msr2.magnitude), msr1.baseunits);
  }

  Measurement operator+(const Measurement& msr) {
    return msr;
  }

  void Measurement::operator+=(const Measurement& msr) {
    Converter conv(msr.baseunits, baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
	Magnitude mag1 = magnitude * conv.dimensions1.numerical;
	Magnitude mag2 = conv.convert(msr.magnitude) * conv.dimensions2.numerical;
	if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	  mag1 = nostd::pow(10, mag1);
	if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	  mag2 = nostd::pow(10, mag2);
	magnitude = nostd::log10(mag1 + mag2) / conv.dimensions1.numerical;
	return;
      }
    }
    magnitude += conv.convert(msr.magnitude);
  }

  Measurement operator-(const Measurement& msr1, const Measurement& msr2) {
    Converter conv(msr2.baseunits, msr1.baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
	Magnitude mag1 = msr1.magnitude * conv.dimensions1.numerical;
	Magnitude mag2 = conv.convert(msr2.magnitude) * conv.dimensions2.numerical;
	if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	  mag1 = nostd::pow(10, mag1);
	if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	  mag2 = nostd::pow(10, mag2);
	return Measurement(nostd::log10(mag1 - mag2) / conv.dimensions1.numerical, msr1.baseunits);
      }
    }
    return Measurement(msr1.magnitude - conv.convert(msr2.magnitude), msr1.baseunits);
  }

  Measurement operator-(const Measurement& msr) {
    return Measurement(-msr.magnitude, msr.baseunits);
  }

  void Measurement::operator-=(const Measurement& msr) {
    Converter conv(msr.baseunits, baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
	Magnitude mag1 = magnitude * conv.dimensions1.numerical;
	Magnitude mag2 = conv.convert(msr.magnitude) * conv.dimensions2.numerical;
	if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
	  mag1 = nostd::pow(10, mag1);
	if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
	  mag2 = nostd::pow(10, mag2);
	magnitude = nostd::log10(mag1 - mag2) / conv.dimensions1.numerical;
	return;
      }
    }
    magnitude -= conv.convert(msr.magnitude);
  }

  Measurement operator*(const Measurement& msr1, const Measurement& msr2) {
    return Measurement(msr1.magnitude * msr2.magnitude, msr1.baseunits + msr2.baseunits);
  }

  void Measurement::operator*=(const Measurement& msr) {
    magnitude *= msr.magnitude;
    baseunits += msr.baseunits;
  }

  Measurement operator/(const Measurement& msr1, const Measurement& msr2) {
    return Measurement(msr1.magnitude / msr2.magnitude, msr1.baseunits - msr2.baseunits);
  }

  void Measurement::operator/=(const Measurement& msr) {
    magnitude /= msr.magnitude;
    baseunits -= msr.baseunits;
  }

  std::ostream& operator<<(std::ostream& os, const Measurement& msr) {
    os << msr.to_string();
    return os;
  }

  void Measurement::pow(const ExponentVariant& exp) {
    magnitude.pow(exp);
    //magnitude = std::pow(magnitude, (ExponentFloat)e);
    baseunits *= exp;
  }

  Measurement Measurement::convert(const std::string& str) const {
    Measurement msr = Measurement(str);
    return convert(msr);
  }

  Measurement Measurement::convert(const Measurement& msr) const {
    Converter conv(baseunits, msr.baseunits);
    Magnitude mag = conv.convert(magnitude, msr.magnitude);
    return Measurement(mag, msr.baseunits);
  }

  Measurement Measurement::convert(const BaseUnits& bun) const {
    Converter conv(baseunits, bun);
    return Measurement(conv.convert(magnitude, 1), bun);
  }

  Measurement Measurement::convert(const Format::Base& format) const {
    BaseUnits bun;
    Dimensions dim = baseunits.dimensions();
    for (int i = 0; i < Config::num_basedim; i++) {
      if (exponent_to_float(dim.physical[i]) == 0)
        continue;
      if constexpr (Config::use_system_eus) {
	if (i == 0 && format == Format::Base::FPS) {
	  bun.append({"", "ft", dim.physical[i]});
	  continue;
	} else if (i == 1 && format == Format::Base::FPS) {
	  bun.append({"", "lb", dim.physical[i]});
	  continue;
	}
      }
      if (i == 1 && format == Format::Base::MKS) {
        bun.append({"k", "g", dim.physical[i]});
      } else if (i == 0 && format == Format::Base::CGS) {
        bun.append({"c", "m", dim.physical[i]});
      } else {
        bun.append({"", SystemData::BaseUnitOrder[i], dim.physical[i]});
      }
    }
    return convert(bun);
  }

  Measurement Measurement::rebase_prefixes() {
    Magnitude mag = magnitude;
    std::map<std::string, BaseUnit> bumap;
    for (const auto& bun : baseunits) {
      if (bumap.find(bun.unit) == bumap.end()) {
        bumap.insert({bun.unit, {bun.prefix, bun.unit, bun.exponent}});
      } else {
        auto prefix1 = UnitPrefixList.find(bun.prefix);
        auto prefix2 = UnitPrefixList.find(bumap[bun.unit].prefix);
	ExponentFloat fexp = exponent_to_float(bun.exponent);
        if (prefix1 != UnitPrefixList.end())
          mag *= nostd::pow(prefix1->second.magnitude, fexp);
        if (prefix2 != UnitPrefixList.end())
          mag /= nostd::pow(prefix2->second.magnitude, fexp);
	bumap[bun.unit].exponent = std::visit([](auto const& a, auto const& b) -> ExponentVariant {
	  return a + b;
	}, bumap[bun.unit].exponent, bun.exponent);
      }
    }
    BaseUnits bus;
    for (const auto& bum : bumap) {
      bus.append(bum.second);
    }
    return Measurement(mag, bus);
  }

  Measurement Measurement::rebase_dimensions() {
    Magnitude mag = magnitude;
    std::map<std::string, BaseUnit> bumap;
    for (const auto& bun : baseunits) {
      Dimensions dim = BaseUnits(bun.prefix + bun.unit).dimensions();
      if ((dim.utype & Utype::LOG) == Utype::LOG)
        throw MeasurementExcept("Dimensions of logarithmic units cannot be rebased: " + baseunits.to_string());
      if ((dim.utype & Utype::TMP) == Utype::TMP)
        throw MeasurementExcept("Dimensions of temperature units cannot be rebased: " + baseunits.to_string());
      std::string key = dim.to_string({Format::Display::UNITS});
      if (bumap.find(key) == bumap.end()) {
        bumap.insert({key, {bun.prefix, bun.unit, bun.exponent}});
      } else {
        Dimensions dim0 = BaseUnits(bumap[key].prefix + bumap[key].unit).dimensions();
        mag *= nostd::pow(dim.numerical / dim0.numerical, exponent_to_float(bun.exponent));
	bumap[key].exponent = std::visit([](auto const& a, auto const& b) -> ExponentVariant {
	  return a + b;
	}, bumap[key].exponent, bun.exponent);
      }
    }
    BaseUnits bus;
    for (const auto& bum : bumap) {
      bus.append(bum.second);
    }
    return Measurement(mag, bus);
  }

} // namespace snt::puq
