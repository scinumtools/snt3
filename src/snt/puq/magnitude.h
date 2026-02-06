#ifndef PUQ_MAGNITUDE_H
#define PUQ_MAGNITUDE_H

#include "../val.h"
#include "exponent.h"
#include "settings.h"
#include "unit_format.h"

namespace snt::puq {

#ifdef MAGNITUDE_ERRORS
  class Magnitude {
  public:
#if defined(MAGNITUDE_VALUES)
    val::BaseValue::PointerType value;
    val::BaseValue::PointerType error;
#else
    MagnitudeFloat value;
    MagnitudeFloat error;
#endif
#ifndef MAGNITUDE_VALUES
    Magnitude() : value(1), error(0) {}
    Magnitude(const MagnitudeFloat& m) : value(m), error(0) {}
    Magnitude(const MagnitudeFloat& m, const MagnitudeFloat& e) : value(m), error(e) {}
    static MagnitudeFloat abs_to_rel(const MagnitudeFloat& v, const MagnitudeFloat& a);
    static MagnitudeFloat rel_to_abs(const MagnitudeFloat& v, const MagnitudeFloat& r);
#else
    Magnitude(const Magnitude& other) {
      value = other.value ? other.value->clone() : nullptr;
      error = other.error ? other.error->clone() : nullptr;
    }
    Magnitude& operator=(const Magnitude& other) {
      if (this != &other) {
        value = other.value ? other.value->clone() : nullptr;
        error = other.error ? other.error->clone() : nullptr;
      }
      return *this;
    }
    Magnitude(Magnitude&&) noexcept = default;
    Magnitude& operator=(Magnitude&&) noexcept = default;
    Magnitude() : value(std::make_unique<val::ArrayValue<double>>(std::vector<double>({1}))),
                  error(nullptr) {}
    Magnitude(const MagnitudeFloat& m) : value(std::make_unique<val::ArrayValue<double>>(std::vector<double>({m}))),
                                              error(nullptr) {}
    Magnitude(const MagnitudeFloat& m, const MagnitudeFloat& e) : value(std::make_unique<val::ArrayValue<double>>(std::vector<double>({m}))),
                                                                            error(std::make_unique<val::ArrayValue<double>>(std::vector<double>({e}))) {}
    static MagnitudeFloat abs_to_rel(const MagnitudeFloat& v, const MagnitudeFloat& a);
    static MagnitudeFloat rel_to_abs(const MagnitudeFloat& v, const MagnitudeFloat& r);
#endif
#if defined(MAGNITUDE_VALUES)
    Magnitude(val::BaseValue::PointerType m);
    Magnitude(val::BaseValue::PointerType m, val::BaseValue::PointerType e);
    static val::BaseValue::PointerType abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType a);
    static val::BaseValue::PointerType rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType r);
#else

#endif
    std::size_t size() const;
#if defined(MAGNITUDE_VALUES)
    val::Array::ShapeType shape() const;
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend Magnitude operator-(const Magnitude& m1);
    friend Magnitude operator+(const Magnitude& m1, const Magnitude& m2);
    friend Magnitude operator-(const Magnitude& m1, const Magnitude& m2);
    friend Magnitude operator*(const Magnitude& m1, const Magnitude& m2);
    friend Magnitude operator/(const Magnitude& m1, const Magnitude& m2);
    friend std::ostream& operator<<(std::ostream& os, const Magnitude& m);
    void operator+=(const Magnitude& m);
    void operator-=(const Magnitude& m);
    void operator*=(const Magnitude& m);
    void operator/=(const Magnitude& m);
    bool operator==(const Magnitude& a) const;
    bool operator!=(const Magnitude& a) const;
    void pow(const EXPONENT_TYPE& e);
  };
#endif

} // namespace snt::puq

#endif // PUQ_MAGNITUDE_H
