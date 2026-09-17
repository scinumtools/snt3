#ifndef PUQ_UNIT_VALUE_H
#define PUQ_UNIT_VALUE_H

#include <snt/puq/base_units.h>
#include <snt/puq/exponent.h>
#include <snt/puq/result.h>
#include <snt/puq/settings.h>

namespace snt::puq {

    /** Numerical estimate and absolute uncertainty associated with physical base units.
     *
     * The `result` and `baseunits` members describe one object: arithmetic propagates
     * uncertainty and combines dimensions, while conversion changes only the unit
     * representation and numerical scale.
     */
    class Measurement {
      public:
        Result result;       ///< Estimate and optional absolute uncertainty; arrays retain their shape.
        BaseUnits baseunits; ///< Physical dimensions and units attached to the estimate.

        /** Convert an absolute uncertainty to a relative uncertainty.
         * @param v Central value.
         * @param a Absolute uncertainty in the same units as `v`.
         */
        static double abs_to_rel(const double v, const double a);
        /** Convert a relative uncertainty to an absolute uncertainty.
         * @param v Central value.
         * @param r Relative uncertainty expressed as a fraction of `v`.
         */
        static double rel_to_abs(const double v, const double r);
        static val::BaseValue::PointerType abs_to_rel(val::BaseValue::PointerType v, val::BaseValue::PointerType a);
        static val::BaseValue::PointerType rel_to_abs(val::BaseValue::PointerType v, val::BaseValue::PointerType r);

        Measurement() : result(1) {}
        Measurement(const std::string& str);
        Measurement(const double est, const std::string& str);
        Measurement(const double est) : result(est) {};
        Measurement(const double est, const Dimensions& dim);
        Measurement(const double est, const BaseUnits& bun) : result(est), baseunits(bun) {};
        Measurement(const double est, const BaseUnits::ListType& bun) : result(est), baseunits(bun) {};
        Measurement(const double est, const double unc, const std::string& str);
        Measurement(const double est, const double unc) : result(est, unc) {};
        Measurement(const double est, const double unc, const BaseUnits& bun) : result(est, unc), baseunits(bun) {};
        Measurement(const double est, const double unc, const BaseUnits::ListType& bun)
            : result(est, unc), baseunits(bun) {};
        Measurement(val::BaseValue::PointerType est, const std::string& str);
        Measurement(val::BaseValue::PointerType est) : result(std::move(est)) {};
        Measurement(val::BaseValue::PointerType est, const BaseUnits& bun) : result(std::move(est)), baseunits(bun) {};
        Measurement(val::BaseValue::PointerType est, const BaseUnits::ListType& bun)
            : result(std::move(est)), baseunits(bun) {};
        Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc, const std::string& str);
        Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc)
            : result(std::move(est), std::move(unc)) {};
        Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc, const BaseUnits& bun)
            : result(std::move(est), std::move(unc)), baseunits(bun) {};
        Measurement(val::BaseValue::PointerType est, val::BaseValue::PointerType unc, const BaseUnits::ListType& bun)
            : result(std::move(est), std::move(unc)), baseunits(bun) {};
        Measurement(const Result& est) : result(est) {}
        Measurement(const Result& est, const std::string& str);
        Measurement(const Result& est, const Dimensions& dim);
        Measurement(const Result& est, const BaseUnits& bun) : result(est), baseunits(bun) {};
        Measurement(const Result& est, const BaseUnits::ListType& bun) : result(est), baseunits(bun) {};
        Measurement(const Measurement& msr) : result(msr.result), baseunits(msr.baseunits) {}
        Measurement(const Measurement& msr, const std::string& str);
        Measurement(const Measurement& msr, const Dimensions& dim);
        Measurement(const Measurement& msr, const BaseUnits& bun) : result(msr.result), baseunits(bun) {};
        Measurement(const Measurement& msr, const BaseUnits::ListType& bun) : result(msr.result), baseunits(bun) {};
        /** Return the number of stored values. */
        std::size_t size() const;
        /** Return the value shape. */
        val::Array::ShapeType shape() const;
        /** Format the measurement and its units. */
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
        /** Add the right-hand value in place.
         * @param msr Measurement whose units or value are used.
         */
        void operator+=(const Measurement& msr);
        /** Subtract the right-hand value in place.
         * @param msr Measurement whose units or value are used.
         */
        void operator-=(const Measurement& msr);
        /** Multiply by the right-hand value in place.
         * @param msr Measurement whose units or value are used.
         */
        void operator*=(const Measurement& msr);
        /** Divide by the right-hand value in place.
         * @param msr Measurement whose units or value are used.
         */
        void operator/=(const Measurement& msr);
        // void pow(const ExponentVariant& exp);
        /** Convert to a selected base-unit representation. */
        Measurement convert(const Format::Base& format) const;
        /** Convert to units parsed from a string. */
        Measurement convert(const std::string& str) const;
        /** Convert to explicit base units. */
        Measurement convert(const BaseUnits& bun) const;
        /** Convert to another measurement's units. */
        Measurement convert(const Measurement& msr) const;
        Measurement rebase_prefixes();
        Measurement rebase_dimensions();
    };

} // namespace snt::puq

#endif // PUQ_UNIT_VALUE_H
