#include <numeric>
#include <snt/puq/exponent.h>
#include <sstream>

namespace snt::puq {

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

  Exponent operator/(const Exponent& e1, const Exponent& e2) {
    Exponent ne;
    ne.numerator = e1.numerator * e2.denominator;
    ne.denominator = e1.denominator * e2.numerator;
    ne.reduce();
    return ne;
  }
  void Exponent::operator/=(const Exponent& e) {
    numerator *= e.denominator;
    denominator *= e.numerator;
    reduce();
  }
  
  Exponent Exponent::operator-() const {
    return Exponent(-numerator, denominator);
  }

  bool Exponent::operator==(const Exponent& e) const {
    return to_float() == e.to_float();
  }

  bool Exponent::operator!=(const Exponent& e) const {
    return to_float() != e.to_float();
  }

  bool Exponent::operator==(const int& e) const {
    return to_float() == e;
  }

  bool Exponent::operator!=(const int& e) const {
    return to_float() != e;
  }

  bool Exponent::operator==(const double e) const {
    return to_float() == e;
  }

  bool Exponent::operator!=(const double e) const {
    return to_float() != e;
  }

  std::ostream& operator<<(std::ostream& os, const Exponent& e) {
    os << e.to_string();
    return os;
  }

  /*
   *  Convert rational exponents to float numbers
   */
  double Exponent::to_float() const {
    return (double)numerator / (double)denominator;
  }

  /*
   *  Cast as a float number
   */
  Exponent::operator double() const {
    return to_float();
  }

  /*
   *  Create a string representation of an exponent
   */
  std::string Exponent::to_string(const UnitFormat& format) const {
    Exponent e(numerator, denominator);
    e.reduce();
    std::stringstream ss;
    if (e.denominator != 1)
      ss << std::to_string(e.numerator) << Symbols::fraction_separator << std::to_string(e.denominator);
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

} // namespace snt::puq
