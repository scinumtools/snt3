#ifndef PUQ_EXPONENT_H
#define PUQ_EXPONENT_H

#import <snt/puq/settings.h>
#include <snt/puq/unit_format.h>

namespace snt::puq {

  class Exponent {
  public:
    ExponentInt numerator;
    ExponentInt denominator;
    Exponent() : numerator(1), denominator(1) {}
    Exponent(const ExponentInt n[2]) : numerator(n[0]), denominator(n[1]) {};
    Exponent(const ExponentInt& n) : numerator(n), denominator(1) {};
    Exponent(const ExponentInt& n, const ExponentInt& d) : numerator(n), denominator(d) {};
    friend Exponent operator*(const Exponent& e1, const Exponent& e2);
    friend std::ostream& operator<<(std::ostream& os, const Exponent& e);
    Exponent operator-() const;
    void operator+=(const Exponent& e);
    void operator-=(const Exponent& e);
    void operator*=(const Exponent& e);
    bool operator==(const Exponent& e) const;
    bool operator!=(const Exponent& e) const;
    bool operator==(const ExponentInt& e) const;
    bool operator!=(const ExponentInt& e) const;
    bool operator==(const ExponentFloat& e) const;
    bool operator!=(const ExponentFloat& e) const;
    ExponentFloat to_float() const;
    operator ExponentFloat() const;
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    void reduce();
  };

  inline Exponent operator+(Exponent a, const ExponentInt& i) {
    a += Exponent{i};
    return a;
  }
  
  inline Exponent operator+(const ExponentInt& i, Exponent a) {
    a += Exponent{i};
    return a;
  }
  
  inline Exponent operator+(Exponent a, const Exponent& b) {
    a += b;
    return a;
  }
  
  inline Exponent operator-(Exponent a, const ExponentInt& i) {
    a -= Exponent{i};
    return a;
  }

  inline Exponent operator-(ExponentInt i, const Exponent& a) {
    Exponent lhs{i};
    lhs -= a;
    return lhs;
  }

  inline Exponent operator-(Exponent a, const Exponent& b) {
    a -= b;
    return a;
  }
  
  inline Exponent operator*(const Exponent& e, const ExponentInt& i) {
    return e * Exponent{i};
  }
  
  inline Exponent operator*(const ExponentInt& i, const Exponent& e) {
    return Exponent{i} * e;
  }
  
  inline void operator*=(Exponent& e, const ExponentInt& i) {
    e *= Exponent{i};
  }
  
  using ExponentVariant = std::variant<int, Exponent>;
  
  inline ExponentVariant add_exp(const ExponentVariant& x, const ExponentVariant& y)
  {
    return std::visit([](auto const& a, auto const& b) -> ExponentVariant {
      using A = std::decay_t<decltype(a)>;
      using B = std::decay_t<decltype(b)>;
      
      if constexpr (std::is_same_v<A,int> && std::is_same_v<B,int>)
	return a + b;
      else
	return Exponent(a) + Exponent(b);
    }, x, y);
  }
  
  inline ExponentVariant mul_exp(const ExponentVariant& x, const ExponentVariant& y)
  {
    return std::visit([](auto const& a, auto const& b) -> ExponentVariant {
      using A = std::decay_t<decltype(a)>;
      using B = std::decay_t<decltype(b)>;
      
      if constexpr (std::is_same_v<A,int> && std::is_same_v<B,int>)
	return a * b;
      else
	return Exponent(a) * Exponent(b);
    }, x, y);
  }
  
  inline ExponentFloat exponent_to_float(const ExponentVariant& exp) {
    if (std::holds_alternative<int>(exp))
      return (ExponentFloat)std::get<int>(exp);
    else
      return (ExponentFloat)std::get<Exponent>(exp);
  };
  
} // namespace snt::puq

#endif // PUQ_EXPONENT_H
