#include "exponent.h"

#include <numeric>
#include <sstream>

#ifdef EXPONENT_FRACTIONS

namespace puq {

  void Exponent::operator+=(const Exponent& e) {
    if (denominator == e.denominator) {
      numerator += e.numerator;
    } else {
      numerator = numerator * e.denominator + denominator * e.numerator;
      denominator = denominator * e.denominator;
    }
    reduce();
  }

  void Exponent::operator-=(const Exponent& e) {
    if (denominator == e.denominator) {
      numerator -= e.numerator;
    } else {
      numerator = numerator * e.denominator - denominator * e.numerator;
      denominator = denominator * e.denominator;
    }
    reduce();
  }

  Exponent operator*(const Exponent& e1, const Exponent& e2) {
    Exponent ne;
    ne.numerator = e1.numerator * e2.numerator;
    ne.denominator = e1.denominator * e2.denominator;
    ne.reduce();
    return ne;
  }
  void Exponent::operator*=(const Exponent& e) {
    numerator *= e.numerator;
    denominator *= e.denominator;
    reduce();
  }

  Exponent Exponent::operator-() const {
    return Exponent(-numerator, denominator);
  }

  bool Exponent::operator==(const Exponent& e) const {
    return to_real() == e.to_real();
  }

  bool Exponent::operator!=(const Exponent& e) const {
    return to_real() != e.to_real();
  }

  bool Exponent::operator==(const EXPONENT_INT_PRECISION& e) const {
    return to_real() == e;
  }

  bool Exponent::operator!=(const EXPONENT_INT_PRECISION& e) const {
    return to_real() != e;
  }

  bool Exponent::operator==(const EXPONENT_REAL_PRECISION& e) const {
    return to_real() == e;
  }

  bool Exponent::operator!=(const EXPONENT_REAL_PRECISION& e) const {
    return to_real() != e;
  }

  std::ostream& operator<<(std::ostream& os, const Exponent& e) {
    os << e.to_string();
    return os;
  }

  /*
   *  Convert rational exponents to real numbers
   */
  EXPONENT_REAL_PRECISION Exponent::to_real() const {
    return (EXPONENT_REAL_PRECISION)numerator / (EXPONENT_REAL_PRECISION)denominator;
  }

  /*
   *  Cast as a real number
   */
  Exponent::operator EXPONENT_REAL_PRECISION() const {
    return to_real();
  }

  /*
   *  Create a string representation of an exponent
   */
  std::string Exponent::to_string(const UnitFormat& format) const {
    Exponent e(numerator, denominator);
    e.reduce();
    std::stringstream ss;
    if (e.denominator != 1)
      ss << std::to_string(e.numerator) << SYMBOL_FRACTION << std::to_string(e.denominator);
    else
      ss << std::to_string(e.numerator);
    return format.format_exponents(ss.str());
  }

  /*
   *  Reduce fraction to a standard minimal form
   */
  void Exponent::reduce() {
    if (numerator == 0) { // zero exponent
      numerator = 0;
      denominator = 1;
      return;
    }
    if (denominator < 0) { // keep minus sign in a numerator
      numerator = -numerator;
      denominator = -denominator;
    }
    int gcd = std::gcd(numerator, denominator);
    if (gcd > 1) {
      numerator = numerator / gcd;
      denominator = denominator / gcd;
    }
  }

} // namespace puq

#endif // EXPONENT_FRACTIONS
