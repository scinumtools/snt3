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
        BaseUnit() {};
        BaseUnit(const int& n) : exponent(n) {};
        BaseUnit(const int& n, const int& d) : exponent(Exponent(n, d)) {};
        BaseUnit(const std::string& p, const std::string& u, const ExponentVariant& e)
            : prefix(p), unit(u), exponent(e) {};
        BaseUnit(const std::string& p, const std::string& u, const int& n, const int& d)
            : prefix(p), unit(u), exponent(Exponent(n, d)) {};
        std::string to_string(const UnitFormat& format = UnitFormat());
    };

    /** Ordered collection of base units representing a unit expression. */
    class BaseUnits {
      public:
        using ListType = std::vector<BaseUnit>;
        BaseUnits::ListType baseunits; ///< Ordered base-unit factors
        BaseUnits() {}
        BaseUnits(const std::string& s);
        BaseUnits(const BaseUnits::ListType& bu) : baseunits(bu) {}
        /** Append a base-unit factor.
         *  @param bu Factor to append.
         */
        void append(const BaseUnit& bu);
        /** Append a factor with a rational exponent.
         *  @param p Prefix, such as ``k``.
         *  @param u Unit symbol.
         *  @param e Exponent.
         */
        void append(const std::string& p, const std::string& u, ExponentVariant e);
        /** Append a factor with numerator and denominator. */
        void append(const std::string& p, const std::string& u, int n, int d);
        /** Format the complete unit expression. */
        std::string to_string(const UnitFormat& format = UnitFormat()) const;
        const BaseUnit& operator[](int index) const;
        friend std::ostream& operator<<(std::ostream& os, const BaseUnits& bu);
        void operator*=(const BaseUnits& bu);
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
