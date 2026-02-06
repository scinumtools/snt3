#ifndef PUQ_BASE_UNITS_H
#define PUQ_BASE_UNITS_H

#include "../exponent.h"
#include "../settings.h"
#include "dimensions.h"

namespace snt::puq {

  class BaseUnit {
  public:
    std::string prefix;
    std::string unit;
    EXPONENT_TYPE exponent;
    BaseUnit() {};
    BaseUnit(const ExponentInt& n) : exponent(n) {};
#ifdef EXPONENT_FRACTIONS
    BaseUnit(const ExponentInt& n, const ExponentInt& d) : exponent(n, d) {};
#endif
    BaseUnit(const std::string& p, const std::string& u, const EXPONENT_TYPE& e) : prefix(p), unit(u), exponent(e) {};
#ifdef EXPONENT_FRACTIONS
    BaseUnit(const std::string& p, const std::string& u, const ExponentInt& n, const ExponentInt& d) : prefix(p), unit(u), exponent(n, d) {};
#endif
    std::string to_string(const UnitFormat& format = UnitFormat());
  };

  class BaseUnits {
  public:
    typedef std::vector<BaseUnit> ListType;
    BaseUnits::ListType baseunits;
    BaseUnits() {}
    BaseUnits(const std::string& s);
    BaseUnits(const BaseUnits::ListType& bu) : baseunits(bu) {}
    void append(const BaseUnit& bu);
    void append(const std::string& p, const std::string& u, EXPONENT_TYPE e);
#ifdef EXPONENT_FRACTIONS
    void append(const std::string& p, const std::string& u, ExponentInt n, ExponentInt d);
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    const BaseUnit& operator[](int index) const;
    friend BaseUnits operator+(const BaseUnits& bu1, const BaseUnits& bu2);
    friend BaseUnits operator-(const BaseUnits& bu1, const BaseUnits& bu2);
    friend std::ostream& operator<<(std::ostream& os, const BaseUnits& bu);
    void operator+=(const BaseUnits& bu);
    void operator-=(const BaseUnits& bu);
    void operator*=(const EXPONENT_TYPE& e);
    BaseUnits::ListType::const_iterator begin() const;
    BaseUnits::ListType::const_iterator end() const;
    std::size_t size() const;
    Dimensions dimensions() const;
  };

} // namespace snt::puq

#endif // PUQ_BASE_UNITS_H
