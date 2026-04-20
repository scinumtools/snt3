#ifndef PUQ_EXPONENT_H
#define PUQ_EXPONENT_H

#import <snt/puq/settings.h>
#include <snt/puq/unit_format.h>
#include <variant>

namespace snt::puq {

  class Exponent {
  public:
    int numerator;
    int denominator;
    Exponent() : numerator(1), denominator(1) {}
    Exponent(const int n[2]) : numerator(n[0]), denominator(n[1]) {};
    Exponent(const int& n) : numerator(n), denominator(1) {};
    Exponent(const int& n, const int& d) : numerator(n), denominator(d) {};
    friend Exponent operator*(Exponent e1, const Exponent& e2);
    friend Exponent operator/(Exponent e1, const Exponent& e2);
    friend std::ostream& operator<<(std::ostream& os, const Exponent& e);
    Exponent operator-() const;
    void operator+=(const Exponent& e);
    void operator-=(const Exponent& e);
    void operator*=(const Exponent& e);
    void operator/=(const Exponent& e);
    bool operator==(const Exponent& e) const;
    bool operator!=(const Exponent& e) const;
    bool operator==(const int& e) const;
    bool operator!=(const int& e) const;
    bool operator==(const double e) const;
    bool operator!=(const double e) const;
    double to_float() const;
    operator double() const;
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    void reduce();
  };

  inline Exponent operator+(Exponent a, const int& i) {
    a += Exponent{i};
    return a;
  }

  inline Exponent operator+(const int& i, Exponent a) {
    a += Exponent{i};
    return a;
  }

  inline Exponent operator+(Exponent a, const Exponent& b) {
    a += b;
    return a;
  }

  inline Exponent operator-(Exponent a, const int& i) {
    a -= Exponent{i};
    return a;
  }

  inline Exponent operator-(int i, const Exponent& a) {
    Exponent lhs{i};
    lhs -= a;
    return lhs;
  }

  inline Exponent operator-(Exponent a, const Exponent& b) {
    a -= b;
    return a;
  }

  inline Exponent operator*(const Exponent& e, const int& i) {
    return e * Exponent{i};
  }

  inline Exponent operator*(const int& i, const Exponent& e) {
    return Exponent{i} * e;
  }

  inline void operator*=(Exponent& e, const int& i) {
    e *= Exponent{i};
  }

  using ExponentVariant = std::variant<int, Exponent>;

  inline ExponentVariant add_exp(const ExponentVariant& x, const ExponentVariant& y) {
    return std::visit([](auto const& a, auto const& b) -> ExponentVariant {
      using A = std::decay_t<decltype(a)>;
      using B = std::decay_t<decltype(b)>;

      if constexpr (std::is_same_v<A, int> && std::is_same_v<B, int>)
        return a + b;
      else
        return Exponent(a) + Exponent(b);
    },
                      x, y);
  }

  inline ExponentVariant mul_exp(const ExponentVariant& x, const ExponentVariant& y) {
    return std::visit([](auto const& a, auto const& b) -> ExponentVariant {
      using A = std::decay_t<decltype(a)>;
      using B = std::decay_t<decltype(b)>;

      if constexpr (std::is_same_v<A, int> && std::is_same_v<B, int>)
        return a * b;
      else
        return Exponent(a) * Exponent(b);
    },
                      x, y);
  }

  inline double exponent_to_float(const ExponentVariant& exp) {
    if (std::holds_alternative<int>(exp))
      return (double)std::get<int>(exp);
    else
      return (double)std::get<Exponent>(exp);
  };

} // namespace snt::puq

#endif // PUQ_EXPONENT_H
