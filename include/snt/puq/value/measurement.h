#ifndef PUQ_UNIT_VALUE_H
#define PUQ_UNIT_VALUE_H

#include <snt/puq/exponent.h>
#include <snt/puq/magnitude.h>
#include <snt/puq/settings.h>
#include <snt/puq/value/base_units.h>

namespace snt::puq {

  class Measurement {
  public:
    Magnitude magnitude;
    BaseUnits baseunits;
    Measurement() : magnitude(1) {}
    Measurement(const std::string& s);
    Measurement(const Magnitude& m) : magnitude(m) {}
    Measurement(const Magnitude& m, const std::string& s);
    Measurement(const Magnitude& m, const Dimensions& dim);
    Measurement(const Magnitude& m, const BaseUnits& bu) : magnitude(m), baseunits(bu) {};
    Measurement(const Magnitude& m, const BaseUnits::ListType& bul) : magnitude(m), baseunits(bul) {};
    Measurement(const MagnitudeFloat& m, const std::string& s);
    Measurement(const MagnitudeFloat& m) : magnitude(m) {};
    Measurement(const MagnitudeFloat& m, const BaseUnits& bu) : magnitude(m), baseunits(bu) {};
    Measurement(const MagnitudeFloat& m, const BaseUnits::ListType& bul) : magnitude(m), baseunits(bul) {};
    Measurement(const MagnitudeFloat& m, const MagnitudeFloat& e, const std::string& s);
    Measurement(const MagnitudeFloat& m, const MagnitudeFloat& e) : magnitude(m, e) {};
    Measurement(const MagnitudeFloat& m, const MagnitudeFloat& e, const BaseUnits& bu) : magnitude(m, e), baseunits(bu) {};
    Measurement(const MagnitudeFloat& m, const MagnitudeFloat& e, const BaseUnits::ListType& bul) : magnitude(m, e), baseunits(bul) {};
#if defined(MAGNITUDE_VALUES)
    Measurement(val::BaseValue::PointerType m, const std::string& s);
    Measurement(val::BaseValue::PointerType m) : magnitude(std::move(m)) {};
    Measurement(val::BaseValue::PointerType m, const BaseUnits& bu) : magnitude(std::move(m)), baseunits(bu) {};
    Measurement(val::BaseValue::PointerType m, const BaseUnits::ListType& bul) : magnitude(std::move(m)), baseunits(bul) {};
    Measurement(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const std::string& s);
    Measurement(val::BaseValue::PointerType m, val::BaseValue::PointerType e) : magnitude(std::move(m), std::move(e)) {};
    Measurement(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnits& bu) : magnitude(std::move(m), std::move(e)), baseunits(bu) {};
    Measurement(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnits::ListType& bul) : magnitude(std::move(m), std::move(e)), baseunits(bul) {};
#endif
    std::size_t size() const;
#if defined(MAGNITUDE_VALUES)
    val::Array::ShapeType shape() const;
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend bool operator==(const Measurement& v1, const Measurement& v2);
    friend bool operator!=(const Measurement& v1, const Measurement& v2);
    friend Measurement operator+(const Measurement& v1, const Measurement& v2);
    friend Measurement operator-(const Measurement& v1, const Measurement& v2);
    friend Measurement operator*(const Measurement& v1, const Measurement& v2);
    friend Measurement operator/(const Measurement& v1, const Measurement& v2);
    friend Measurement operator+(const Measurement& v);
    friend Measurement operator-(const Measurement& v);
    friend std::ostream& operator<<(std::ostream& os, const Measurement& v);
    void operator+=(const Measurement& v);
    void operator-=(const Measurement& v);
    void operator*=(const Measurement& v);
    void operator/=(const Measurement& v);
    void pow(const ExponentVariant& e);
    Measurement convert(const Format::Base& format) const;
    Measurement convert(const std::string& s) const;
    Measurement convert(const BaseUnits& bu) const;
    Measurement convert(const Measurement& v) const;
    Measurement rebase_prefixes();
    Measurement rebase_dimensions();
  };

} // namespace snt::puq

#endif // PUQ_UNIT_VALUE_H
