#ifndef PUQ_EXPONENT_H
#define PUQ_EXPONENT_H

#import "settings.h"
#include "unit_format.h"

namespace snt::puq {

#ifdef EXPONENT_FRACTIONS

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

#endif

} // namespace snt::puq

#endif // PUQ_EXPONENT_H
