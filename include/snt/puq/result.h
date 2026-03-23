#ifndef PUQ_MAGNITUDE_H
#define PUQ_MAGNITUDE_H

#include <snt/puq/exponent.h>
#include <snt/puq/settings.h>
#include <snt/puq/unit_format.h>
#include <snt/val/value_base.h>
#include <snt/val/values_array.h>
#include <variant>

namespace snt::puq {

  using ValueVariant = std::variant<double, val::BaseValue::PointerType>;

  class Result {
  public:
    val::BaseValue::PointerType estimate;
    val::BaseValue::PointerType uncertainty;
    Result(const Result& other) {
      estimate = other.estimate ? other.estimate->clone() : nullptr;
      uncertainty = other.uncertainty ? other.uncertainty->clone() : nullptr;
    }
    Result& operator=(const Result& other) {
      if (this != &other) {
        estimate = other.estimate ? other.estimate->clone() : nullptr;
        uncertainty = other.uncertainty ? other.uncertainty->clone() : nullptr;
      }
      return *this;
    }
    Result(Result&&) noexcept = default;
    Result& operator=(Result&&) noexcept = default;
    Result() : estimate(std::make_unique<val::ArrayValue<double>>(1)),
               uncertainty(nullptr) {}
    Result(const double m) : estimate(std::make_unique<val::ArrayValue<double>>(m)),
                             uncertainty(nullptr) {}
    Result(const double m, const double e) : estimate(std::make_unique<val::ArrayValue<double>>(m)),
                                             uncertainty(std::make_unique<val::ArrayValue<double>>(e)) {}
    Result(val::BaseValue::PointerType m);
    Result(val::BaseValue::PointerType m, val::BaseValue::PointerType e);

    static double abs_to_rel(const double v, const double a);
    static double rel_to_abs(const double v, const double r);
    static val::BaseValue::PointerType abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType a);
    static val::BaseValue::PointerType rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType r);

    std::size_t size() const;
    val::Array::ShapeType shape() const;
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend Result operator-(const Result& m1);
    friend Result operator+(const Result& m1, const Result& m2);
    friend Result operator-(const Result& m1, const Result& m2);
    friend Result operator*(const Result& m1, const Result& m2);
    friend Result operator/(const Result& m1, const Result& m2);
    friend std::ostream& operator<<(std::ostream& os, const Result& m);
    void operator+=(const Result& m);
    void operator-=(const Result& m);
    void operator*=(const Result& m);
    void operator/=(const Result& m);
    bool operator==(const Result& a) const;
    bool operator!=(const Result& a) const;
    void pow(const ExponentVariant& e);
  };

} // namespace snt::puq

#endif // PUQ_MAGNITUDE_H
