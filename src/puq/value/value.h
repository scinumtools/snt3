#ifndef PUQ_VALUE_H
#define PUQ_VALUE_H

#include <vector>

#include "../settings.h"
#include "../systems/systems.h"

namespace puq {

  class UnitValueExcept : public std::exception {
  private:
    std::string message;

  public:
    UnitValueExcept(std::string m) : message(m) {}
    const char* what() const noexcept override {
      return message.c_str();
    }
  };

  class Dimensions {
  public:
    MAGNITUDE_TYPE numerical;
    BaseDimensions physical;
    std::vector<std::string> symbols;
    Utype utype;
    Dimensions();
    Dimensions(const MAGNITUDE_TYPE& n);
    Dimensions(const MAGNITUDE_TYPE& n, const BaseDimensions& p) : utype(Utype::NUL), numerical(n), physical(p) {};
#ifdef MAGNITUDE_ERRORS
    Dimensions(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e);
    Dimensions(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e, const BaseDimensions& p) : utype(Utype::NUL), numerical(m, e), physical(p) {};
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend std::ostream& operator<<(std::ostream& os, const Dimensions& d);
    bool operator==(const Dimensions& d) const;
    bool operator!=(const Dimensions& d) const;
  };

  class BaseUnit {
  public:
    std::string prefix;
    std::string unit;
    EXPONENT_TYPE exponent;
    BaseUnit() {};
    BaseUnit(const EXPONENT_INT_PRECISION& n) : exponent(n) {};
#ifdef EXPONENT_FRACTIONS
    BaseUnit(const EXPONENT_INT_PRECISION& n, const EXPONENT_INT_PRECISION& d) : exponent(n, d) {};
#endif
    BaseUnit(const std::string& p, const std::string& u, const EXPONENT_TYPE& e) : prefix(p), unit(u), exponent(e) {};
#ifdef EXPONENT_FRACTIONS
    BaseUnit(const std::string& p, const std::string& u, const EXPONENT_INT_PRECISION& n, const EXPONENT_INT_PRECISION& d) : prefix(p), unit(u), exponent(n, d) {};
#endif
    std::string to_string(const UnitFormat& format = UnitFormat());
  };

  typedef std::vector<BaseUnit> BaseUnitsList;
  class BaseUnits {
  private:
    BaseUnitsList baseunits;

  public:
    BaseUnits() {}
    BaseUnits(const std::string& s);
    BaseUnits(const BaseUnitsList& bu) : baseunits(bu) {}
    void append(BaseUnit bu);
    void append(std::string p, std::string u, EXPONENT_TYPE e);
#ifdef EXPONENT_FRACTIONS
    void append(std::string p, std::string u, EXPONENT_INT_PRECISION n, EXPONENT_INT_PRECISION d);
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    const BaseUnit& operator[](int index) const;
    friend BaseUnits operator+(const BaseUnits& bu1, const BaseUnits& bu2);
    friend BaseUnits operator-(const BaseUnits& bu1, const BaseUnits& bu2);
    friend std::ostream& operator<<(std::ostream& os, const BaseUnits& bu);
    void operator+=(const BaseUnits& bu);
    void operator-=(const BaseUnits& bu);
    void operator*=(const EXPONENT_TYPE& e);
    BaseUnitsList::const_iterator begin() const;
    BaseUnitsList::const_iterator end() const;
    std::size_t size() const;
    Dimensions dimensions() const;
  };

  class UnitValue {
  public:
    MAGNITUDE_TYPE magnitude;
    BaseUnits baseunits;
    UnitValue() : magnitude(1) {}
    UnitValue(const std::string& s);
    UnitValue(const MAGNITUDE_TYPE& m) : magnitude(m) {}
    UnitValue(const MAGNITUDE_TYPE& m, const std::string& s);
    UnitValue(const MAGNITUDE_TYPE& m, const Dimensions& dim);
    UnitValue(const MAGNITUDE_TYPE& m, const BaseUnits& bu) : magnitude(m), baseunits(bu) {};
    UnitValue(const MAGNITUDE_TYPE& m, const BaseUnitsList& bul) : magnitude(m), baseunits(bul) {};
#ifdef MAGNITUDE_ERRORS
    UnitValue(const MAGNITUDE_PRECISION& m, const std::string& s);
    UnitValue(const MAGNITUDE_PRECISION& m) : magnitude(m) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const BaseUnits& bu) : magnitude(m), baseunits(bu) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const BaseUnitsList& bul) : magnitude(m), baseunits(bul) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e, const std::string& s);
    UnitValue(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e) : magnitude(m, e) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e, const BaseUnits& bu) : magnitude(m, e), baseunits(bu) {};
    UnitValue(const MAGNITUDE_PRECISION& m, const MAGNITUDE_PRECISION& e, const BaseUnitsList& bul) : magnitude(m, e), baseunits(bul) {};
#if defined(MAGNITUDE_ARRAYS)
    UnitValue(const Array& m, const std::string& s);
    UnitValue(const Array& m) : magnitude(m) {};
    UnitValue(const Array& m, const BaseUnits& bu) : magnitude(m), baseunits(bu) {};
    UnitValue(const Array& m, const BaseUnitsList& bul) : magnitude(m), baseunits(bul) {};
    UnitValue(const Array& m, const Array& e, const std::string& s);
    UnitValue(const Array& m, const Array& e) : magnitude(m, e) {};
    UnitValue(const Array& m, const Array& e, const BaseUnits& bu) : magnitude(m, e), baseunits(bu) {};
    UnitValue(const Array& m, const Array& e, const BaseUnitsList& bul) : magnitude(m, e), baseunits(bul) {};
#elif defined(MAGNITUDE_VALUES)
    UnitValue(val::BaseValue::PointerType m, const std::string& s);
    UnitValue(val::BaseValue::PointerType m) : magnitude(std::move(m)) {};
    UnitValue(val::BaseValue::PointerType m, const BaseUnits& bu) : magnitude(std::move(m)), baseunits(bu) {};
    UnitValue(val::BaseValue::PointerType m, const BaseUnitsList& bul) : magnitude(std::move(m)), baseunits(bul) {};
    UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const std::string& s);
    UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e) : magnitude(std::move(m), std::move(e)) {};
    UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnits& bu) : magnitude(std::move(m), std::move(e)), baseunits(bu) {};
    UnitValue(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnitsList& bul) : magnitude(std::move(m), std::move(e)), baseunits(bul) {};
#endif
#endif
    std::size_t size() const;
#if defined(MAGNITUDE_ARRAYS)
    ArrayShape shape() const;
#elif defined(MAGNITUDE_VALUES)
    val::Array::ShapeType shape() const;
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend UnitValue operator+(const UnitValue& v1, const UnitValue& v2);
    friend UnitValue operator-(const UnitValue& v1, const UnitValue& v2);
    friend UnitValue operator*(const UnitValue& v1, const UnitValue& v2);
    friend UnitValue operator/(const UnitValue& v1, const UnitValue& v2);
    friend UnitValue operator+(const UnitValue& v);
    friend UnitValue operator-(const UnitValue& v);
    friend std::ostream& operator<<(std::ostream& os, const UnitValue& v);
    void operator+=(const UnitValue& v);
    void operator-=(const UnitValue& v);
    void operator*=(const UnitValue& v);
    void operator/=(const UnitValue& v);
    void pow(const EXPONENT_TYPE& e);
    UnitValue convert(const Format::Base& format) const;
    UnitValue convert(const std::string& s) const;
    UnitValue convert(const BaseUnits& bu) const;
    UnitValue convert(const UnitValue& v) const;
    UnitValue rebase_prefixes();
    UnitValue rebase_dimensions();
  };

} // namespace puq

#endif // PUQ_VALUE_H
