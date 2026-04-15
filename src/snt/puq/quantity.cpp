#include <iostream>
#include <snt/puq/converter.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/quantity.h>
#include <snt/puq/unit_format.h>

namespace snt::puq {

  void Quantity::preprocess(std::string& expression, SystemType& system) const {
    if constexpr (Config::preprocess_system) {
      for (auto sys : SystemMap) {
        if (UnitFormat::preprocess_system(expression, sys.second->SystemAbbrev)) {
          if (system == SystemType::NONE)
            system = sys.first;
          else if (system != sys.first) {
            auto it = SystemMap.find(system);
            throw UnitSystemExcept("Selected unit systems are ambiguous: " + it->second->SystemAbbrev + " " + sys.second->SystemAbbrev);
          }
          break;
        }
      }
      if (system == SystemType::NONE)
        system = UnitSystem::System;
    }
    if constexpr (Config::preprocess_symbols) {
      UnitFormat::preprocess_symbols(expression);
    }
  }

  Quantity::Quantity(std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    measurement = Measurement(s);
  }

  Quantity::Quantity(const Measurement& v, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = v;
  }

  Quantity::Quantity(const Result& m, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(m);
  }

  Quantity::Quantity(const Result& m, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(m, bu);
  }

  Quantity::Quantity(const Result& m, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    measurement = Measurement(m, s);
  }

  Quantity::Quantity(const double m, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(m);
  }

  Quantity::Quantity(const double m, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(m, bu);
  }

  Quantity::Quantity(const double m, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    Result mag(m);
    measurement = Measurement(mag, s);
  }

  Quantity::Quantity(const double m, const double e, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(m, e);
  }

  Quantity::Quantity(const double m, const double e, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(m, e, bu);
  }

  Quantity::Quantity(const double m, const double e, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    Result mag(m, e);
    measurement = Measurement(mag, s);
  }

  Quantity::Quantity(val::BaseValue::PointerType m, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(std::move(m));
  }

  Quantity::Quantity(val::BaseValue::PointerType m, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(std::move(m), bu);
  }

  Quantity::Quantity(val::BaseValue::PointerType m, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    Result mag(std::move(m));
    measurement = Measurement(mag, s);
  }

  Quantity::Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(std::move(m), std::move(e));
  }

  Quantity::Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    measurement = Measurement(std::move(m), std::move(e), bu);
  }

  Quantity::Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    Result mag(std::move(m), std::move(e));
    measurement = Measurement(mag, s);
  }

  std::string Quantity::unit_system() const {
    auto it = SystemMap.find(stype);
    if (it == SystemMap.end()) {
      it = SystemMap.find(UnitSystem::System);
    }
    return it->second->SystemAbbrev;
  }

  std::size_t Quantity::size() const {
    return measurement.size();
  }

  val::Array::ShapeType Quantity::shape() const {
    return measurement.shape();
  }

  Quantity::PointerType Quantity::clone() const {
    if (measurement.result.uncertainty)
      return std::make_unique<Quantity>(measurement.result.estimate->clone(), measurement.result.uncertainty->clone(), measurement.baseunits.baseunits, stype);
    else
      return std::make_unique<Quantity>(measurement.result.estimate->clone(), measurement.baseunits.baseunits, stype);
  }

  // strings and streams
  std::string Quantity::to_string(const UnitFormat& format) const {
    UnitSystem us(stype);
    std::string qstr = measurement.to_string(format);
    return format.format_system(qstr, unit_system());
  }

  std::ostream& operator<<(std::ostream& os, const Quantity& q) {
    os << q.to_string();
    return os;
  }

  // overloading quantity/quantity binary operations
  Quantity operator+(const Quantity& q1, const Quantity& q2) {
    UnitSystem us(q1.stype);
    if (q1.stype == q2.stype) {
      return Quantity(q1.measurement + q2.measurement);
    } else {
      Quantity q3 = q2.convert(q1.measurement.baseunits, q1.stype);
      return Quantity(q1.measurement + q3.measurement);
    }
  }
  Quantity operator-(const Quantity& q1, const Quantity& q2) {
    UnitSystem us(q1.stype);
    if (q1.stype == q2.stype) {
      return Quantity(q1.measurement - q2.measurement);
    } else {
      Quantity q3 = q2.convert(q1.measurement.baseunits, q1.stype);
      return Quantity(q1.measurement - q3.measurement);
    }
  }
  Quantity operator*(const Quantity& q1, const Quantity& q2) {
    if (q1.stype != q2.stype)
      throw UnitSystemExcept(q1.stype, q2.stype);
    UnitSystem us(q1.stype);
    return Quantity(q1.measurement * q2.measurement);
  }
  Quantity operator/(const Quantity& q1, const Quantity& q2) {
    if (q1.stype != q2.stype)
      throw UnitSystemExcept(q1.stype, q2.stype);
    UnitSystem us(q1.stype);
    return Quantity(q1.measurement / q2.measurement);
  }
  bool operator==(const Quantity& q1, const Quantity& q2) {
    if (q1.stype != q2.stype)
      throw UnitSystemExcept(q1.stype, q2.stype);
    UnitSystem us(q1.stype);
    return q1.measurement == q2.measurement;
  }
  bool operator!=(const Quantity& q1, const Quantity& q2) {
    if (q1.stype != q2.stype)
      throw UnitSystemExcept(q1.stype, q2.stype);
    UnitSystem us(q1.stype);
    return q1.measurement != q2.measurement;
  }

  // overloading scalar/quantity
  Quantity operator+(const double m, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(Measurement(m) + q.measurement);
  }
  Quantity operator-(const double m, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(Measurement(m) - q.measurement);
  }
  Quantity operator*(const double m, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(Measurement(m) * q.measurement);
  }
  Quantity operator/(const double m, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(Measurement(m) / q.measurement);
  }

  // overloading quantity/scalar
  Quantity operator+(const Quantity& q, const double m) {
    UnitSystem us(q.stype);
    return Quantity(q.measurement + Measurement(m));
  }
  Quantity operator-(const Quantity& q, const double m) {
    UnitSystem us(q.stype);
    return Quantity(q.measurement - Measurement(m));
  }
  Quantity operator*(const Quantity& q, const double m) {
    UnitSystem us(q.stype);
    return Quantity(q.measurement * Measurement(m));
  }
  Quantity operator/(const Quantity& q, const double m) {
    UnitSystem us(q.stype);
    return Quantity(q.measurement / Measurement(m));
  }

  // overloading array/quantity
  Quantity operator+(val::BaseValue::PointerType a, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(Measurement(std::move(a)) + q.measurement);
  }
  Quantity operator-(val::BaseValue::PointerType a, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(Measurement(std::move(a)) - q.measurement);
  }
  Quantity operator*(val::BaseValue::PointerType a, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(Measurement(std::move(a)) * q.measurement);
  }
  Quantity operator/(val::BaseValue::PointerType a, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(Measurement(std::move(a)) / q.measurement);
  }

  // overloading quantity/array
  Quantity operator+(const Quantity& q, val::BaseValue::PointerType a) {
    UnitSystem us(q.stype);
    return Quantity(q.measurement + Measurement(std::move(a)));
  }
  Quantity operator-(const Quantity& q, val::BaseValue::PointerType a) {
    UnitSystem us(q.stype);
    return Quantity(q.measurement - Measurement(std::move(a)));
  }
  Quantity operator*(const Quantity& q, val::BaseValue::PointerType a) {
    UnitSystem us(q.stype);
    return Quantity(q.measurement * Measurement(std::move(a)));
  }
  Quantity operator/(const Quantity& q, val::BaseValue::PointerType a) {
    UnitSystem us(q.stype);
    return Quantity(q.measurement / Measurement(std::move(a)));
  }

  // overloading unary operations
  Quantity operator+(const Quantity& q) {
    return Quantity(+q.measurement);
  }
  Quantity operator-(const Quantity& q) {
    return Quantity(-q.measurement);
  }

  // overloading self assignment operations
  void Quantity::operator+=(Quantity& q) {
    UnitSystem us(stype);
    if (stype == q.stype) {
      measurement += q.measurement;
    } else {
      Quantity qc = q.convert(measurement.baseunits, stype);
      measurement += qc.measurement;
    }
  }
  void Quantity::operator-=(Quantity& q) {
    UnitSystem us(stype);
    if (stype == q.stype) {
      measurement -= q.measurement;
    } else {
      Quantity qc = q.convert(measurement.baseunits, stype);
      measurement -= qc.measurement;
    }
  }
  void Quantity::operator*=(Quantity& q) {
    if (stype != q.stype)
      throw UnitSystemExcept(stype, q.stype);
    UnitSystem us(stype);
    measurement *= q.measurement;
  }
  void Quantity::operator/=(Quantity& q) {
    if (stype != q.stype)
      throw UnitSystemExcept(stype, q.stype);
    UnitSystem us(stype);
    measurement /= q.measurement;
  }

  // conversion of units
  Measurement Quantity::_convert_without_context(UnitSystem& us, const SystemType stt) const {
    Dimensions dim = measurement.baseunits.dimensions();
    us.change(stt); // change the unit system
    return Measurement(measurement.result, dim);
  }

  Measurement Quantity::_convert_with_context(UnitSystem& us, const SystemType stt,
                                              QuantityListType::iterator& qs1, QuantityListType::iterator& qs2,
                                              const std::string& q) const {
    Measurement msr = measurement;
    if (qs1->second.sifactor != "") {
      msr *= Measurement(std::string(Symbols::si_factor_start) + q + std::string(Symbols::si_factor_end));
    }
    msr = msr.convert(std::string(Symbols::quantity_start) + q + std::string(Symbols::quantity_end));
    us.change(stt);
    msr = Measurement(msr.result, std::string(Symbols::quantity_start) + q + std::string(Symbols::quantity_end));
    if (qs2->second.sifactor != "") {
      msr /= Measurement(std::string(Symbols::si_factor_start) + q + std::string(Symbols::si_factor_end));
    }
    return msr;
  }

  // convert using Format::Base
  Quantity Quantity::convert(const Format::Base& format, SystemType system) const {
    Measurement msr = measurement;
    if (system == SystemType::NONE)
      system = stype;
    else if (system != stype)
      msr = measurement.convert(Format::Base::MGS);
    UnitSystem us(system);
    return Quantity(msr.convert(format));
  }

  // convert using quantity
  Quantity Quantity::convert(const Quantity& q) const {
    return convert(q.measurement, q.stype);
  }

  // convert using unit measurement
  Quantity Quantity::convert(const Measurement& msr1) const {
    UnitSystem us(stype);
    Measurement msr2 = measurement.convert(msr1);
    return Quantity(msr2);
  }

  Quantity Quantity::convert(const Measurement& msr1, const SystemType system, const std::string& q) const {
    UnitSystem us(stype);
    if (stype == system) {
      return convert(msr1);
    } else if (q == "") {
      Measurement msr2 = _convert_without_context(us, system);
      try {
        return Quantity(msr2.convert(msr1), system);
      } catch (const snt::puq::ConvDimExcept& e) {
        throw snt::puq::ConvDimExcept(measurement.baseunits, stype, msr1.baseunits, system);
      }
    } else {
      QuantityListType::iterator qs1 = puq::UnitSystem::Data->QuantityList.find(q);
      if (qs1 == puq::UnitSystem::Data->QuantityList.end())
        throw UnitSystemExcept("Quantity symbol not found: " + q);
      us.change(system);
      QuantityListType::iterator qs2 = puq::UnitSystem::Data->QuantityList.find(q);
      if (qs2 == puq::UnitSystem::Data->QuantityList.end())
        throw UnitSystemExcept("Quantity symbol not found: " + q);
      us.change(stype);
      if (qs1->second.sifactor == "" && qs2->second.sifactor == "") {
        Measurement msr2 = _convert_without_context(us, system);
        return Quantity(msr2.convert(msr1), system);
      } else {
        Measurement msr2 = _convert_with_context(us, system, qs1, qs2, q);
        return Quantity(msr2.convert(msr1), system);
      }
    }
  }

  // convert using base units
  Quantity Quantity::convert(const BaseUnits& bu) const {
    UnitSystem us(stype);
    Measurement msr = measurement.convert(bu);
    return Quantity(msr);
  }

  Quantity Quantity::convert(const BaseUnits& bu, const SystemType system, const std::string& q) const {
    UnitSystem us(stype);
    if (stype == system) {
      return convert(bu);
    } else if (q == "") {
      Measurement msr = _convert_without_context(us, system);
      try {
        return Quantity(msr.convert(bu), system);
      } catch (const snt::puq::ConvDimExcept& e) {
        throw snt::puq::ConvDimExcept(measurement.baseunits, stype, Measurement(1, bu).baseunits, system);
      }
    } else {
      QuantityListType::iterator qs1 = puq::UnitSystem::Data->QuantityList.find(q);
      if (qs1 == puq::UnitSystem::Data->QuantityList.end())
        throw UnitSystemExcept("Quantity symbol not found: " + q);
      us.change(system);
      QuantityListType::iterator qs2 = puq::UnitSystem::Data->QuantityList.find(q);
      if (qs2 == puq::UnitSystem::Data->QuantityList.end())
        throw UnitSystemExcept("Quantity symbol not found: " + q);
      us.change(stype);
      if (qs1->second.sifactor == "" && qs2->second.sifactor == "") {
        Measurement msr = _convert_without_context(us, system);
        return Quantity(msr.convert(bu), system);
      } else {
        Measurement msr = _convert_with_context(us, system, qs1, qs2, q);
        return Quantity(msr.convert(bu), system);
      }
    }
  }

  // convert using string expression
  Quantity Quantity::convert(std::string s, SystemType system, const std::string& q) const {
    preprocess(s, system);
    UnitSystem us(stype);
    if (stype == system) {
      Measurement msr = measurement.convert(s);
      return Quantity(msr);
    } else if (q == "") {
      Measurement msr = _convert_without_context(us, system);
      try {
        return Quantity(msr.convert(s), system);
      } catch (const snt::puq::ConvDimExcept& e) {
        throw snt::puq::ConvDimExcept(measurement.baseunits, stype, Measurement(s).baseunits, system);
      }
    } else {
      QuantityListType::iterator qs1 = puq::UnitSystem::Data->QuantityList.find(q);
      if (qs1 == puq::UnitSystem::Data->QuantityList.end())
        throw UnitSystemExcept("Quantity symbol not found: " + q);
      us.change(system);
      QuantityListType::iterator qs2 = puq::UnitSystem::Data->QuantityList.find(q);
      if (qs2 == puq::UnitSystem::Data->QuantityList.end())
        throw UnitSystemExcept("Quantity symbol not found: " + q);
      us.change(stype);
      if (qs1->second.sifactor == "" && qs2->second.sifactor == "") {
        Measurement msr = _convert_without_context(us, system);
        return Quantity(msr.convert(s), system);
      } else {
        Measurement msr = _convert_with_context(us, system, qs1, qs2, q);
        return Quantity(msr.convert(s), system);
      }
    }
  }

  Quantity Quantity::rebase_prefixes() {
    UnitSystem us(stype);
    Measurement msr = measurement.rebase_prefixes();
    return Quantity(msr);
  }

  Quantity Quantity::rebase_dimensions() {
    UnitSystem us(stype);
    Measurement msr = measurement.rebase_dimensions();
    return Quantity(msr);
  }

} // namespace snt::puq
