#ifndef PUQ_QUANTITY_H
#define PUQ_QUANTITY_H

#include <iomanip>
#include <snt/puq/settings.h>
#include <snt/puq/solver/unit_solver.h>
#include <snt/puq/systems/unit_system.h>
#include <sstream>

namespace snt::puq {

    /**
     * A numerical value with optional uncertainty, physical units, and a unit system.
     *
     * Quantities preserve the shape of the underlying VAL value. Arithmetic combines
     * values and derives dimensions according to the unit algebra; addition and
     * subtraction require compatible dimensions. Conversions return a new quantity.
     */
    class Quantity {
      private:
        void preprocess(std::string& expression, SystemType& system) const;
        Measurement _convert_without_context(UnitSystem& us, const SystemType stt) const;
        Measurement _convert_with_context(
            UnitSystem& us,
            const SystemType stt,
            QuantityListType::iterator& qs1,
            QuantityListType::iterator& qs2,
            const std::string& q
        ) const;

      public:
        SystemType stype;        ///< Unit system used to parse and format this quantity.
        Measurement measurement; ///< Value and absolute uncertainty together with its base units.
        /// Construct a dimensionless quantity with value one in the current unit system.
        Quantity() : stype(UnitSystem::current.type) {};
        /// Parse a quantity expression such as `"3.2 m/s"`.
        /// @param s Quantity expression containing a value and unit expression.
        /// @param system Unit system used for resolving unit names; `NONE` selects the current system.
        Quantity(std::string s, const SystemType system = SystemType::NONE);
        /// Construct a quantity from an existing measurement.
        /// @param v Measurement providing value, uncertainty, and units.
        /// @param system Unit system associated with the quantity.
        Quantity(const Measurement& v, const SystemType system = UnitSystem::current.type);
        /// Construct from a numerical result and parse the target unit expression.
        /// @param m Estimate and optional uncertainty.
        /// @param s Unit or quantity expression.
        /// @param system Unit system used to resolve `s`.
        Quantity(const Result& m, std::string s, const SystemType system = SystemType::NONE);
        /// Construct from a result using the current unit system.
        /// @param m Estimate and optional uncertainty.
        /// @param system Unit system associated with the quantity.
        Quantity(const Result& m, const SystemType system = UnitSystem::current.type);
        /// Construct from a result and explicit base-unit components.
        /// @param m Estimate and optional uncertainty.
        /// @param bu Base-unit components describing the dimensions.
        /// @param system Unit system associated with the quantity.
        Quantity(const Result& m, const BaseUnits::ListType& bu, const SystemType system = UnitSystem::current.type);
        /// Construct a quantity from a scalar and a unit expression.
        /// @param m Numerical estimate.
        /// @param s Unit or quantity expression.
        /// @param system Unit system used to resolve `s`.
        Quantity(const double m, std::string s, const SystemType system = SystemType::NONE);
        /// Construct a dimensionless scalar quantity.
        /// @param m Numerical estimate.
        /// @param system Unit system associated with the quantity.
        Quantity(const double m, const SystemType system = UnitSystem::current.type);
        /// Construct a scalar quantity from explicit base-unit components.
        /// @param m Numerical estimate.
        /// @param bu Base-unit components describing the dimensions.
        /// @param system Unit system associated with the quantity.
        Quantity(const double m, const BaseUnits::ListType& bu, const SystemType system = UnitSystem::current.type);
        /// Construct a quantity from an estimate, absolute uncertainty, and unit expression.
        /// @param m Numerical estimate.
        /// @param e Absolute uncertainty in the same units as `m`.
        /// @param s Unit or quantity expression.
        /// @param system Unit system used to resolve `s`.
        Quantity(const double m, const double e, std::string s, const SystemType system = SystemType::NONE);
        /// Construct a dimensionless quantity with an absolute uncertainty.
        /// @param m Numerical estimate.
        /// @param e Absolute uncertainty matching `m`.
        /// @param system Unit system associated with the quantity.
        Quantity(const double m, const double e, const SystemType system = UnitSystem::current.type);
        Quantity(
            const double m,
            const double e,
            const BaseUnits::ListType& bu,
            const SystemType system = UnitSystem::current.type
        );
        Quantity(val::BaseValue::PointerType m, std::string s, const SystemType system = SystemType::NONE);
        Quantity(val::BaseValue::PointerType m, const SystemType system = UnitSystem::current.type);
        Quantity(
            val::BaseValue::PointerType m,
            const BaseUnits::ListType& bu,
            const SystemType system = UnitSystem::current.type
        );
        Quantity(
            val::BaseValue::PointerType m,
            val::BaseValue::PointerType e,
            std::string s,
            const SystemType system = SystemType::NONE
        );
        Quantity(
            val::BaseValue::PointerType m,
            val::BaseValue::PointerType e,
            const SystemType system = UnitSystem::current.type
        );
        Quantity(
            val::BaseValue::PointerType m,
            val::BaseValue::PointerType e,
            const BaseUnits::ListType& bu,
            const SystemType system = UnitSystem::current.type
        );
        /** Return the active unit system name. */
        std::string unit_system() const;
        /** Return the number of stored values. */
        std::size_t size() const;
        /** Return the shape of the stored value. */
        val::Array::ShapeType shape() const;
        /** Return a human-readable description of the quantity. */
        std::string info() const;
        /** Format the quantity using the requested options. */
        std::string to_string(const UnitFormat& format = UnitFormat()) const;
        // quantity operations
        friend Quantity operator+(const Quantity& q1, const Quantity& q2);
        friend Quantity operator-(const Quantity& q1, const Quantity& q2);
        friend Quantity operator*(const Quantity& q1, const Quantity& q2);
        friend Quantity operator/(const Quantity& q1, const Quantity& q2);
        friend bool operator==(const Quantity& q1, const Quantity& q2);
        friend bool operator!=(const Quantity& q1, const Quantity& q2);
        // scalar operations
        friend Quantity operator+(const double m, const Quantity& q);
        friend Quantity operator-(const double m, const Quantity& q);
        friend Quantity operator*(const double m, const Quantity& q);
        friend Quantity operator/(const double m, const Quantity& q);
        // friend bool operator==(const double m, const Quantity& q);
        friend Quantity operator+(const Quantity& q, const double m);
        friend Quantity operator-(const Quantity& q, const double m);
        friend Quantity operator*(const Quantity& q, const double m);
        friend Quantity operator/(const Quantity& q, const double m);
        // friend bool operator==(const Quantity& q, const double m);
        //  array operations
        friend Quantity operator+(val::BaseValue::PointerType a, const Quantity& q);
        friend Quantity operator-(val::BaseValue::PointerType a, const Quantity& q);
        friend Quantity operator*(val::BaseValue::PointerType a, const Quantity& q);
        friend Quantity operator/(val::BaseValue::PointerType a, const Quantity& q);
        // friend bool operator==(val::BaseValue::PointerType a, const Quantity& q);
        friend Quantity operator+(const Quantity& q, val::BaseValue::PointerType a);
        friend Quantity operator-(const Quantity& q, val::BaseValue::PointerType a);
        friend Quantity operator*(const Quantity& q, val::BaseValue::PointerType a);
        friend Quantity operator/(const Quantity& q, val::BaseValue::PointerType a);
        // friend bool operator==(const Quantity& q, val::BaseValue::PointerType a);
        friend Quantity operator+(const Quantity& q);
        friend Quantity operator-(const Quantity& q);
        friend std::ostream& operator<<(std::ostream& os, const Quantity& q);
        /** Add the right-hand value in place.
         * @param q Quantity name or expression.
         */
        void operator+=(Quantity& q);
        /** Subtract the right-hand value in place.
         * @param q Quantity name or expression.
         */
        void operator-=(Quantity& q);
        /** Multiply by the right-hand value in place.
         * @param q Quantity name or expression.
         */
        void operator*=(Quantity& q);
        /** Divide by the right-hand value in place.
         * @param q Quantity name or expression.
         */
        void operator/=(Quantity& q);
        /** Convert to a base-unit representation in the selected system.
         *  @param format Base-unit formatting mode.
         *  @param system Target unit system.
         *  @return Converted quantity.
         */
        Quantity convert(const Format::Base& format, SystemType system = SystemType::NONE) const;
        /** Convert to the units represented by another quantity.
         *  @param q Quantity providing the target units.
         *  @return Converted quantity.
         */
        Quantity convert(const Quantity& q) const;
        /** Convert to the units represented by a measurement.
         * @param uv Measurement whose units define the target dimensions and scale.
         * @return A converted copy; the source quantity is unchanged.
         */
        Quantity convert(const Measurement& uv) const;
        /** Convert using explicit target units and an optional unit-system context.
         * @param uv Measurement whose units define the target dimensions and scale.
         * @param system Unit system used while resolving contextual conversions.
         * @param q Optional quantity name used by contextual conversion rules.
         * @return A converted copy; throws when dimensions are incompatible.
         */
        Quantity convert(const Measurement& uv, const SystemType system, const std::string& q = "") const;
        /// Convert to explicit base units, preserving the numerical value's shape.
        Quantity convert(const BaseUnits& bu) const;
        /** Convert to explicit base units in a selected system.
         * @param bu Target base-unit expression.
         * @param system Unit system used for contextual conversion rules.
         * @param q Optional quantity name for contextual conversions.
         * @return A converted copy; throws when dimensions are incompatible.
         */
        Quantity convert(const BaseUnits& bu, const SystemType system, const std::string& q = "") const;
        /** Parse target units and convert to them.
         * @param s Unit or quantity expression describing the target units.
         * @param system Unit system used to resolve the expression.
         * @param q Optional quantity name for contextual conversion rules.
         * @return A converted copy; throws for invalid or incompatible units.
         */
        Quantity convert(std::string s, SystemType system = SystemType::NONE, const std::string& q = "") const;
        /** Re-express units using their base prefixes. */
        Quantity rebase_prefixes();
        /** Re-express units using base dimensions. */
        Quantity rebase_dimensions();
    };

} // namespace snt::puq

#endif // PUQ_QUANTITY_H
