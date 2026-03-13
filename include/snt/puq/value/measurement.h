#ifndef PUQ_UNIT_VALUE_H
#define PUQ_UNIT_VALUE_H

#include <snt/puq/exponent.h>
#include <snt/puq/magnitude.h>
#include <snt/puq/settings.h>
#include <snt/puq/value/base_units.h>

namespace snt::puq {

  class Measurement {
  public:
    Magnitude magnitude;
    BaseUnits baseunits;
    
    static double abs_to_rel(const double& v, const double& a);
    static double rel_to_abs(const double& v, const double& r);
    static val::BaseValue::PointerType abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType a);
    static val::BaseValue::PointerType rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType r);
    
    Measurement() : magnitude(1) {}
    Measurement(const std::string& str);
    Measurement(const double& est, const std::string& str);
    Measurement(const double& est) : magnitude(est) {};
    Measurement(const double& est, const Dimensions& dim);
    Measurement(const double& est, const BaseUnits& bun) : magnitude(est), baseunits(bun) {};
    Measurement(const double& est, const BaseUnits::ListType& bun) : magnitude(est), baseunits(bun) {};
    Measurement(const double& est, const double& unc, const std::string& str);
    Measurement(const double& est, const double& unc) : magnitude(est, unc) {};
    Measurement(const double& est, const double& unc, const BaseUnits& bun) : magnitude(est, unc), baseunits(bun) {};
    Measurement(const double& est, const double& unc, const BaseUnits::ListType& bun) : magnitude(est, unc), baseunits(bun) {};
    Measurement(val::BaseValue::PointerType est, const std::string& str);
    Measurement(val::BaseValue::PointerType est) : magnitude(std::move(est)) {};
    Measurement(val::BaseValue::PointerType est, const BaseUnits& bun) : magnitude(std::move(est)), baseunits(bun) {};
    Measurement(val::BaseValue::PointerType est, const BaseUnits::ListType& bun) : magnitude(std::move(est)), baseunits(bun) {};
    Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc, const std::string& str);
    Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc) : magnitude(std::move(est), std::move(unc)) {};
    Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc, const BaseUnits& bun) : magnitude(std::move(est), std::move(unc)), baseunits(bun) {};
    Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc, const BaseUnits::ListType& bun) : magnitude(std::move(est), std::move(unc)), baseunits(bun) {};
    Measurement(const Magnitude& est) : magnitude(est) {}
    Measurement(const Magnitude& est, const std::string& str);
    Measurement(const Magnitude& est, const Dimensions& dim);
    Measurement(const Magnitude& est, const BaseUnits& bun) : magnitude(est), baseunits(bun) {};
    Measurement(const Magnitude& est, const BaseUnits::ListType& bun) : magnitude(est), baseunits(bun) {};
    Measurement(const Measurement& msr) : magnitude(msr.magnitude), baseunits(msr.baseunits) {}
    Measurement(const Measurement& msr, const std::string& str);
    Measurement(const Measurement& msr, const Dimensions& dim);
    Measurement(const Measurement& msr, const BaseUnits& bun) : magnitude(msr.magnitude), baseunits(bun) {};
    Measurement(const Measurement& msr, const BaseUnits::ListType& bun) : magnitude(msr.magnitude), baseunits(bun) {};
    std::size_t size() const;
    val::Array::ShapeType shape() const;
    std::string to_string(const UnitFormat& format = UnitFormat()) const;
    friend bool operator==(const Measurement& msr1, const Measurement& msr2);
    friend bool operator!=(const Measurement& msr1, const Measurement& msr2);
    friend Measurement operator+(const Measurement& msr1, const Measurement& msr2);
    friend Measurement operator-(const Measurement& msr1, const Measurement& msr2);
    friend Measurement operator*(const Measurement& msr1, const Measurement& msr2);
    friend Measurement operator/(const Measurement& msr1, const Measurement& msr2);
    friend Measurement operator+(const Measurement& msr);
    friend Measurement operator-(const Measurement& msr);
    friend std::ostream& operator<<(std::ostream& os, const Measurement& msr);
    void operator+=(const Measurement& msr);
    void operator-=(const Measurement& msr);
    void operator*=(const Measurement& msr);
    void operator/=(const Measurement& msr);
    void pow(const ExponentVariant& exp);
    Measurement convert(const Format::Base& format) const;
    Measurement convert(const std::string& str) const;
    Measurement convert(const BaseUnits& bun) const;
    Measurement convert(const Measurement& msr) const;
    Measurement rebase_prefixes();
    Measurement rebase_dimensions();
  };

} // namespace snt::puq

#endif // PUQ_UNIT_VALUE_H
