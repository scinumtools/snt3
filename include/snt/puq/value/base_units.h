#ifndef PUQ_BASE_UNITS_H
#define PUQ_BASE_UNITS_H

#include <snt/puq/exponent.h>
#include <snt/puq/settings.h>
#include <snt/puq/value/dimensions.h>

namespace snt::puq {
  
  class BaseUnit {
  public:
    std::string prefix;
    std::string unit;
    ExponentVariant exponent;
    BaseUnit() {};
    BaseUnit(const ExponentInt& n) :
      exponent(n) {};
    BaseUnit(const ExponentInt& n, const ExponentInt& d) :
      exponent(Exponent(n, d)) {};
    BaseUnit(const std::string& p, const std::string& u, const ExponentVariant& e) :
      prefix(p), unit(u), exponent(e) {};
    BaseUnit(const std::string& p, const std::string& u, const ExponentInt& n, const ExponentInt& d) :
      prefix(p), unit(u), exponent(Exponent(n, d)) {};
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
    void append(const std::string& p, const std::string& u, ExponentVariant e);
    void append(const std::string& p, const std::string& u, ExponentInt n, ExponentInt d);
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    const BaseUnit& operator[](int index) const;
    friend BaseUnits operator+(const BaseUnits& bu1, const BaseUnits& bu2);
    friend BaseUnits operator-(const BaseUnits& bu1, const BaseUnits& bu2);
    friend std::ostream& operator<<(std::ostream& os, const BaseUnits& bu);
    void operator+=(const BaseUnits& bu);
    void operator-=(const BaseUnits& bu);
    void operator*=(const ExponentVariant& e);
    BaseUnits::ListType::const_iterator begin() const;
    BaseUnits::ListType::const_iterator end() const;
    std::size_t size() const;
    Dimensions dimensions() const;
  };

} // namespace snt::puq

#endif // PUQ_BASE_UNITS_H
