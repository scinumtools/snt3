#ifndef PUQ_QUANTITY_H
#define PUQ_QUANTITY_H

#include <snt/puq/settings.h>
#include <snt/puq/solver/unit_solver.h>
#include <snt/puq/systems/unit_system.h>

#include <iomanip>
#include <sstream>

namespace snt::puq {

  class Quantity {
  public:
    typedef std::unique_ptr<Quantity> PointerType;

  private:
    void preprocess(std::string& expression, SystemType& system) const;
    UnitValue _convert_without_context(UnitSystem& us, const SystemType stt) const;
    UnitValue _convert_with_context(UnitSystem& us, const SystemType stt, QuantityListType::iterator& qs1, QuantityListType::iterator& qs2, const std::string& q) const;

  public:
    SystemType stype;
    UnitValue value;
    Quantity() : stype(UnitSystem::System) {};
    Quantity(std::string s, const SystemType system = SystemType::NONE);
    Quantity(const UnitValue& v, const SystemType system = UnitSystem::System);
    Quantity(const MAGNITUDE_TYPE& m, std::string s, const SystemType system = SystemType::NONE);
    Quantity(const MAGNITUDE_TYPE& m, const SystemType system = UnitSystem::System);
    Quantity(const MAGNITUDE_TYPE& m, const BaseUnits::ListType& bu, const SystemType system = UnitSystem::System);
#ifdef MAGNITUDE_ERRORS
    Quantity(const MagnitudeFloat& m, std::string s, const SystemType system = SystemType::NONE);
    Quantity(const MagnitudeFloat& m, const SystemType system = UnitSystem::System);
    Quantity(const MagnitudeFloat& m, const BaseUnits::ListType& bu, const SystemType system = UnitSystem::System);
    Quantity(const MagnitudeFloat& m, const MagnitudeFloat& e, std::string s, const SystemType system = SystemType::NONE);
    Quantity(const MagnitudeFloat& m, const MagnitudeFloat& e, const SystemType system = UnitSystem::System);
    Quantity(const MagnitudeFloat& m, const MagnitudeFloat& e, const BaseUnits::ListType& bu, const SystemType system = UnitSystem::System);
#if defined(MAGNITUDE_VALUES)
    Quantity(val::BaseValue::PointerType m, std::string s, const SystemType system = SystemType::NONE);
    Quantity(val::BaseValue::PointerType m, const SystemType system = UnitSystem::System);
    Quantity(val::BaseValue::PointerType m, const BaseUnits::ListType& bu, const SystemType system = UnitSystem::System);
    Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, std::string s, const SystemType system = SystemType::NONE);
    Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const SystemType system = UnitSystem::System);
    Quantity(val::BaseValue::PointerType m, val::BaseValue::PointerType e, const BaseUnits::ListType& bu, const SystemType system = UnitSystem::System);
#endif
#endif
    std::string unit_system() const;
    std::size_t size() const;
#if defined(MAGNITUDE_VALUES)
    val::Array::ShapeType shape() const;
    PointerType clone() const;
#endif
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    // quantity operations
    friend Quantity operator+(const Quantity& q1, const Quantity& q2);
    friend Quantity operator-(const Quantity& q1, const Quantity& q2);
    friend Quantity operator*(const Quantity& q1, const Quantity& q2);
    friend Quantity operator/(const Quantity& q1, const Quantity& q2);
    friend bool operator==(const Quantity& q1, const Quantity& q2);
    friend bool operator!=(const Quantity& q1, const Quantity& q2);
    // scalar operations
    friend Quantity operator+(const MagnitudeFloat& m, const Quantity& q);
    friend Quantity operator-(const MagnitudeFloat& m, const Quantity& q);
    friend Quantity operator*(const MagnitudeFloat& m, const Quantity& q);
    friend Quantity operator/(const MagnitudeFloat& m, const Quantity& q);
    //friend bool operator==(const MagnitudeFloat& m, const Quantity& q);
    friend Quantity operator+(const Quantity& q, const MagnitudeFloat& m);
    friend Quantity operator-(const Quantity& q, const MagnitudeFloat& m);
    friend Quantity operator*(const Quantity& q, const MagnitudeFloat& m);
    friend Quantity operator/(const Quantity& q, const MagnitudeFloat& m);
    //friend bool operator==(const Quantity& q, const MagnitudeFloat& m);
#if defined(MAGNITUDE_VALUES)
    // array operations
    friend Quantity operator+(val::BaseValue::PointerType a, const Quantity& q);
    friend Quantity operator-(val::BaseValue::PointerType a, const Quantity& q);
    friend Quantity operator*(val::BaseValue::PointerType a, const Quantity& q);
    friend Quantity operator/(val::BaseValue::PointerType a, const Quantity& q);
    //friend bool operator==(val::BaseValue::PointerType a, const Quantity& q);
    friend Quantity operator+(const Quantity& q, val::BaseValue::PointerType a);
    friend Quantity operator-(const Quantity& q, val::BaseValue::PointerType a);
    friend Quantity operator*(const Quantity& q, val::BaseValue::PointerType a);
    friend Quantity operator/(const Quantity& q, val::BaseValue::PointerType a);
    //friend bool operator==(const Quantity& q, val::BaseValue::PointerType a);
#endif
    friend Quantity operator+(const Quantity& q);
    friend Quantity operator-(const Quantity& q);
    friend std::ostream& operator<<(std::ostream& os, const Quantity& q);
    void operator+=(Quantity& q);
    void operator-=(Quantity& q);
    void operator*=(Quantity& q);
    void operator/=(Quantity& q);
    Quantity convert(const Format::Base& format, SystemType system = SystemType::NONE) const;
    Quantity convert(const Quantity& q) const;
    Quantity convert(const UnitValue& uv) const;
    Quantity convert(const UnitValue& uv, const SystemType system, const std::string& q = "") const;
    Quantity convert(const BaseUnits& bu) const;
    Quantity convert(const BaseUnits& bu, const SystemType system, const std::string& q = "") const;
    Quantity convert(std::string s, SystemType system = SystemType::NONE, const std::string& q = "") const;
    Quantity rebase_prefixes();
    Quantity rebase_dimensions();
  };

} // namespace snt::puq

#endif // PUQ_QUANTITY_H
