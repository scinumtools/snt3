#ifndef PUQ_EXPONENT_H
#define PUQ_EXPONENT_H

#include <snt/puq/settings.h>
#include <snt/puq/unit_format.h>
#include <variant>

namespace snt::puq {

    /** Rational exponent represented by an integer numerator and denominator. */
    class Exponent {
      public:
        int numerator;
        int denominator;
        Exponent() : numerator(1), denominator(1) {}
        Exponent(const int n[2]) : numerator(n[0]), denominator(n[1]) {};
        Exponent(const int& n) : numerator(n), denominator(1) {};
        Exponent(const int& n, const int& d) : numerator(n), denominator(d) {};
        friend Exponent operator*(Exponent e1, const Exponent& e2);
        friend Exponent operator/(Exponent e1, const Exponent& e2);
        friend std::ostream& operator<<(std::ostream& os, const Exponent& e);
        Exponent operator-() const;
        /** Add the right-hand value in place.
         * @param e Exponent value.
         */
        void operator+=(const Exponent& e);
        /** Subtract the right-hand value in place.
         * @param e Exponent value.
         */
        void operator-=(const Exponent& e);
        /** Multiply by the right-hand value in place.
         * @param e Exponent value.
         */
        void operator*=(const Exponent& e);
        /** Divide by the right-hand value in place.
         * @param e Exponent value.
         */
        void operator/=(const Exponent& e);
        /** Compare the two values for equality.
         * @param e Exponent value.
         */
        bool operator==(const Exponent& e) const;
        /** Compare the two values for inequality.
         * @param e Exponent value.
         */
        bool operator!=(const Exponent& e) const;
        /** Compare the two values for equality.
         * @param e Exponent value.
         */
        bool operator==(const int& e) const;
        /** Compare the two values for inequality.
         * @param e Exponent value.
         */
        bool operator!=(const int& e) const;
        bool operator==(const double e) const;
        bool operator!=(const double e) const;
        double to_float() const;
        operator double() const;
        /** Format the object as text.
         * @param format Formatting options controlling the textual representation.
         */
        std::string to_string(const UnitFormat& format = UnitFormat()) const;
        void reduce();
    };

    /** Add an integer to an exponent.
     * @param a Exponent value.
     * @param i Integer exponent increment.
     */
    inline Exponent operator+(Exponent a, const int& i) {
        a += Exponent{i};
        return a;
    }

    /** Add an integer to an exponent.
     * @param i Integer exponent increment.
     * @param a Exponent value.
     */
    inline Exponent operator+(const int& i, Exponent a) {
        a += Exponent{i};
        return a;
    }

    /** Add two exponents.
     * @param a First operand.
     * @param b Second operand.
     */
    inline Exponent operator+(Exponent a, const Exponent& b) {
        a += b;
        return a;
    }

    /** Subtract an integer from an exponent.
     * @param a Exponent value.
     * @param i Integer exponent decrement.
     */
    inline Exponent operator-(Exponent a, const int& i) {
        a -= Exponent{i};
        return a;
    }

    /** Subtract an exponent from an integer.
     * @param i Integer exponent value.
     * @param a Exponent to subtract.
     */
    inline Exponent operator-(int i, const Exponent& a) {
        Exponent lhs{i};
        lhs -= a;
        return lhs;
    }

    /** Subtract two exponents.
     * @param a First operand.
     * @param b Second operand.
     */
    inline Exponent operator-(Exponent a, const Exponent& b) {
        a -= b;
        return a;
    }

    /** Multiply an exponent by an integer.
     * @param e Exponent value.
     * @param i Integer multiplier.
     */
    inline Exponent operator*(const Exponent& e, const int& i) {
        return e * Exponent{i};
    }

    /** Multiply an integer by an exponent.
     * @param i Integer multiplier.
     * @param e Exponent value.
     */
    inline Exponent operator*(const int& i, const Exponent& e) {
        return Exponent{i} * e;
    }

    /** Multiply an exponent by an integer in place.
     * @param e Exponent value.
     * @param i Integer multiplier.
     */
    inline void operator*=(Exponent& e, const int& i) {
        e *= Exponent{i};
    }

    using ExponentVariant = std::variant<int, Exponent>;

    inline ExponentVariant add_exp(const ExponentVariant& x, const ExponentVariant& y) {
        return std::visit(
            [](auto const& a, auto const& b) -> ExponentVariant {
                using A = std::decay_t<decltype(a)>;
                using B = std::decay_t<decltype(b)>;

                if constexpr (std::is_same_v<A, int> && std::is_same_v<B, int>)
                    return a + b;
                else
                    return Exponent(a) + Exponent(b);
            },
            x,
            y
        );
    }

    inline ExponentVariant mul_exp(const ExponentVariant& x, const ExponentVariant& y) {
        return std::visit(
            [](auto const& a, auto const& b) -> ExponentVariant {
                using A = std::decay_t<decltype(a)>;
                using B = std::decay_t<decltype(b)>;

                if constexpr (std::is_same_v<A, int> && std::is_same_v<B, int>)
                    return a * b;
                else
                    return Exponent(a) * Exponent(b);
            },
            x,
            y
        );
    }

    inline double exponent_to_float(const ExponentVariant& exp) {
        if (std::holds_alternative<int>(exp))
            return (double)std::get<int>(exp);
        else
            return (double)std::get<Exponent>(exp);
    };

} // namespace snt::puq

#endif // PUQ_EXPONENT_H
