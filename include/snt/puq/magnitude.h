#ifndef PUQ_MAGNITUDE_H
#define PUQ_MAGNITUDE_H

#include <snt/val/value_base.h>
#include <snt/val/values_array.h>
#include <snt/puq/exponent.h>
#include <snt/puq/settings.h>
#include <snt/puq/unit_format.h>

#include <variant>

namespace snt::puq {

  using ValueVariant = std::variant<MagnitudeFloat, val::BaseValue::PointerType>;
  
  class Magnitude {
  public:
#if defined(MAGNITUDE_VALUES)
    val::BaseValue::PointerType estimate;
    val::BaseValue::PointerType uncertainty;
#else
    MagnitudeFloat estimate;
    MagnitudeFloat uncertainty;
#endif
#ifndef MAGNITUDE_VALUES
    Magnitude() : estimate(1), uncertainty(0) {}
    Magnitude(const MagnitudeFloat& m) : estimate(m), uncertainty(0) {}
    Magnitude(const MagnitudeFloat& m, const MagnitudeFloat& e) : estimate(m), uncertainty(e) {}
    static MagnitudeFloat abs_to_rel(const MagnitudeFloat& v, const MagnitudeFloat& a);
    static MagnitudeFloat rel_to_abs(const MagnitudeFloat& v, const MagnitudeFloat& r);
#else
    Magnitude(const Magnitude& other) {
      estimate = other.estimate ? other.estimate->clone() : nullptr;
      uncertainty = other.uncertainty ? other.uncertainty->clone() : nullptr;
    }
    Magnitude& operator=(const Magnitude& other) {
      if (this != &other) {
        estimate = other.estimate ? other.estimate->clone() : nullptr;
        uncertainty = other.uncertainty ? other.uncertainty->clone() : nullptr;
      }
      return *this;
    }
    Magnitude(Magnitude&&) noexcept = default;
    Magnitude& operator=(Magnitude&&) noexcept = default;
    Magnitude() : estimate(std::make_unique<val::ArrayValue<double>>(std::vector<double>({1}))),
                  uncertainty(nullptr) {}
    Magnitude(const MagnitudeFloat& m) : estimate(std::make_unique<val::ArrayValue<double>>(std::vector<double>({m}))),
                                              uncertainty(nullptr) {}
    Magnitude(const MagnitudeFloat& m, const MagnitudeFloat& e) : estimate(std::make_unique<val::ArrayValue<double>>(std::vector<double>({m}))),
                                                                            uncertainty(std::make_unique<val::ArrayValue<double>>(std::vector<double>({e}))) {}
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
    void pow(const ExponentVariant& e);
  };

} // namespace snt::puq

#endif // PUQ_MAGNITUDE_H
