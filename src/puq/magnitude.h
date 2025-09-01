#ifndef PUQ_MAGNITUDE_H
#define PUQ_MAGNITUDE_H

#include "../val/pch_val.h"
#include "array.h"
#include "settings.h"

namespace snt::puq {

#ifdef MAGNITUDE_ERRORS
  class Magnitude {
  public:
#if defined(MAGNITUDE_ARRAYS)
    Array value;
    Array error;
#elif defined(MAGNITUDE_VALUES)
    val::BaseValue::PointerType value;
    val::BaseValue::PointerType error;
#else
    MAGNITUDE_PRECISION value;
    MAGNITUDE_PRECISION error;
#endif
#ifndef MAGNITUDE_VALUES
    Magnitude() : value(1), error(0) {}
    Magnitude(const MAGNITUDE_PRECISION& m) : value(m), error(0) {}
    Magnitude(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e) : value(m), error(e) {}
    static MAGNITUDE_PRECISION abs_to_rel(const MAGNITUDE_PRECISION& v, const MAGNITUDE_PRECISION& a);
    static MAGNITUDE_PRECISION rel_to_abs(const MAGNITUDE_PRECISION& v, const MAGNITUDE_PRECISION& r);
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
    Magnitude(const MAGNITUDE_PRECISION& m) : value(std::make_unique<val::ArrayValue<double>>(std::vector<double>({m}))),
                                              error(nullptr) {}
    Magnitude(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e) : value(std::make_unique<val::ArrayValue<double>>(std::vector<double>({m}))),
                                                                            error(std::make_unique<val::ArrayValue<double>>(std::vector<double>({e}))) {}
    static MAGNITUDE_PRECISION abs_to_rel(const MAGNITUDE_PRECISION& v, const MAGNITUDE_PRECISION& a);
    static MAGNITUDE_PRECISION rel_to_abs(const MAGNITUDE_PRECISION& v, const MAGNITUDE_PRECISION& r);
#endif
#if defined(MAGNITUDE_ARRAYS)
    Magnitude(const Array& m);
    Magnitude(const Array& m, const Array& e);
    static Array abs_to_rel(const Array& v, const Array& a);
    static Array rel_to_abs(const Array& v, const Array& r);
#elif defined(MAGNITUDE_VALUES)
    Magnitude(val::BaseValue::PointerType m);
    Magnitude(val::BaseValue::PointerType m, val::BaseValue::PointerType e);
    static val::BaseValue::PointerType abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType a);
    static val::BaseValue::PointerType rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType r);
#else

#endif
    std::size_t size() const;
#if defined(MAGNITUDE_ARRAYS)
    ArrayShape shape() const;
#elif defined(MAGNITUDE_VALUES)
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
