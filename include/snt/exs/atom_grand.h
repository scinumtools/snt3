#ifndef EXS_ATOM_GRAND_H
#define EXS_ATOM_GRAND_H

#include <string>

namespace snt::exs {

    /**
     * @class AtomGrand
     *  Virtual base class actually used in the solver
     */
    class AtomGrand {
      public:
        /**
         *  Default class destructor
         */
        virtual ~AtomGrand() = default;

        /**
         *  Get a string representation of an atom
         */
        virtual std::string to_string() = 0;

        /**
         *  Print out atoms representation
         */
        void print();

        // Math operations

        /**
         *  Add this value to another value
         *
         * @param other Pointer to another atom
         */
        virtual void math_add(AtomGrand* other) {};

        /**
         *  Subtract another value from this value
         *
         * @param other Pointer to another atom
         */
        virtual void math_subtract(AtomGrand* other) {};

        /**
         *  Negate value of this value
         */
        virtual void math_negate() {};

        /**
         *  Multiply this value with another value
         *
         * @param other Pointer to another atom
         */
        virtual void math_multiply(AtomGrand* other) {};

        /**
         *  Divide this value with another value
         *
         * @param other Pointer to another atom
         */
        virtual void math_divide(AtomGrand* other) {};

        /**
         *  Raise this values to a given power
         *
         * @param other Pointer to another atom
         */
        virtual void math_power(AtomGrand* other) {};

        /**
         *  Perform modulo on this atom
         *
         * @param other Pointer to another atom
         */
        virtual void math_modulo(AtomGrand* other) {};

        // Argument math operators

        /**
         *  Get an exponential value of this value
         */
        virtual void math_exponent() {};

        /**
         *  Get a natural logarithm of this value
         */
        virtual void math_logarithm() {};

        /**
         *  Get a decadic logarithm of this value
         */
        virtual void math_logarithm_10() {};

        /**
         *  Get a logarithm with arbitrary base of this value
         *
         * @param other Pointer to another atom
         */
        virtual void math_logarithm_base(AtomGrand* other) {};

        /**
         *  Get power with arbitrary base of this value
         *
         * @param other Pointer to another atom
         */
        virtual void math_power_base(AtomGrand* other) {};

        /**
         *  Get square root value of this value
         */
        virtual void math_square_root() {};

        /**
         *  Get cubic root of this value
         */
        virtual void math_cubic_root() {};

        /**
         *  Get sinus of this value
         */
        virtual void math_sinus() {};

        /**
         *  Get cosinus of this value
         */
        virtual void math_cosinus() {};

        /**
         *  Get tangens value of this value
         */
        virtual void math_tangens() {};

        // Comparison operations

        /**
         *  Compare equality of this and other value
         *
         * @param other Pointer to another atom
         */
        virtual void comparison_equal(AtomGrand* other) {};

        /**
         *  Compare difference of ths and other value
         *
         * @param other Pointer to another atom
         */
        virtual void comparison_not_equal(AtomGrand* other) {};

        /**
         *  Compare if this value is less or equal than other
         *
         * @param other Pointer to another atom
         */
        virtual void comparison_less_equal(AtomGrand* other) {};

        /**
         *  Compare if this value is greater or equal than other
         *
         * @param other Pointer to another atom
         */
        virtual void comparison_greater_equal(AtomGrand* other) {};

        /**
         *  Compare if this value is less than other
         *
         * @param other Pointer to another atom
         */
        virtual void comparison_less(AtomGrand* other) {};

        /**
         *  Compare if this value is greater than other
         *
         * @param other Pointer to another atom
         */
        virtual void comparison_greater(AtomGrand* other) {};

        // Logical operations

        /**
         *  Get logical negation of this value
         */
        virtual void logical_not() {};

        /**
         *  Get logical and of this and other value
         *
         * @param other Pointer to another atom
         */
        virtual void logical_and(AtomGrand* other) {};

        /**
         *  Get logical or of this and other value
         *
         * @param other Pointer to another atom
         */
        virtual void logical_or(AtomGrand* other) {};

        // Conditional operator

        /**
         *  Decide between an option1 and option2 based of this value
         *
         * @param option1 Pointer to another atom
         * @param option2 Pointer to another atom
         */
        virtual void condition(AtomGrand* option1, AtomGrand* option2) {};
    };

} // namespace snt::exs

#endif // EXS_ATOM_GRAND_H
