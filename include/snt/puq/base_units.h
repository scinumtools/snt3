#ifndef PUQ_BASE_UNITS_H
#define PUQ_BASE_UNITS_H

#include <snt/puq/dimensions.h>
#include <snt/puq/exponent.h>
#include <snt/puq/settings.h>

namespace snt::puq {

    /** One base unit, including an optional prefix and exponent. */
    class BaseUnit {
      public:
        std::string prefix;       ///< Unit prefix, such as k or m
        std::string unit;         ///< Unit symbol
        ExponentVariant exponent; ///< Unit exponent
        BaseUnit() = default;
        /** Construct an unnamed unit factor with an integer exponent.
         * @param n Integer exponent stored in the factor.
         */
        BaseUnit(const int& n) : exponent(n) {};
        /** Construct a base unit with a rational exponent.
         * @param n Exponent numerator.
         * @param d Exponent denominator.
         */
        BaseUnit(const int& n, const int& d) : exponent(Exponent(n, d)) {};
        /** Construct a base unit from a prefix, symbol, and exponent.
         * @param p Unit prefix, such as `k`.
         * @param u Unit symbol.
         * @param e Unit exponent.
         */
        BaseUnit(const std::string& p, const std::string& u, const ExponentVariant& e)
            : prefix(p), unit(u), exponent(e) {};
        /** Construct a base unit from a prefix, symbol, and rational exponent.
         * @param p Unit prefix, such as `k`.
         * @param u Unit symbol.
         * @param n Exponent numerator.
         * @param d Exponent denominator.
         */
        BaseUnit(const std::string& p, const std::string& u, const int& n, const int& d)
            : prefix(p), unit(u), exponent(Exponent(n, d)) {};
        /** Format the object as text.
         * @param format Formatting options controlling the textual representation.
         */
        std::string to_string(const UnitFormat& format = UnitFormat());
    };

    /** Ordered collection of base units representing a unit expression. */
    class BaseUnits {
      public:
        using ListType = std::vector<BaseUnit>;
        BaseUnits::ListType baseunits; ///< Ordered base-unit factors
        BaseUnits() = default;
        /** Parse a unit expression into base-unit factors.
         * @param s Unit expression to parse.
         */
        BaseUnits(const std::string& s);
        /** Construct a collection from existing base-unit factors.
         * @param bu Factors to copy.
         */
        BaseUnits(const BaseUnits::ListType& bu) : baseunits(bu) {}
        /** Append a unit factor, combining it with an existing matching factor.
         * @param bu Unit factor to append; a resulting zero exponent removes the factor.
         */
        void append(const BaseUnit& bu);
        /** Append a factor with an exponent variant.
         * @param p Unit prefix.
         * @param u Unit symbol.
         * @param e Integer or rational exponent.
         */
        void append(const std::string& p, const std::string& u, ExponentVariant e);
        /** Append a factor with a rational exponent.
         * @param p Unit prefix.
         * @param u Unit symbol.
         * @param n Exponent numerator.
         * @param d Exponent denominator.
         */
        void append(const std::string& p, const std::string& u, int n, int d);
        /** Format the complete unit expression. */
        std::string to_string(const UnitFormat& format = UnitFormat()) const;
        /** Access the item at the requested key or index.
         * @param index Zero-based element index.
         */
        const BaseUnit& operator[](int index) const;
        friend std::ostream& operator<<(std::ostream& os, const BaseUnits& bu);
        /** Multiply by the right-hand value in place.
         * @param bu Base units associated with the value.
         */
        void operator*=(const BaseUnits& bu);
        /** Divide by the right-hand value in place.
         * @param bu Base units associated with the value.
         */
        void operator/=(const BaseUnits& bu);
        // void operator*=(const ExponentVariant& exp);
        // void operator/=(const ExponentVariant& exp);
        friend BaseUnits operator*(BaseUnits bu1, const BaseUnits& bu2);
        friend BaseUnits operator/(BaseUnits bu1, const BaseUnits& bu2);
        // friend BaseUnits operator*(BaseUnits bu, const ExponentVariant& exp);
        // friend BaseUnits operator/(BaseUnits bu, const ExponentVariant& exp);
        BaseUnits::ListType::const_iterator begin() const;
        BaseUnits::ListType::const_iterator end() const;
        /** Return the number of unit factors. */
        std::size_t size() const;
        /** Test whether any factor carries dimensions. */
        bool has_dimensions() const;
        /** Calculate the combined dimensions. */
        Dimensions dimensions() const;
    };

} // namespace snt::puq

#endif // PUQ_BASE_UNITS_H
