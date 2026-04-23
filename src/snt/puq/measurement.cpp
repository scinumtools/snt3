#include <iostream>
#include <memory>
#include <snt/puq/converter.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/math/log10.h>
#include <snt/puq/math/pow.h>
#include <snt/puq/measurement.h>
#include <snt/puq/solver/unit_solver.h>
#include <sstream>

namespace snt::puq {

  /*
   * Convert absolute and relative (in %) uncertainties
   */
  double Measurement::abs_to_rel(const double v, const double e) {
    return 100 * e / v;
  };

  double Measurement::rel_to_abs(const double v, const double e) {
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
    result = atom.value.result;
    baseunits = atom.value.baseunits;
  }

  Measurement::Measurement(const Result& est, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    result = est * atom.value.result;
    baseunits = atom.value.baseunits;
  }

  Measurement::Measurement(const Result& est, const Dimensions& dim) {
    result = est * dim.numerical;
    for (int i = 0; i < Config::num_basedim; i++) {
      const std::string& symbol = SystemData::BaseUnitOrder[i];
      baseunits.append("", symbol, dim.physical[i]);
    }
  }

  Measurement::Measurement(const Measurement& msr, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    result = msr.result * atom.value.result;
    baseunits = atom.value.baseunits;
  }

  Measurement::Measurement(const Measurement& msr, const Dimensions& dim) {
    result = msr.result * dim.numerical;
    for (int i = 0; i < Config::num_basedim; i++) {
      const std::string& symbol = SystemData::BaseUnitOrder[i];
      baseunits.append("", symbol, dim.physical[i]);
    }
  }

  Measurement::Measurement(const double est, const Dimensions& dim) {
    result = Result(est) * dim.numerical;
    for (int i = 0; i < Config::num_basedim; i++) {
      const std::string& symbol = SystemData::BaseUnitOrder[i];
      baseunits.append("", symbol, dim.physical[i]);
    }
  }

  Measurement::Measurement(const double est, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    result = Result(est) * atom.value.result;
    baseunits = atom.value.baseunits;
  }
  Measurement::Measurement(const double est, const double unc, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    result = Result(est, unc) * atom.value.result;
    baseunits = atom.value.baseunits;
  }

  Measurement::Measurement(val::BaseValue::PointerType est, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    result = Result(std::move(est)) * atom.value.result;
    baseunits = atom.value.baseunits;
  }
  Measurement::Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc, const std::string& str) {
    UnitSolver solver;
    UnitAtom atom = solver.solve(str);
    result = Result(std::move(est), std::move(unc)) * atom.value.result;
    baseunits = atom.value.baseunits;
  }

  std::size_t Measurement::size() const {
    return result.size();
  }

  val::Array::ShapeType Measurement::shape() const {
    return result.shape();
  }

  std::string Measurement::to_string(const UnitFormat& format) const {
    std::string multiply = format.multiply_symbol();
    std::stringstream ss;
    if (format.display_result()) {
      if (!result.estimate->is_unity() || baseunits.size() == 0)
        ss << result.to_string(format) << multiply;
      // if (result.estimate != 1 || baseunits.size() == 0)
      //   ss << result.to_string(format) << multiply;
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
          Result res1 = msr1.result * conv.dimensions1.numerical;
          Result res2 = conv.convert(msr2.result) * conv.dimensions2.numerical;
          if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
            res1 = math::pow(10, res1);
          if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
            res2 = math::pow(10, res2);
          return res1 == res2;
        }
      }
      return msr1.result == conv.convert(msr2.result);
    } catch (const snt::puq::ConvDimExcept& e) {
      return false;
    }
  }

  bool operator!=(const Measurement& msr1, const Measurement& msr2) {
    try {
      Converter conv(msr2.baseunits, msr1.baseunits);
      if constexpr (puq::Config::use_units_logarithmic) {
        if (conv.utype == Utype::LOG) {
          Result res1 = msr1.result * conv.dimensions1.numerical;
          Result res2 = conv.convert(msr2.result) * conv.dimensions2.numerical;
          if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
            res1 = math::pow(10, res1);
          if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
            res2 = math::pow(10, res2);
          return res1 != res2;
        }
      }
      return msr1.result != conv.convert(msr2.result);
    } catch (const snt::puq::ConvDimExcept& e) {
      return true;
    }
  }

  Measurement operator+(const Measurement& msr1, const Measurement& msr2) {
    Converter conv(msr2.baseunits, msr1.baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
        Result res1 = msr1.result * conv.dimensions1.numerical;
        Result res2 = conv.convert(msr2.result) * conv.dimensions2.numerical;
        if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
          res1 = math::pow(10, res1);
        if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
          res2 = math::pow(10, res2);
        return Measurement(math::log10(res1 + res2) / conv.dimensions1.numerical, msr1.baseunits);
      }
    }
    return Measurement(msr1.result + conv.convert(msr2.result), msr1.baseunits);
  }

  Measurement operator+(const Measurement& msr) {
    return msr;
  }

  void Measurement::operator+=(const Measurement& msr) {
    Converter conv(msr.baseunits, baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
        Result res1 = result * conv.dimensions1.numerical;
        Result res2 = conv.convert(msr.result) * conv.dimensions2.numerical;
        if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
          res1 = math::pow(10, res1);
        if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
          res2 = math::pow(10, res2);
        result = math::log10(res1 + res2) / conv.dimensions1.numerical;
        return;
      }
    }
    result += conv.convert(msr.result);
  }

  Measurement operator-(const Measurement& msr1, const Measurement& msr2) {
    Converter conv(msr2.baseunits, msr1.baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
        Result res1 = msr1.result * conv.dimensions1.numerical;
        Result res2 = conv.convert(msr2.result) * conv.dimensions2.numerical;
        if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
          res1 = math::pow(10, res1);
        if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
          res2 = math::pow(10, res2);
        return Measurement(math::log10(res1 - res2) / conv.dimensions1.numerical, msr1.baseunits);
      }
    }
    return Measurement(msr1.result - conv.convert(msr2.result), msr1.baseunits);
  }

  Measurement operator-(const Measurement& msr) {
    return Measurement(-msr.result, msr.baseunits);
  }

  void Measurement::operator-=(const Measurement& msr) {
    Converter conv(msr.baseunits, baseunits);
    if constexpr (puq::Config::use_units_logarithmic) {
      if (conv.utype == Utype::LOG) {
        Result res1 = result * conv.dimensions1.numerical;
        Result res2 = conv.convert(msr.result) * conv.dimensions2.numerical;
        if ((conv.dimensions1.utype & Utype::LOG) == Utype::LOG)
          res1 = math::pow(10, res1);
        if ((conv.dimensions2.utype & Utype::LOG) == Utype::LOG)
          res2 = math::pow(10, res2);
        result = math::log10(res1 - res2) / conv.dimensions1.numerical;
        return;
      }
    }
    result -= conv.convert(msr.result);
  }

  Measurement operator*(const Measurement& msr1, const Measurement& msr2) {
    return Measurement(msr1.result * msr2.result, msr1.baseunits * msr2.baseunits);
  }

  void Measurement::operator*=(const Measurement& msr) {
    result *= msr.result;
    baseunits *= msr.baseunits;
  }

  Measurement operator/(const Measurement& msr1, const Measurement& msr2) {
    return Measurement(msr1.result / msr2.result, msr1.baseunits / msr2.baseunits);
  }

  void Measurement::operator/=(const Measurement& msr) {
    result /= msr.result;
    baseunits /= msr.baseunits;
  }

  std::ostream& operator<<(std::ostream& os, const Measurement& msr) {
    os << msr.to_string();
    return os;
  }

  // void Measurement::pow(const ExponentVariant& exp) {
  //   result.pow(exp);
  //   // TODO: should be pow() instead of *=
  //   // result = std::pow(result, (double)e);
  //   baseunits *= exp;
  // }

  Measurement Measurement::convert(const std::string& str) const {
    Measurement msr = Measurement(str);
    return convert(msr);
  }

  Measurement Measurement::convert(const Measurement& msr) const {
    Converter conv(baseunits, msr.baseunits);
    Result res = conv.convert(result, msr.result);
    return Measurement(res, msr.baseunits);
  }

  Measurement Measurement::convert(const BaseUnits& bun) const {
    Converter conv(baseunits, bun);
    return Measurement(conv.convert(result, 1), bun);
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
    Result res = result;
    std::map<std::string, BaseUnit> bumap;
    for (const auto& bun : baseunits) {
      if (bumap.find(bun.unit) == bumap.end()) {
        bumap.insert({bun.unit, {bun.prefix, bun.unit, bun.exponent}});
      } else {
        auto prefix1 = UnitPrefixList.find(bun.prefix);
        auto prefix2 = UnitPrefixList.find(bumap[bun.unit].prefix);
        double fexp = exponent_to_float(bun.exponent);
        if (prefix1 != UnitPrefixList.end())
          res *= math::pow(prefix1->second.result, fexp);
        if (prefix2 != UnitPrefixList.end())
          res /= math::pow(prefix2->second.result, fexp);
        bumap[bun.unit].exponent = std::visit([](auto const& a, auto const& b) -> ExponentVariant {
          return a + b;
        },
                                              bumap[bun.unit].exponent, bun.exponent);
      }
    }
    BaseUnits bus;
    for (const auto& bum : bumap) {
      bus.append(bum.second);
    }
    return Measurement(res, bus);
  }

  Measurement Measurement::rebase_dimensions() {
    Result res = result;
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
        res *= math::pow(dim.numerical / dim0.numerical, exponent_to_float(bun.exponent));
        bumap[key].exponent = std::visit([](auto const& a, auto const& b) -> ExponentVariant {
          return a + b;
        },
                                         bumap[key].exponent, bun.exponent);
      }
    }
    BaseUnits bus;
    for (const auto& bum : bumap) {
      bus.append(bum.second);
    }
    return Measurement(res, bus);
  }

} // namespace snt::puq
