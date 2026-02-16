#include <snt/puq/quantity.h>

#include <snt/puq/converter.h>
#include <snt/puq/exceptions.h>
#include <snt/puq/unit_format.h>

#include <iostream>

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
    value = UnitValue(s);
  }

  Quantity::Quantity(const UnitValue& v, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = v;
  }

  Quantity::Quantity(const MAGNITUDE_TYPE& m, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(m);
  }

  Quantity::Quantity(const MAGNITUDE_TYPE& m, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(m, bu);
  }

  Quantity::Quantity(const MAGNITUDE_TYPE& m, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    value = UnitValue(m, s);
  }

#ifdef MAGNITUDE_ERRORS

  Quantity::Quantity(const MagnitudeFloat& m, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(m);
  }

  Quantity::Quantity(const MagnitudeFloat& m, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(m, bu);
  }

  Quantity::Quantity(const MagnitudeFloat& m, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    Magnitude mag(m);
    value = UnitValue(mag, s);
  }

  Quantity::Quantity(const MagnitudeFloat& m, const MagnitudeFloat& e, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(m, e);
  }

  Quantity::Quantity(const MagnitudeFloat& m, const MagnitudeFloat& e, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(m, e, bu);
  }

  Quantity::Quantity(const MagnitudeFloat& m, const MagnitudeFloat& e, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    Magnitude mag(m, e);
    value = UnitValue(mag, s);
  }

#if defined(MAGNITUDE_VALUES)

  Quantity::Quantity(val::BaseValue::PointerType m, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(std::move(m));
  }

  Quantity::Quantity(val::BaseValue::PointerType m, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(std::move(m), bu);
  }

  Quantity::Quantity(val::BaseValue::PointerType m, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    Magnitude mag(std::move(m));
    value = UnitValue(mag, s);
  }

  Quantity::Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(std::move(m), std::move(e));
  }

  Quantity::Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnits::ListType& bu, const SystemType system) : stype(system) {
    UnitSystem us(stype);
    value = UnitValue(std::move(m), std::move(e), bu);
  }

  Quantity::Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, std::string s, const SystemType system) : stype(system) {
    preprocess(s, stype);
    UnitSystem us(stype);
    Magnitude mag(std::move(m), std::move(e));
    value = UnitValue(mag, s);
  }

#endif

#endif

  std::string Quantity::unit_system() const {
    auto it = SystemMap.find(stype);
    if (it == SystemMap.end()) {
      it = SystemMap.find(UnitSystem::System);
    }
    return it->second->SystemAbbrev;
  }

  std::size_t Quantity::size() const {
    return value.size();
  }

#if defined(MAGNITUDE_VALUES)
  val::Array::ShapeType Quantity::shape() const {
    return value.shape();
  }

  Quantity::PointerType Quantity::clone() const {
    if (value.magnitude.error)
      return std::make_unique<Quantity>(value.magnitude.value->clone(), value.magnitude.error->clone(), value.baseunits.baseunits, stype);
    else
      return std::make_unique<Quantity>(value.magnitude.value->clone(), value.baseunits.baseunits, stype);
  }
#endif

  // strings and streams
  std::string Quantity::to_string(const UnitFormat& format) const {
    UnitSystem us(stype);
    std::string qstr = value.to_string(format);
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
      return Quantity(q1.value + q2.value);
    } else {
      Quantity q3 = q2.convert(q1.value.baseunits, q1.stype);
      return Quantity(q1.value + q3.value);
    }
  }
  Quantity operator-(const Quantity& q1, const Quantity& q2) {
    UnitSystem us(q1.stype);
    if (q1.stype == q2.stype) {
      return Quantity(q1.value - q2.value);
    } else {
      Quantity q3 = q2.convert(q1.value.baseunits, q1.stype);
      return Quantity(q1.value - q3.value);
    }
  }
  Quantity operator*(const Quantity& q1, const Quantity& q2) {
    if (q1.stype != q2.stype)
      throw UnitSystemExcept(q1.stype, q2.stype);
    UnitSystem us(q1.stype);
    return Quantity(q1.value * q2.value);
  }
  Quantity operator/(const Quantity& q1, const Quantity& q2) {
    if (q1.stype != q2.stype)
      throw UnitSystemExcept(q1.stype, q2.stype);
    UnitSystem us(q1.stype);
    return Quantity(q1.value / q2.value);
  }
  bool operator==(const Quantity& q1, const Quantity& q2) {
    if (q1.stype != q2.stype)
      throw UnitSystemExcept(q1.stype, q2.stype);
    UnitSystem us(q1.stype);
    return q1.value==q2.value;
  }
  bool operator!=(const Quantity& q1, const Quantity& q2) {
    if (q1.stype != q2.stype)
      throw UnitSystemExcept(q1.stype, q2.stype);
    UnitSystem us(q1.stype);
    return q1.value!=q2.value;
  }

  // overloading scalar/quantity
  Quantity operator+(const MagnitudeFloat& m, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(UnitValue(m) + q.value);
  }
  Quantity operator-(const MagnitudeFloat& m, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(UnitValue(m) - q.value);
  }
  Quantity operator*(const MagnitudeFloat& m, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(UnitValue(m) * q.value);
  }
  Quantity operator/(const MagnitudeFloat& m, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(UnitValue(m) / q.value);
  }

  // overloading quantity/scalar
  Quantity operator+(const Quantity& q, const MagnitudeFloat& m) {
    UnitSystem us(q.stype);
    return Quantity(q.value + UnitValue(m));
  }
  Quantity operator-(const Quantity& q, const MagnitudeFloat& m) {
    UnitSystem us(q.stype);
    return Quantity(q.value - UnitValue(m));
  }
  Quantity operator*(const Quantity& q, const MagnitudeFloat& m) {
    UnitSystem us(q.stype);
    return Quantity(q.value * UnitValue(m));
  }
  Quantity operator/(const Quantity& q, const MagnitudeFloat& m) {
    UnitSystem us(q.stype);
    return Quantity(q.value / UnitValue(m));
  }

#if defined(MAGNITUDE_VALUES)
  // overloading array/quantity
  Quantity operator+(val::BaseValue::PointerType a, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(UnitValue(std::move(a)) + q.value);
  }
  Quantity operator-(val::BaseValue::PointerType a, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(UnitValue(std::move(a)) - q.value);
  }
  Quantity operator*(val::BaseValue::PointerType a, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(UnitValue(std::move(a)) * q.value);
  }
  Quantity operator/(val::BaseValue::PointerType a, const Quantity& q) {
    UnitSystem us(q.stype);
    return Quantity(UnitValue(std::move(a)) / q.value);
  }

  // overloading quantity/array
  Quantity operator+(const Quantity& q, val::BaseValue::PointerType a) {
    UnitSystem us(q.stype);
    return Quantity(q.value + UnitValue(std::move(a)));
  }
  Quantity operator-(const Quantity& q, val::BaseValue::PointerType a) {
    UnitSystem us(q.stype);
    return Quantity(q.value - UnitValue(std::move(a)));
  }
  Quantity operator*(const Quantity& q, val::BaseValue::PointerType a) {
    UnitSystem us(q.stype);
    return Quantity(q.value * UnitValue(std::move(a)));
  }
  Quantity operator/(const Quantity& q, val::BaseValue::PointerType a) {
    UnitSystem us(q.stype);
    return Quantity(q.value / UnitValue(std::move(a)));
  }
#endif

  // overloading unary operations
  Quantity operator+(const Quantity& q) {
    return Quantity(+q.value);
  }
  Quantity operator-(const Quantity& q) {
    return Quantity(-q.value);
  }

  // overloading self assignment operations
  void Quantity::operator+=(Quantity& q) {
    UnitSystem us(stype);
    if (stype == q.stype) {
      value += q.value;
    } else {
      Quantity qc = q.convert(value.baseunits, stype);
      value += qc.value;
    }
  }
  void Quantity::operator-=(Quantity& q) {
    UnitSystem us(stype);
    if (stype == q.stype) {
      value -= q.value;
    } else {
      Quantity qc = q.convert(value.baseunits, stype);
      value -= qc.value;
    }
  }
  void Quantity::operator*=(Quantity& q) {
    if (stype != q.stype)
      throw UnitSystemExcept(stype, q.stype);
    UnitSystem us(stype);
    value *= q.value;
  }
  void Quantity::operator/=(Quantity& q) {
    if (stype != q.stype)
      throw UnitSystemExcept(stype, q.stype);
    UnitSystem us(stype);
    value /= q.value;
  }

  // conversion of units
  UnitValue Quantity::_convert_without_context(UnitSystem& us, const SystemType stt) const {
    Dimensions dim = value.baseunits.dimensions();
    us.change(stt); // change the unit system
    return UnitValue(value.magnitude, dim);
  }

  UnitValue Quantity::_convert_with_context(UnitSystem& us, const SystemType stt,
                                            QuantityListType::iterator& qs1, QuantityListType::iterator& qs2,
                                            const std::string& q) const {
    UnitValue uv = value;
    if (qs1->second.sifactor != "") {
      uv *= UnitValue(std::string(Symbols::si_factor_start) + q + std::string(Symbols::si_factor_end));
    }
    uv = uv.convert(std::string(Symbols::quantity_start) + q + std::string(Symbols::quantity_end));
    us.change(stt);
    uv = UnitValue(uv.magnitude, std::string(Symbols::quantity_start) + q + std::string(Symbols::quantity_end));
    if (qs2->second.sifactor != "") {
      uv /= UnitValue(std::string(Symbols::si_factor_start) + q + std::string(Symbols::si_factor_end));
    }
    return uv;
  }

  // convert using Format::Base
  Quantity Quantity::convert(const Format::Base& format, SystemType system) const {
    UnitValue uv = value;
    if (system == SystemType::NONE)
      system = stype;
    else if (system != stype)
      uv = value.convert(Format::Base::MGS);
    UnitSystem us(system);
    return Quantity(uv.convert(format));
  }

  // convert using quantity
  Quantity Quantity::convert(const Quantity& q) const {
    return convert(q.value, q.stype);
  }

  // convert using unit value
  Quantity Quantity::convert(const UnitValue& uv1) const {
    UnitSystem us(stype);
    UnitValue uv2 = value.convert(uv1);
    return Quantity(uv2);
  }

  Quantity Quantity::convert(const UnitValue& uv1, const SystemType system, const std::string& q) const {
    UnitSystem us(stype);
    if (stype == system) {
      return convert(uv1);
    } else if (q == "") {
      UnitValue uv2 = _convert_without_context(us, system);
      try {
        return Quantity(uv2.convert(uv1), system);
      } catch (const snt::puq::ConvDimExcept& e) {
        throw snt::puq::ConvDimExcept(value.baseunits, stype, uv1.baseunits, system);
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
        UnitValue uv2 = _convert_without_context(us, system);
        return Quantity(uv2.convert(uv1), system);
      } else {
        UnitValue uv2 = _convert_with_context(us, system, qs1, qs2, q);
        return Quantity(uv2.convert(uv1), system);
      }
    }
  }

  // convert using base units
  Quantity Quantity::convert(const BaseUnits& bu) const {
    UnitSystem us(stype);
    UnitValue uv = value.convert(bu);
    return Quantity(uv);
  }

  Quantity Quantity::convert(const BaseUnits& bu, const SystemType system, const std::string& q) const {
    UnitSystem us(stype);
    if (stype == system) {
      return convert(bu);
    } else if (q == "") {
      UnitValue uv = _convert_without_context(us, system);
      try {
        return Quantity(uv.convert(bu), system);
      } catch (const snt::puq::ConvDimExcept& e) {
        throw snt::puq::ConvDimExcept(value.baseunits, stype, UnitValue(1, bu).baseunits, system);
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
        UnitValue uv = _convert_without_context(us, system);
        return Quantity(uv.convert(bu), system);
      } else {
        UnitValue uv = _convert_with_context(us, system, qs1, qs2, q);
        return Quantity(uv.convert(bu), system);
      }
    }
  }

  // convert using string expression
  Quantity Quantity::convert(std::string s, SystemType system, const std::string& q) const {
    preprocess(s, system);
    UnitSystem us(stype);
    if (stype == system) {
      UnitValue uv = value.convert(s);
      return Quantity(uv);
    } else if (q == "") {
      UnitValue uv = _convert_without_context(us, system);
      try {
        return Quantity(uv.convert(s), system);
      } catch (const snt::puq::ConvDimExcept& e) {
        throw snt::puq::ConvDimExcept(value.baseunits, stype, UnitValue(s).baseunits, system);
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
        UnitValue uv = _convert_without_context(us, system);
        return Quantity(uv.convert(s), system);
      } else {
        UnitValue uv = _convert_with_context(us, system, qs1, qs2, q);
        return Quantity(uv.convert(s), system);
      }
    }
  }

  Quantity Quantity::rebase_prefixes() {
    UnitSystem us(stype);
    UnitValue uv = value.rebase_prefixes();
    return Quantity(uv);
  }

  Quantity Quantity::rebase_dimensions() {
    UnitSystem us(stype);
    UnitValue uv = value.rebase_dimensions();
    return Quantity(uv);
  }

} // namespace snt::puq
