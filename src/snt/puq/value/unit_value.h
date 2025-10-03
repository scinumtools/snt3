#ifndef PUQ_UNIT_VALUE_H
#define PUQ_UNIT_VALUE_H

#include "../exponent.h"
#include "../magnitude.h"
#include "../settings.h"
#include "base_units.h"

namespace snt::puq {

  class UnitValue {
  public:
    MAGNITUDE_TYPE magnitude;
    BaseUnits baseunits;
    UnitValue() : magnitude(1) {}
    UnitValue(const std::string& s);
    UnitValue(const MAGNITUDE_TYPE& m) : magnitude(m) {}
    UnitValue(const MAGNITUDE_TYPE& m, const std::string& s);
    UnitValue(const MAGNITUDE_TYPE& m, const Dimensions& dim);
    UnitValue(const MAGNITUDE_TYPE& m, const BaseUnits& bu) : magnitude(m), baseunits(bu) {};
    UnitValue(const MAGNITUDE_TYPE& m, const BaseUnitsList& bul) : magnitude(m), baseunits(bul) {};
#ifdef MAGNITUDE_ERRORS
    UnitValue(const MAGNITUDE_PRECISION& m, const std::string& s);
    UnitValue(const MAGNITUDE_PRECISION& m) : magnitude(m) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const BaseUnits& bu) : magnitude(m), baseunits(bu) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const BaseUnitsList& bul) : magnitude(m), baseunits(bul) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e, const std::string& s);
    UnitValue(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e) : magnitude(m, e) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e, const BaseUnits& bu) : magnitude(m, e), baseunits(bu) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e, const BaseUnitsList& bul) : magnitude(m, e), baseunits(bul) {};
#if defined(MAGNITUDE_VALUES)
    UnitValue(val::BaseValue::PointerType m, const std::string& s);
    UnitValue(val::BaseValue::PointerType m) : magnitude(std::move(m)) {};
    UnitValue(val::BaseValue::PointerType m, const BaseUnits& bu) : magnitude(std::move(m)), baseunits(bu) {};
    UnitValue(val::BaseValue::PointerType m, const BaseUnitsList& bul) : magnitude(std::move(m)), baseunits(bul) {};
    UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const std::string& s);
    UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e) : magnitude(std::move(m), std::move(e)) {};
    UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnits& bu) : magnitude(std::move(m), std::move(e)), baseunits(bu) {};
    UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnitsList& bul) : magnitude(std::move(m), std::move(e)), baseunits(bul) {};
#endif
#endif
    std::size_t size() const;
#if defined(MAGNITUDE_VALUES)
    val::Array::ShapeType shape() const;
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend UnitValue operator+(const UnitValue& v1, const UnitValue& v2);
    friend UnitValue operator-(const UnitValue& v1, const UnitValue& v2);
    friend UnitValue operator*(const UnitValue& v1, const UnitValue& v2);
    friend UnitValue operator/(const UnitValue& v1, const UnitValue& v2);
    friend UnitValue operator+(const UnitValue& v);
    friend UnitValue operator-(const UnitValue& v);
    friend std::ostream& operator<<(std::ostream& os, const UnitValue& v);
    void operator+=(const UnitValue& v);
    void operator-=(const UnitValue& v);
    void operator*=(const UnitValue& v);
    void operator/=(const UnitValue& v);
    void pow(const EXPONENT_TYPE& e);
    UnitValue convert(const Format::Base& format) const;
    UnitValue convert(const std::string& s) const;
    UnitValue convert(const BaseUnits& bu) const;
    UnitValue convert(const UnitValue& v) const;
    UnitValue rebase_prefixes();
    UnitValue rebase_dimensions();
  };

} // namespace snt::puq

#endif // PUQ_UNIT_VALUE_H
